/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Natale Patriciello <natale.patriciello@gmail.com>
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
 */
#include "tcp-congestion-ops.h"
#include "tcp-socket-base.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("TcpCongestionOps");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpCongestionOps);

TypeId
TcpCongestionOps::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpCongestionOps")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
 ;
  return tid;
}

TcpCongestionOps::TcpCongestionOps ()
{
}

TcpCongestionOps::TcpCongestionOps (const TcpCongestionOps &other)
{
}

TcpCongestionOps::~TcpCongestionOps ()
{
}


// RENO

NS_OBJECT_ENSURE_REGISTERED (TcpReno);

TypeId
TcpReno::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpReno")
    .SetParent<TcpCongestionOps> ()
    .SetGroupName ("Internet")
    .AddConstructor<TcpReno> ()
  ;
  return tid;
}

TcpReno::TcpReno (void) : TcpCongestionOps ()
{
  NS_LOG_FUNCTION (this);
}

TcpReno::TcpReno (const TcpReno& sock)
  : TcpCongestionOps (sock)
{
  NS_LOG_FUNCTION (this);
}

TcpReno::~TcpReno (void)
{
}

void
TcpReno::NewAck(Ptr<TcpSocketState> state)
{
  NS_LOG_FUNCTION (this);

  // Check for exit condition of fast recovery
  if (state->m_inFastRec)
    { // RFC2001, sec.4; RFC2581, sec.3.2
      // First new ACK after fast recovery: reset cwnd
      state->m_cWnd = state->m_ssThresh;
      state->m_inFastRec = false;
      NS_LOG_INFO ("Reset cwnd to " << state->m_cWnd);
    };

  // Increase of cwnd based on current phase (slow start or congestion avoidance)
  if (state->m_cWnd < state->m_ssThresh)
    {
      state->m_cWnd += state->m_segmentSize;
      NS_LOG_INFO ("In SlowStart, updated to cwnd " << state->m_cWnd << " ssthresh " << state->m_ssThresh);
    }
  else
    { // Congestion avoidance mode, increase by (segSize*segSize)/cwnd. (RFC2581, sec.3.1)
      // To increase cwnd for one segSize per RTT, it should be (ackBytes*segSize)/cwnd
      double adder = static_cast<double> (state->m_segmentSize * state->m_segmentSize) / state->m_cWnd.Get ();
      adder = std::max (1.0, adder);
      state->m_cWnd += static_cast<uint32_t> (adder);
      NS_LOG_INFO ("In CongAvoid, updated to cwnd " << state->m_cWnd << " ssthresh " << state->m_ssThresh);
    }
}

std::string
TcpReno::GetName() const
{
  return "TcpReno";
}

uint32_t
TcpReno::GetSSThresh(Ptr<const TcpSocketState> state)
{
  return std::max (state->m_cWnd.Get () >> 1U, 2U);
}

// NewReno

NS_OBJECT_ENSURE_REGISTERED (TcpNewReno);

TypeId
TcpNewReno::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpNewReno")
    .SetParent<TcpCongestionOps> ()
    .SetGroupName ("Internet")
    .AddConstructor<TcpNewReno> ()
  ;
  return tid;
}

TcpNewReno::TcpNewReno (void) : TcpCongestionOps ()
{
  NS_LOG_FUNCTION (this);
}

TcpNewReno::TcpNewReno (const TcpNewReno& sock)
  : TcpCongestionOps (sock)
{
  NS_LOG_FUNCTION (this);
}

TcpNewReno::~TcpNewReno (void)
{
}

void
TcpNewReno::NewAck(Ptr<TcpSocketState> state)
{
  // TODO
}

std::string
TcpNewReno::GetName() const
{
  return "TcpNewReno";
}

uint32_t
TcpNewReno::GetSSThresh(Ptr<const TcpSocketState> state)
{
  return std::max (state->m_cWnd.Get () >> 1U, 2U);
}

} // namespace ns3

