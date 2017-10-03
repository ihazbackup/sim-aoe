#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/wifi-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/ptr.h"

#include "ndn-custom-apps/forward-geocast-strategy.hpp"

namespace ns3 {

class Coord {
  public:
    string x, y;

    Coord(string xPos, string yPos) 
    {
      x = xPos;
      y = yPos;
    }
};

int
main(int argc, char* argv[])
{
  /*
  Config::SetDefault("ns3::CsmaChannel::DataRate", StringValue("10Mbps"));
  Config::SetDefault("ns3::CsmaChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::DropTailQueue::MaxPackets", StringValue("20"));
  */
  
  Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                     StringValue("OfdmRate24Mbps"));
  
  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Mode", StringValue ("Time"));
  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Time", StringValue ("2s"));
  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Bounds", StringValue ("0|200|0|200"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  NodeContainer nodes;
  nodes.Create(5);

  // Coordinates
  Coord coordinates[5] = {
    Coord("30"  , "30"), // Consumer, the rest is producer
    Coord("50"  , "30"),
    Coord("60"  , "20"),
    Coord("30"  , "40"),
    Coord("20"  , "60")
  };

  // WIFI
  
  WifiHelper wifi = WifiHelper::Default();
  wifi.SetStandard(WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("OfdmRate24Mbps"));
  YansWifiChannelHelper wifiChannel; // = YansWifiChannelHelper::Default ();
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  // Can use this to model signal range
  wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
      "MaxRange", DoubleValue(30.0));
  // wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");
  YansWifiPhyHelper wifiPhyHelper = YansWifiPhyHelper::Default();
  wifiPhyHelper.SetChannel(wifiChannel.Create());
  wifiPhyHelper.Set("TxPowerStart", DoubleValue(5));
  wifiPhyHelper.Set("TxPowerEnd", DoubleValue(5));
  NqosWifiMacHelper wifiMacHelper = NqosWifiMacHelper::Default();
  wifiMacHelper.SetType("ns3::AdhocWifiMac");

  NetDeviceContainer wifiNetDevices = wifi.Install(wifiPhyHelper, wifiMacHelper, nodes);
  
  ndn::AppHelper consumerApp("MyConsumer");
  consumerApp.SetPrefix("/prefix");
  
  ndn::AppHelper prodApp("MyProducer");
  prodApp.SetPrefix("/prefix");
  prodApp.SetAttribute("PayloadSize", StringValue("1024"));
  // CSMA
  /*
  CsmaHelper csma;
  csma.Install(nodes);
  */
  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();

  for (int i = 0 ; i < 5 ; i++) {
    MobilityHelper mob;
    mob.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
      "X", StringValue (coordinates[i].x),
      "Y", StringValue (coordinates[i].y),
      "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=0]"));
    mob.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    if (i == 0) {
      // Consumer
      consumerApp.Install(nodes.Get(i));
    } else {
      // Producers
      /*
      mob.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
        "Mode", StringValue("Time"),
        "Time", StringValue("2s"),
        "Speed", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
        "Bounds", StringValue("0|150|0|150"));
        */
      prodApp.Install(nodes.Get(i));
    }

    mob.Install(nodes.Get(i));
  }

  ndn::StrategyChoiceHelper::Install<nfd::fw::ForwardGeocastStrategy>(nodes, "/prefix");
  ndn::L3RateTracer::InstallAll("jason-rate-trace.txt", Seconds(1.0));

  Simulator::Stop(Seconds(20.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  std::cout << "Starting custom app..." << std::endl;
  return ns3::main(argc, argv);
}
