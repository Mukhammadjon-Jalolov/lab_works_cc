/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/mobility-model.h"
#include "ns3/csma-module.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/internet-module.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/olsr-module.h"
#include <iostream>


           
        
using namespace ns3;



NS_LOG_COMPONENT_DEFINE ("LAB3");


int
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nWifi = 6;

  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.Parse (argc,argv);

  if (nWifi > 18)
    {
      std::cout << "Number of wifi nodes " << nWifi << 
                   " specified exceeds the mobility bounding box" << std::endl;
      exit (1);
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

/////////////////////////////Nodes/////////////////////////////  
  //TODO
  // Create nodes

 /////////////////////////////Wi-Fi part///////////////////////////// 

  //TODO
  // Create WifiChannel with PropagationLossModel and SpeedPropagationDelayModel
 
 
  
  
  //Physical layer of WiFi
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  
      
  phy.Set("TxPowerEnd", DoubleValue(16));
  phy.Set("TxPowerStart", DoubleValue(16));
  phy.Set("EnergyDetectionThreshold", DoubleValue(-89));
  phy.Set("CcaMode1Threshold", DoubleValue(-99));
  phy.Set("ChannelNumber", UintegerValue(7));
  //TODO
  //Attach WiFi channel to physical layer


  //TODO
  //Create WiFi helper and set standart and RemoteStationManager
  
  //Mac layer
  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();
  mac.SetType ("ns3::AdhocWifiMac");

/////////////////////////////Devices///////////////////////////// 


  //TODO
  // Create network devices
  // Attach devices and all parts of WiFi system and Nodes
  



 /////////////////////////////Deployment///////////////////////////// 
//TODO
// Create mobility model with constant positions and deploy all devices in a line with 400m distance between them Z coordinate for all should be 1m!! 
  
/////////////////////////////Stack of protocols///////////////////////////// 

  
//  Enable OLSR routing
   OlsrHelper olsr;

 //  Install the routing protocol
   Ipv4ListRoutingHelper list;
   list.Add (olsr, 10);

  // Set up internet stack

  InternetStackHelper stack;
  stack.SetRoutingHelper (list);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

 /////////////////////////////Ip addresation/////////////////////////////  
  address.SetBase ("10.1.1.0", "255.255.255.0");
  //TODO
  //Create Ipv4InterfaceContainer 
  // assign IP addresses to WifiDevices into Ipv4InterfaceContainer



/////////////////////////////Application part///////////////////////////// 
 
   uint16_t dlPort = 1000; //Port number
    
    //Sending application on the first station
    
    ApplicationContainer onOffApp;
    OnOffHelper onOffHelper("ns3::UdpSocketFactory", InetSocketAddress(wifiInterfaces.GetAddress (nWifi-1), dlPort)); //OnOffApplication, UDP traffic,
    onOffHelper.SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=5000]"));
    onOffHelper.SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    onOffHelper.SetAttribute("DataRate", DataRateValue(DataRate("10.0Mbps"))); //Traffic Bit Rate
    onOffHelper.SetAttribute("PacketSize", UintegerValue(200)); // Packet size
    onOffApp.Add(onOffHelper.Install(wifiStaNodes.Get(0)));  
 
    
  //Opening receiver socket on the last station
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (wifiStaNodes.Get (nWifi-1), tid);
  InetSocketAddress local = InetSocketAddress (wifiInterfaces.GetAddress (nWifi-1), dlPort);
  bool ipRecvTos = true;
  recvSink->SetIpRecvTos (ipRecvTos);
  bool ipRecvTtl = true;
  recvSink->SetIpRecvTtl (ipRecvTtl);
  recvSink->Bind (local);


///////////////USE WHEN WORKING WITH TCP TO FILL IN ARP TABLES//////////////////////
/*
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (wifiStaNodes.Get (nWifi-1));
  serverApps.Start (Seconds (2.0));
  serverApps.Stop (Seconds (6.0));

  UdpEchoClientHelper echoClient (wifiInterfaces.GetAddress (nWifi-1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (100));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (100));

  ApplicationContainer clientApps = echoClient.Install (wifiStaNodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (6.0));
*/
/////////////////////////////////////////////////////////////////////////////////

/////////////////////TCP app///////////////////////////
/*
  uint16_t port = 50000;
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApp = sinkHelper.Install (wifiStaNodes.Get (nWifi-1));
  sinkApp.Start (Seconds (10.0));
  sinkApp.Stop (Seconds (100.0));
  // Create the OnOff applications to send TCP to the server (client part)
  OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address ());
  clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper.SetAttribute("DataRate", DataRateValue(DataRate("10.0Mbps"))); //Traffic Bit Rate
  clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  ApplicationContainer clientApp;
      AddressValue clientAddress(InetSocketAddress (wifiInterfaces.GetAddress (nWifi-1), port));
      clientHelper.SetAttribute ("Remote", clientAddress);
      clientApp=clientHelper.Install (wifiStaNodes.Get (0));
      clientApp.Start (Seconds (10.0));
      clientApp.Stop (Seconds (100.0));
*/

////////////////////////////////////////////////////////////


/////////////////////////////Application part///////////////////////////// 

  Simulator::Stop (Seconds (100.0));
/////////////////////////////PCAP tracing/////////////////////////////   
   //TODO 
   //Enable PCAP tracing for all devices

  Simulator::Run ();
  Simulator::Destroy ();
return 0;
};
