
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

#include "helper.cpp"
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
      /*
      .AddAttribute("MobilityHelper",
                    "Reference to mobility model",
                    PointerValue(), MakePointerAccessor(&MyProducer::m_mobility), 
                    MakePointerChecker<MobilityHelper>());*/
  
  return tid;
}

MyProducer::MyProducer()
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


  std::cout << "Producer started!!" << std::endl;
  
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
  std::cout << "[ MyProducer ] receiving interest " << *interest << std::endl;
  App::OnInterest(interest); // tracing inside

  NS_LOG_FUNCTION(this << interest);

  // std::cout << "[ MyProducer ] receiving interest " << *interest << std::endl;

  Name dataName(interest->getName());
  // dataName.append(m_postfix);
  // dataName.appendVersion();

  std::cout << "parsing name ...." << std::endl;
  std::vector<std::string> parts = std::Helper::split(dataName.toUri(), '/');
  std::cout << "idx-0 = " << parts[0] << std::endl;
  std::cout << "idx-1 (xpos) = " << parts[1] << std::endl;
  std::cout << "idx-2 (ypos) = " << parts[2] << std::endl;
  std::cout << "idx-3 (radius) = " << parts[3] << std::endl;

  // ATMT Packet
  std::cout << "ATMT UUID = " << interest->get_atmt_uuid() << std::endl;

  // check mobility (BIG THX TO https://groups.google.com/forum/#!topic/ns-3-users/ftQqy23ug_E)
  Ptr<RandomWalk2dMobilityModel> model = GetNode()->GetObject<RandomWalk2dMobilityModel>();
  Vector pos = model->GetPosition();
  std::cout << "current pos upon receiving interest: " << pos << std::endl;

  // Getting extra tag
  /*
  auto tag = interest-> template getTag<lp::ATMTPacketTag>();
  if (tag == nullptr)
  {
    std::cout << "[ MyProducer ] No ATMT Packet found" << std::endl;
  } 
  else 
  {
    std::cout << "[ MyProducer ] ATMT UUID: " <<  tag->get().getData() << std::endl;
  }
  */

  auto data = make_shared<Data>();
  data->setName(dataName);
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));
  data->setContent(make_shared< ::ndn::Buffer>(m_virtualPayloadSize));
  StackHelper::getKeyChain().sign(*data);

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
  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);
}

} // namespace ndn
} // namespace ns3
