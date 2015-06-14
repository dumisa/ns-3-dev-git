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
#ifndef TCPCONGESTIONOPS_H
#define TCPCONGESTIONOPS_H

#include "ns3/object.h"

namespace ns3 {

class TcpSocketState;

/**
 * \brief Congestion control abstract class
 */
class TcpCongestionOps : public Object
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Default constructor
   */
  TcpCongestionOps();

  /**
   * \brief Default deconstructor
   */
  virtual ~TcpCongestionOps ();

  /**
   * \brief Get the slow start threshold after a loss event
   * \param Socket internal state
   * \return Slow start threshold
   */
  virtual uint32_t GetSSThresh (Ptr<TcpSocketState> state)     = 0;

  /**
   * \brief Congestion avoidance algorithm implementation
   *
   * \param Socket internal state
   */
  virtual void CongestionAvoid (Ptr<TcpSocketState> state) = 0;

  // Linux:
  /* call before changing ca_state (optional) */
  // void (*set_state)(struct sock *sk, u8 new_state);
  /* call when cwnd event occurs (optional) */
  // void (*cwnd_event)(struct sock *sk, enum tcp_ca_event ev);
  /* call when ack arrives (optional) */
  // void (*in_ack_event)(struct sock *sk, u32 flags);
  /* new value of cwnd after loss (optional) */
  // u32  (*undo_cwnd)(struct sock *sk);
  /* hook for packet ack accounting (optional) */
  // void (*pkts_acked)(struct sock *sk, u32 num_acked, s32 rtt_us);

  virtual std::string GetName () = 0;
};

} // namespace ns3

#endif // TCPCONGESTIONOPS_H
