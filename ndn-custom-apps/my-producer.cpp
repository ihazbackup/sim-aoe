
#include "my-producer.hpp"
#include "ns3/mobility-module.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"

#include <memory>

#include "helper.hpp"
#include "forward-geocast-strategy.hpp"
#include <ndn-cxx/lp/tags.hpp>

NS_LOG_COMPONENT_DEFINE("MyProducer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(MyProducer);

TypeId
MyProducer::GetTypeId(void)
{
  static TypeId tid =
    TypeId("MyProducer")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<MyProducer>()
      .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&MyProducer::m_prefix), MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
         StringValue("/"), MakeNameAccessor(&MyProducer::m_postfix), MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&MyProducer::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&MyProducer::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&MyProducer::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&MyProducer::m_keyLocator), MakeNameChecker());
  
  return tid;
}

MyProducer::MyProducer() 
  : m_rand(CreateObject<UniformRandomVariable>())
{
  NS_LOG_FUNCTION_NOARGS();

}

// inherited from Application base class.
void
MyProducer::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();

  FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
}

void
MyProducer::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}

void
MyProducer::OnInterest(shared_ptr<const Interest> interest)
{
  
  App::OnInterest(interest); // tracing inside

  NS_LOG_FUNCTION(this << interest);

  auto id = GetNode()->GetId();
  std::cout << "[ Node "<< id << " ] receiving interest " << *interest << std::endl;
  std::cout << "[ Node "<< id << " ] prevDist interest = " << interest->get_atmt_prevDist() 
            << std::endl;

  Name dataName(interest->getName());
  // dataName.append(m_postfix);
  // dataName.appendVersion();

  std::vector<std::string> parts = std::Helper::split(dataName.toUri(), '/');

  // check mobility (BIG THX TO https://groups.google.com/forum/#!topic/ns-3-users/ftQqy23ug_E)
  Ptr<MobilityModel> model = GetNode()->GetObject<MobilityModel>();
  Vector pos = model->GetPosition();
  std::cout << "[ Node "<< id << " ] location at " << pos << std::endl;

  // Process if inside AoI
  bool shouldProcessInterest = 
      nfd::fw::IsInsideAoI(pos.x, pos.y, std::stod(parts[1]), std::stod(parts[2]), std::stod(parts[3]));

  if (shouldProcessInterest) {
    std::cout << "[ Node "<< id << " ] is inside AoI.."<< std::endl;
    auto data = make_shared<Data>();
    data->setName(dataName);
    data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));
    data->setContent(make_shared< ::ndn::Buffer>(m_virtualPayloadSize));
    StackHelper::getKeyChain().sign(*data);

    m_transmittedDatas(data, this, m_face);
    m_appLink->onReceiveData(*data);
  } 
  else {
    std::cout << "[ Node "<< id << " ] is outside AoI.. forwarding.."<< std::endl;
    // forward it
    shared_ptr<Interest> fInterest = make_shared<Interest>(interest->getName());
    fInterest->set_atmt_uuid(interest->get_atmt_uuid());
    fInterest->setNonce(interest->getNonce());
    fInterest->set_atmt_prevDist(interest->get_atmt_prevDist());
    fInterest->setInterestLifetime(interest->getInterestLifetime());

    std::cout << "[ Node "<< id << " ] forwarding with prevDist: "<< *fInterest << std::endl;

    m_transmittedInterests(fInterest, this, m_face);
    m_appLink->onReceiveInterest(*fInterest);
    
  }
/*
  Signature signature;
  SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

  if (m_keyLocator.size() > 0) {
    signatureInfo.setKeyLocator(m_keyLocator);
  }

  signature.setInfo(signatureInfo);
  signature.setValue(::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));

  data->setSignature(signature);

  NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());

  // to create real wire encoding
  data->wireEncode();
  std::cout << "[ MyProducer ] sending data " << *data << std::endl;

  */
}

} // namespace ndn
} // namespace ns3
