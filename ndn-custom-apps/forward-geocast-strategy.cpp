#include <math.h>

#include "forward-geocast-strategy.hpp"

#include <boost/random/uniform_int_distribution.hpp>

#include <ndn-cxx/util/random.hpp>

#include "core/logger.hpp"

#include "ns3/mobility-module.h"

#include "ns3/node-list.h"

#include "helper.hpp"

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
  ns3::Vector pos = mobilityModel->GetPosition();
  if (mobilityModel == nullptr)
  {
    std::cout << "[Forwarder "<< nodeId <<"] No mobility model!, the node does not know its current position";
    this->rejectPendingInterest(pitEntry);
    return;
  }

  // Get parsed interest name
  std::vector<std::string> parts = std::Helper::split(interest.getName().toUri(), '/');

  bool shouldForward = true;
  shared_ptr<Interest> outInterest = nullptr;

  if(IsInsideAoI(pos.x, pos.y, std::stod(parts[1]), std::stod(parts[2]), std::stod(parts[3])))
  {
    std::cout << "[Forwarder "<< nodeId <<"] is inside AoI" << std::endl;
    shouldForward = true;
  } 
  else {
    std::cout << "[Forwarder "<< nodeId <<"] is outside AoI" << std::endl;
    // check distance
    uint32_t thisDist = GetDistance(pos.x, pos.y, std::stod(parts[1]), std::stod(parts[2]));
    shouldForward = thisDist <= interest.get_atmt_prevDist();

    std::cout << "- this node's distance to AOI = " << thisDist << std::endl;
    std::cout << "- prevDist to AOI = " << interest.get_atmt_prevDist() << std::endl;

    if (shouldForward)
    {
      // Update if closer
      outInterest = make_shared<ndn::Interest>(interest);
      outInterest->set_atmt_prevDist(static_cast<uint32_t>(thisDist));
    }
  }

  if (shouldForward)
  {
    std::cout << "[Forwarder "<< nodeId <<"] forwarding..." << std::endl;
    
    std::cout << "[Forwarder "<< nodeId <<"] forwarding..." << std::endl;
    for (fib::NextHopList::const_iterator it = nexthops.begin(); it != nexthops.end(); ++it) {
      Face& outFace = it->getFace();
      // if (!wouldViolateScope(inFace, interest, outFace) &&
      //  canForwardToLegacy(*pitEntry, outFace)) {
      
      if (outInterest != nullptr) 
      {
        std::cout << "[Forwarder "<< nodeId <<"] begin forwarding updated..." << std::endl;
        this->sendInterest(pitEntry, outFace, *outInterest);
      }
      else {
        this->sendInterest(pitEntry, outFace, interest);
      }

        // return;
      // }
    }
    
  } 
  else {
    std::cout << "[Forwarder "<< nodeId <<"] dropping..." << std::endl;
    this->rejectPendingInterest(pitEntry);
  }

}

} // namespace fw
} // namespace nfd
