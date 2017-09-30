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

// ndn-custom-apps.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/wifi-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/ptr.h"

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

static void 
CourseChange (std::string foo, Ptr<const MobilityModel> mobility)
{
  Vector pos = mobility->GetPosition ();
  Vector vel = mobility->GetVelocity ();
  std::cout << foo << ": model=" << mobility << ", POS: x=" << pos.x << ", y=" << pos.y
            << ", z=" << pos.z << "; VEL:" << vel.x << ", y=" << vel.y
            << ", z=" << vel.z << std::endl;
}

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
  nodes.Create(2);

  // Coordinates
  Coord coordinates[4] = {
    Coord("0"   , "0"), // Consumer
    Coord("30"  , "0"),
    Coord("60"  , "0"),
    Coord("50"  , "30"), // Producer
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
      "MaxRange", DoubleValue(45.0));
  // wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");
  YansWifiPhyHelper wifiPhyHelper = YansWifiPhyHelper::Default();
  wifiPhyHelper.SetChannel(wifiChannel.Create());
  wifiPhyHelper.Set("TxPowerStart", DoubleValue(5));
  wifiPhyHelper.Set("TxPowerEnd", DoubleValue(5));
  NqosWifiMacHelper wifiMacHelper = NqosWifiMacHelper::Default();
  wifiMacHelper.SetType("ns3::AdhocWifiMac");

  NetDeviceContainer wifiNetDevices = wifi.Install(wifiPhyHelper, wifiMacHelper, nodes);
  
  // CSMA
  /*
  CsmaHelper csma;
  csma.Install(nodes);
  */
  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();

  // Mobility model
  /*
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
   "X", StringValue ("50.0"),
   "Y", StringValue ("50.0"),
   "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=30]"));
  mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
    "Mode", StringValue("Time"),
    "Time", StringValue("2s"),
    "Speed", StringValue("ns3::ConstantRandomVariable[Constant=3.0]"),
    "Bounds", StringValue("0|125|0|125"));
    */

  MobilityHelper mob;
  mob.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
   "X", StringValue ("20.0"),
   "Y", StringValue ("20.0"),
   "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=0]"));
  mob.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mob.Install(nodes.Get(0));
  ndn::AppHelper consumerApp("MyConsumer");
  consumerApp.SetPrefix("/prefix");
  consumerApp.Install(nodes.Get(0));

  //  MobilityHelper mob;
  
  mob.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
   "X", StringValue ("35.0"),
   "Y", StringValue ("35.0"),
   "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=0]"));
  mob.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
    "Mode", StringValue("Time"),
    "Time", StringValue("2s"),
    "Speed", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
    "Bounds", StringValue("0|125|0|125"));
  mob.Install(nodes.Get(1));
  ndn::AppHelper prodApp("MyProducer");
  prodApp.SetPrefix("/prefix");
  prodApp.SetAttribute("PayloadSize", StringValue("1024"));
  prodApp.Install(nodes.Get(1));

  
  // Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
  //                 MakeCallback (&CourseChange));
  

  // The static model
  // MobilityHelper staticMob;
  /*
  staticMob.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
   "X", StringValue ("35.0"),
   "Y", StringValue ("35.0"),
   "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=30]"));
  staticMob.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  */
  /*
  for (int i = 0 ; i < 4 ; i++) {
     
    MobilityHelper staticMob;
    staticMob.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
     "X", StringValue (coordinates[i].x),
     "Y", StringValue (coordinates[i].y),
     "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=0]"));
    staticMob.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    
    if (i == 0) {
      ndn::AppHelper consumerApp("MyConsumer");
      consumerApp.SetPrefix("/prefix");
      consumerApp.Install(nodes.Get(i));
    } else if (i == 3) {
      ndn::AppHelper producerApp("MyProducer");
      producerApp.SetPrefix("/prefix");
      producerApp.SetAttribute("PayloadSize",   StringValue("1024"));
      // producerApp.SetAttribute("MobilityModel", PointerValue(staticMob));
      producerApp.Install(nodes.Get(i));
    } else {
      staticMob.Install(nodes.Get(i));
    }
  }
  */
  // consumerApp.Install(nodes.Get(0));
  /*
  for (int i = 0 ; i < 9 ; i++) {
    mobility.Install(nodes.Get(i));
  }
  */
  /*
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
   "X", StringValue ("10.0"),
   "Y", StringValue ("30.0"),
   "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=0]"));
  mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
    "Mode", StringValue("Time"),
    "Time", StringValue("2s"),
    "Speed", StringValue("ns3::ConstantRandomVariable[Constant=3.0]"),
    "Bounds", StringValue("0|50|0|50"));
  producerApp.Install(nodes.Get(3));
  mobility.Install(nodes.Get(3));
  */
  // staticMob.Install(nodes.Get(9));

  // Strategy for given prefix
  // ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");

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
