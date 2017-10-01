#include <math.h>

#include "forward-geocast-strategy.hpp"

#include <boost/random/uniform_int_distribution.hpp>

#include <ndn-cxx/util/random.hpp>

#include "core/logger.hpp"

#include "ns3/mobility-module.h"

#include "ns3/node-list.h"

NFD_LOG_INIT("ForwardGeocastStrategy");

namespace nfd {
namespace fw {

const Name
  ForwardGeocastStrategy::STRATEGY_NAME("ndn:/localhost/nfd/strategy/forward-geocast");

ForwardGeocastStrategy::ForwardGeocastStrategy(Forwarder& forwarder, const Name& name)
  : Strategy(forwarder, name)
{
}

ForwardGeocastStrategy::~ForwardGeocastStrategy()
{
}

static bool
canForwardToNextHop(const Face& inFace, shared_ptr<pit::Entry> pitEntry, const fib::NextHop& nexthop)
{

  // TODO Check node location
  return !wouldViolateScope(inFace, pitEntry->getInterest(), nexthop.getFace()) &&
    canForwardToLegacy(*pitEntry, nexthop.getFace());
}

static bool
hasFaceForForwarding(const Face& inFace, const fib::NextHopList& nexthops, const shared_ptr<pit::Entry>& pitEntry)
{
  return std::find_if(nexthops.begin(), nexthops.end(), bind(&canForwardToNextHop, cref(inFace), pitEntry, _1))
         != nexthops.end();
}

void
ForwardGeocastStrategy::afterReceiveInterest(const Face& inFace, const Interest& interest,
                                                 const shared_ptr<pit::Entry>& pitEntry)
{
  NFD_LOG_TRACE("afterReceiveInterest");

  if (hasPendingOutRecords(*pitEntry)) {
    // not a new Interest, don't forward
    return;
  }

  const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
  const fib::NextHopList& nexthops = fibEntry.getNextHops();

  // Ensure there is at least 1 Face is available for forwarding
  if (!hasFaceForForwarding(inFace, nexthops, pitEntry)) {
    this->rejectPendingInterest(pitEntry);
    return;
  }

  
  // Get the node's location
  auto node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  uint32_t nodeId = node->GetId();
  auto mobilityModel = node->GetObject<ns3::MobilityModel>();
  if (mobilityModel == nullptr)
  {
    std::cout << "[Node "<< nodeId <<"] No mobility model!, the node does not know its current position";
  } 
  else {
    ns3::Vector pos = mobilityModel->GetPosition();
    std::cout << "[Node "<< nodeId <<"] current pos upon receiving interest: " << pos << std::endl;
  }

  // Get interest field
  std::cout << "[Node "<< nodeId <<"] interest name: " << interest.getName() << std::endl;
  std::cout << "[Node "<< nodeId <<"] prevDist on interest: " << interest.get_atmt_prevDist() << std::endl;
  
  // TODO check whether this node is inside AoI
  // TODO check whether this node is closer to AoI if above req is not met

  for (fib::NextHopList::const_iterator it = nexthops.begin(); it != nexthops.end(); ++it) {
    Face& outFace = it->getFace();
    if (!wouldViolateScope(inFace, interest, outFace) &&
        canForwardToLegacy(*pitEntry, outFace)) {
      this->sendInterest(pitEntry, outFace, interest);
      return;
    }
  }

  this->rejectPendingInterest(pitEntry);
  /*
  fib::NextHopList::const_iterator selected;
  do {
    boost::random::uniform_int_distribution<> dist(0, nexthops.size() - 1);
    const size_t randomIndex = dist(m_randomGenerator);

    uint64_t currentIndex = 0;

    for (selected = nexthops.begin(); selected != nexthops.end() && currentIndex != randomIndex;
         ++selected, ++currentIndex) {
    }
  } while (!canForwardToNextHop(inFace, pitEntry, *selected));
  */

  // this->sendInterest(pitEntry, inFace, interest);
}

} // namespace fw
} // namespace nfd
