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

#include "tcp-t.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "ns3/node.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpT");

NS_OBJECT_ENSURE_REGISTERED (TcpT);

TypeId
TcpT::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpT")
    .SetParent<TcpNewReno> ()
    .SetGroupName ("Internet")
    .AddConstructor<TcpT> ()
 ;
  return tid;
}

TcpT::TcpT (void)
{
  NS_LOG_FUNCTION (this);
}

TcpT::TcpT (const TcpT& sock)
  : TcpNewReno (sock)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("Invoked the copy constructor");
}

TcpT::~TcpT (void)
{
}

void
TcpT::DupAck(const TcpHeader &t, uint32_t count)
{
}

void
TcpT::NewAck(const SequenceNumber32 &seq)
{
  TcpSocketBase::NewAck (seq);
}

void
TcpT::Retransmit()
{
  TcpSocketBase::Retransmit();
}

void
TcpT::IncrCwnd()
{
  NS_LOG_UNCOND (Simulator::Now().GetSeconds() << " HO CHIAMATO INCR");

  m_timer.SetDelay(Time::FromDouble(1.0, Time::S));
  m_timer.SetFunction (&TcpT::IncrCwnd, this);
  m_timer.Schedule();
}

int
TcpT::Connect(const Address &address)
{
  int ret = TcpSocketBase::Connect(address);

  m_timer.SetDelay(Time::FromDouble(1.0, Time::S));
  m_timer.SetFunction (&TcpT::IncrCwnd, this);
  m_timer.Schedule();

  return ret;
}

} // namespace ns3
