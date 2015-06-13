/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Adrian Sai-wah Tam
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Adrian Sai-wah Tam <adrian.sw.tam@gmail.com>
 */

#define NS_LOG_APPEND_CONTEXT \
  if (m_node) { std::clog << Simulator::Now ().GetSeconds () << " [node " << m_node->GetId () << "] "; }

#include "tcp-reno.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "ns3/node.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpReno");

NS_OBJECT_ENSURE_REGISTERED (TcpReno);

TypeId
TcpReno::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpReno")
    .SetParent<TcpSocketBase> ()
    .SetGroupName ("Internet")
    .AddConstructor<TcpReno> ()
  ;
  return tid;
}

TcpReno::TcpReno (void) : TcpSocketBase ()
{
  NS_LOG_FUNCTION (this);
}

TcpReno::TcpReno (const TcpReno& sock)
  : TcpSocketBase (sock)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("Invoked the copy constructor");
}

TcpReno::~TcpReno (void)
{
}

Ptr<TcpSocketBase>
TcpReno::Fork (void)
{
  return CopyObject<TcpReno> (this);
}

/* New ACK (up to seqnum seq) received. Increase cwnd and call TcpSocketBase::NewAck() */
void
TcpReno::NewAck (const SequenceNumber32& seq)
{
  NS_LOG_FUNCTION (this << seq);
  NS_LOG_LOGIC ("TcpReno receieved ACK for seq " << seq <<
                " cwnd " << m_sState->m_cWnd <<
                " ssthresh " << m_sState->m_ssThresh);

  // Check for exit condition of fast recovery
  if (m_sState->m_inFastRec)
    { // RFC2001, sec.4; RFC2581, sec.3.2
      // First new ACK after fast recovery: reset cwnd
      m_sState->m_cWnd = m_sState->m_ssThresh;
      m_sState->m_inFastRec = false;
      NS_LOG_INFO ("Reset cwnd to " << m_sState->m_cWnd);
    };

  // Increase of cwnd based on current phase (slow start or congestion avoidance)
  if (m_sState->m_cWnd < m_sState->m_ssThresh)
    { // Slow start mode, add one segSize to cWnd. Default m_sState->m_ssThresh is 65535. (RFC2001, sec.1)
      m_sState->m_cWnd += m_sState->m_segmentSize;
      NS_LOG_INFO ("In SlowStart, updated to cwnd " << m_sState->m_cWnd << " ssthresh " << m_sState->m_ssThresh);
    }
  else
    { // Congestion avoidance mode, increase by (segSize*segSize)/cwnd. (RFC2581, sec.3.1)
      // To increase cwnd for one segSize per RTT, it should be (ackBytes*segSize)/cwnd
      double adder = static_cast<double> (m_sState->m_segmentSize * m_sState->m_segmentSize) / m_sState->m_cWnd.Get ();
      adder = std::max (1.0, adder);
      m_sState->m_cWnd += static_cast<uint32_t> (adder);
      NS_LOG_INFO ("In CongAvoid, updated to cwnd " << m_sState->m_cWnd << " ssthresh " << m_sState->m_ssThresh);
    }

  // Complete newAck processing
  TcpSocketBase::NewAck (seq);
}

// Fast recovery and fast retransmit
void
TcpReno::DupAck (const TcpHeader& t, uint32_t count)
{
  NS_LOG_FUNCTION (this << "t " << count);
  if (count == m_sState->m_retxThresh && !m_sState->m_inFastRec)
    { // triple duplicate ack triggers fast retransmit (RFC2581, sec.3.2)
      m_sState->m_ssThresh = std::max (2 * m_sState->m_segmentSize, BytesInFlight () / 2);
      m_sState->m_cWnd = m_sState->m_ssThresh + 3 * m_sState->m_segmentSize;
      m_sState->m_inFastRec = true;
      NS_LOG_INFO ("Triple dupack. Reset cwnd to " << m_sState->m_cWnd << ", ssthresh to " << m_sState->m_ssThresh);
      DoRetransmit ();
    }
  else if (m_sState->m_inFastRec)
    { // In fast recovery, inc cwnd for every additional dupack (RFC2581, sec.3.2)
      m_sState->m_cWnd += m_sState->m_segmentSize;
      NS_LOG_INFO ("Increased cwnd to " << m_sState->m_cWnd);
      if (!m_sendPendingDataEvent.IsRunning ())
        {
          SendPendingData (m_connected);
        }
    };
}

// Retransmit timeout
void TcpReno::Retransmit (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " ReTxTimeout Expired at time " << Simulator::Now ().GetSeconds ());
  m_sState->m_inFastRec = false;

  // If erroneous timeout in closed/timed-wait state, just return
  if (m_state == CLOSED || m_state == TIME_WAIT) return;
  // If all data are received (non-closing socket and nothing to send), just return
  if (m_state <= ESTABLISHED && m_txBuffer->HeadSequence () >= m_highTxMark) return;

  // According to RFC2581 sec.3.1, upon RTO, ssthresh is set to half of flight
  // size and cwnd is set to 1*MSS, then the lost packet is retransmitted and
  // TCP back to slow start
  m_sState->m_ssThresh = std::max (2 * m_sState->m_segmentSize, BytesInFlight () / 2);
  m_sState->m_cWnd = m_sState->m_segmentSize;
  m_nextTxSequence = m_txBuffer->HeadSequence (); // Restart from highest Ack
  NS_LOG_INFO ("RTO. Reset cwnd to " << m_sState->m_cWnd <<
               ", ssthresh to " << m_sState->m_ssThresh << ", restart from seqnum " << m_nextTxSequence);
  DoRetransmit ();                          // Retransmit the packet
}

} // namespace ns3
