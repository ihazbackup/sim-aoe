/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "my-consumer.hpp"
#include "ns3/mobility-module.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"

#include "utils/ndn-ns3-packet-tag.hpp"
#include "utils/ndn-rtt-mean-deviation.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/ref.hpp>

#include <ndn-cxx/lp/tags.hpp>
#include "forward-geocast-strategy.hpp"
// #include "ns3/ndnSIM/ndn-cxx/lp/atmt.hpp"

NS_LOG_COMPONENT_DEFINE("MyConsumer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(MyConsumer);

TypeId
MyConsumer::GetTypeId(void)
{
  static TypeId tid =
    TypeId("MyConsumer")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<MyConsumer>()
      .AddAttribute("StartSeq", "Initial sequence number", IntegerValue(0),
                    MakeIntegerAccessor(&MyConsumer::m_seq), MakeIntegerChecker<int32_t>())

      .AddAttribute("Prefix", "Name of the Interest", StringValue("/"),
                    MakeNameAccessor(&MyConsumer::m_interestName), MakeNameChecker())
      .AddAttribute("LifeTime", "LifeTime for interest packet", StringValue("2s"),
                    MakeTimeAccessor(&MyConsumer::m_interestLifeTime), MakeTimeChecker())

      .AddAttribute("RetxTimer",
                    "Timeout defining how frequent retransmission timeouts should be checked",
                    StringValue("50ms"),
                    MakeTimeAccessor(&MyConsumer::GetRetxTimer, &MyConsumer::SetRetxTimer),
                    MakeTimeChecker())

      .AddTraceSource("LastRetransmittedInterestDataDelay",
                      "Delay between last retransmitted Interest and received Data",
                      MakeTraceSourceAccessor(&MyConsumer::m_lastRetransmittedInterestDataDelay),
                      "MyConsumer::LastRetransmittedInterestDataDelayCallback")

      .AddTraceSource("FirstInterestDataDelay",
                      "Delay between first transmitted Interest and received Data",
                      MakeTraceSourceAccessor(&MyConsumer::m_firstInterestDataDelay),
                      "MyConsumer::FirstInterestDataDelayCallback");

  return tid;
}

MyConsumer::MyConsumer()
  : m_rand(CreateObject<UniformRandomVariable>())
  , m_seq(0)
  , m_seqMax(0) // don't request anything
{
  NS_LOG_FUNCTION_NOARGS();

  m_rtt = CreateObject<RttMeanDeviation>();
}

void
MyConsumer::SetRetxTimer(Time retxTimer)
{
  /*
  m_retxTimer = retxTimer;
  if (m_retxEvent.IsRunning()) {
    // m_retxEvent.Cancel (); // cancel any scheduled cleanup events
    Simulator::Remove(m_retxEvent); // slower, but better for memory
  }

  // schedule even with new timeout
  m_retxEvent = Simulator::Schedule(m_retxTimer, &MyConsumer::CheckRetxTimeout, this);
  */
}

Time
MyConsumer::GetRetxTimer() const
{
  return m_retxTimer;
}

void
MyConsumer::CheckRetxTimeout()
{
  /*
  Time now = Simulator::Now();

  Time rto = m_rtt->RetransmitTimeout();
  // NS_LOG_DEBUG ("Current RTO: " << rto.ToDouble (Time::S) << "s");

  while (!m_seqTimeouts.empty()) {
    SeqTimeoutsContainer::index<i_timestamp>::type::iterator entry =
      m_seqTimeouts.get<i_timestamp>().begin();
    if (entry->time + rto <= now) // timeout expired?
    {
      uint32_t seqNo = entry->seq;
      m_seqTimeouts.get<i_timestamp>().erase(entry);
      OnTimeout(seqNo);
    }
    else
      break; // nothing else to do. All later packets need not be retransmitted
  }

  m_retxEvent = Simulator::Schedule(m_retxTimer, &MyConsumer::CheckRetxTimeout, this);
  */
}

// Application Methods
void
MyConsumer::StartApplication() // Called at time specified by Start
{
  NS_LOG_FUNCTION_NOARGS();
  // do base stuff
  App::StartApplication();

  // Send 1 interest packet
  Simulator::Schedule(Seconds(1.0), &MyConsumer::SendInterest, this);
}

void
MyConsumer::StopApplication() // Called at time specified by Stop
{
  NS_LOG_FUNCTION_NOARGS();

  // cancel periodic packet generation
  Simulator::Cancel(m_sendEvent);

  // cleanup base stuff
  App::StopApplication();

}

void
MyConsumer::SendInterest()
{
  NS_LOG_FUNCTION_NOARGS();

  /*
  uint32_t seq = std::numeric_limits<uint32_t>::max(); // invalid

  while (m_retxSeqs.size()) {
    seq = *m_retxSeqs.begin();
    m_retxSeqs.erase(m_retxSeqs.begin());
    break;
  }

  if (seq == std::numeric_limits<uint32_t>::max()) {
    if (m_seqMax != std::numeric_limits<uint32_t>::max()) {
      if (m_seq >= m_seqMax) {
        return; // we are totally done
      }
    }

    seq = m_seq++;
  }
  */

  // setup a location where do u want the producers to process the Interest
  std::stringstream ss;
  ss << m_interestName.toUri() << "/50/30/10";
  Name reqWithLoc = Name(ss.str());

  shared_ptr<Name> nameWithSequence = make_shared<Name>(reqWithLoc);
  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setName(*nameWithSequence);
  interest->set_atmt_uuid("12345");
  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
  interest->setInterestLifetime(interestLifeTime);

  // Set previous distance
  Ptr<MobilityModel> model = GetNode()->GetObject<MobilityModel>();
  Vector pos = model->GetPosition();
  double distToAoi = nfd::fw::GetDistance(pos.x, pos.y, 50.0, 30.0);
  interest->set_atmt_prevDist(static_cast<uint32_t>(distToAoi));
  
  std::cout << "[ MyConsumer ] distance to AoI = " << distToAoi << std::endl;

  // Create a new ATMT Packet
  /*
  lp::ATMTPacket packet;
  interest->setTag(make_shared<lp::ATMTPacketTag>(packet));
  */

  std::cout << "[ MyConsumer ] sending interest " << *interest << std::endl;

  m_transmittedInterests(interest, this, m_face);
  m_appLink->onReceiveInterest(*interest);

  // Simulator::Schedule(Seconds(1.0), &MyConsumer::SendInterest, this);
}

///////////////////////////////////////////////////
//          Process incoming packets             //
///////////////////////////////////////////////////

void
MyConsumer::OnData(shared_ptr<const Data> data)
{

  App::OnData(data); // tracing inside

  NS_LOG_FUNCTION(this << data);

  std::cout << "[ MyConsumer ] receiving data " << *data << std::endl;

  // NS_LOG_INFO ("Received content object: " << boost::cref(*data));

  // This could be a problem......
  /*
  uint32_t seq = data->getName().at(-1).toSequenceNumber();
  NS_LOG_INFO("< DATA for " << seq);

  int hopCount = 0;
  auto hopCountTag = data->getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  }
  NS_LOG_DEBUG("Hop count: " << hopCount);

  SeqTimeoutsContainer::iterator entry = m_seqLastDelay.find(seq);
  if (entry != m_seqLastDelay.end()) {
    m_lastRetransmittedInterestDataDelay(this, seq, Simulator::Now() - entry->time, hopCount);
  }

  entry = m_seqFullDelay.find(seq);
  if (entry != m_seqFullDelay.end()) {
    m_firstInterestDataDelay(this, seq, Simulator::Now() - entry->time, m_seqRetxCounts[seq], hopCount);
  }

  m_seqRetxCounts.erase(seq);
  m_seqFullDelay.erase(seq);
  m_seqLastDelay.erase(seq);

  m_seqTimeouts.erase(seq);
  m_retxSeqs.erase(seq);

  m_rtt->AckSeq(SequenceNumber32(seq));
  */
}

void
MyConsumer::OnNack(shared_ptr<const lp::Nack> nack)
{
  /// tracing inside
  App::OnNack(nack);

  std::cout << "[ MyConsumer ] receiving NACK " << nack->getInterest().getName() 
            << " due to " << nack->getReason() << std::endl;

  NS_LOG_INFO("NACK received for: " << nack->getInterest().getName()
              << ", reason: " << nack->getReason());
}

void
MyConsumer::OnTimeout(uint32_t sequenceNumber)
{
  /*
  NS_LOG_FUNCTION(sequenceNumber);
  // std::cout << Simulator::Now () << ", TO: " << sequenceNumber << ", current RTO: " <<
  // m_rtt->RetransmitTimeout ().ToDouble (Time::S) << "s\n";

  m_rtt->IncreaseMultiplier(); // Double the next RTO
  m_rtt->SentSeq(SequenceNumber32(sequenceNumber),
                 1); // make sure to disable RTT calculation for this sample
  m_retxSeqs.insert(sequenceNumber);
  */
}

void
MyConsumer::WillSendOutInterest(uint32_t sequenceNumber)
{
  /*
  NS_LOG_DEBUG("Trying to add " << sequenceNumber << " with " << Simulator::Now() << ". already "
                                << m_seqTimeouts.size() << " items");

  m_seqTimeouts.insert(SeqTimeout(sequenceNumber, Simulator::Now()));
  m_seqFullDelay.insert(SeqTimeout(sequenceNumber, Simulator::Now()));

  m_seqLastDelay.erase(sequenceNumber);
  m_seqLastDelay.insert(SeqTimeout(sequenceNumber, Simulator::Now()));

  m_seqRetxCounts[sequenceNumber]++;

  m_rtt->SentSeq(SequenceNumber32(sequenceNumber), 1);
  */
}

} // namespace ndn
} // namespace ns3
