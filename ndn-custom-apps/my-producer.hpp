
#ifndef MY_PRODUCER_H
#define MY_PRODUCER_H

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/mobility-module.h"

#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/nstime.h"
#include "ns3/ptr.h"

namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-apps
 * @brief A simple Interest-sink applia simple Interest-sink application
 *
 * A simple Interest-sink applia simple Interest-sink application,
 * which replying every incoming Interest with Data packet with a specified
 * size and name same as in Interest.cation, which replying every incoming Interest
 * with Data packet with a specified size and name same as in Interest.
 */
class MyProducer : public App {
public:
  static TypeId
  GetTypeId(void);

  MyProducer();

  // inherited from NdnApp
  virtual void
  OnInterest(shared_ptr<const Interest> interest);

protected:
  // inherited from Application base class.
  virtual void
  StartApplication(); // Called at time specified by Start

  virtual void
  StopApplication(); // Called at time specified by Stop

private:
  Name m_prefix;
  Name m_postfix;
  uint32_t m_virtualPayloadSize;
  Time m_freshness;

  uint32_t m_signature;
  Name m_keyLocator;

  Ptr<UniformRandomVariable> m_rand; ///< @brief nonce generator
  // Mobility model settings
  // Ptr<MobilityHelper> m_mobility;
};

} // namespace ndn
} // namespace ns3

#endif // MY_PRODUCER_H
