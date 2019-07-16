#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/mobility-model.h>
#include <ns3/lte-module.h>
#include <ns3/lte-amc.h>
#include <ns3/lte-helper.h>
#include <ns3/epc-helper.h>
#include <ns3/network-module.h>
#include <ns3/ipv4-global-routing-helper.h>
#include <ns3/internet-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/applications-module.h>

using namespace ns3;

//*****************************************************************************************************************//
//     
//            d
//      UE <------> eNodeB <--EPC---> RemoteHost
//   7.0.0.x                           1.0.0.x
//
//
//******************************************************************************************************************//

int main (int argc, char *argv[])
{
 CommandLine cmd;
  //cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.Parse (argc,argv);
  
  
  // parse again so you can override default values from the command line
  

	

	//LTE helper
	Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
	Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> (); //Create EPC

    	lteHelper->SetEpcHelper (epcHelper); //Set EPC 
	lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::TwoRayGroundPropagationLossModel")); //Path loss model
	lteHelper->SetSchedulerType ("ns3::PfFfMacScheduler");    // MAC PF-FF scheduler
    	Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));

	//lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel");
    	//lteHelper->SetEnbAntennaModelType ("ns3::CosineAntennaModel");
    	lteHelper->SetEnbAntennaModelType ("ns3::ParabolicAntennaModel");

    
	lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (180));
	lteHelper->SetEnbAntennaModelAttribute ("Beamwidth", DoubleValue (60));
	//lteHelper->SetEnbAntennaModelAttribute ("MaxGain", DoubleValue (0.0));

	cmd.Parse (argc,argv);

    //Define PDN GW 
	Ptr<Node> pgw = epcHelper->GetPgwNode ();

   	// Create a single RemoteHost
  	NodeContainer remoteHostContainer;
 	remoteHostContainer.Create (1);
  	Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  	InternetStackHelper internet;
  	internet.Install (remoteHostContainer);

//**********************Create the Internet***************************************************************************//
  	PointToPointHelper p2ph;
  	p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  	p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  	p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  	NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  	Ipv4AddressHelper ipv4h;
  	ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  	Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  	// interface 0 is localhost, 1 is the p2p device
  	//Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);
  	Ipv4StaticRoutingHelper ipv4RoutingHelper;
 	Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  	remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

	//Create eNodeB and UE
	NodeContainer enbNodes;
	enbNodes.Create (1);
	NodeContainer ueNodes;
	ueNodes.Create (1);


 // Install Mobility Model
  
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);
  
  Ptr<MobilityModel> MM;

  //Define the location of eNodeB (0,0,0)
  MM = enbNodes.Get(0) -> GetObject<MobilityModel>();
  MM -> SetPosition(Vector3D(0,0,1));
  
  //Define the location of the first UE
  MM = ueNodes.Get(0) -> GetObject<MobilityModel>();
  MM -> SetPosition(Vector3D(1,0,1)); // distance between UE and eNodeB


	//LTE stack on UE and eNodeB
	NetDeviceContainer enbDevs;
	enbDevs = lteHelper->InstallEnbDevice (enbNodes);
	NetDeviceContainer ueDevs;
	ueDevs = lteHelper->InstallUeDevice (ueNodes);

	//Atributes for DL and UL
	uint32_t DlEarfcn = 100;
    uint32_t UlEarfcn = 18100;
    uint32_t DlBandwidth = 50; 
    uint32_t UlBandwidth = 50;
    lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (DlEarfcn));
    lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (UlEarfcn));
	lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (DlBandwidth));
    lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (UlBandwidth));

    //Install IP stack
    internet.Install (ueNodes);
  	Ipv4InterfaceContainer ueIpIface;
  	ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));

  	Ptr<Node> ueNode = ueNodes.Get(0);
    // Set the default gateway for the UE
    Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
    ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress(), 1);


    // Attach one UE per eNodeB
    lteHelper->Attach (ueDevs, enbDevs.Get(0)); //Attach UE to eNodeB



    uint16_t dlPort = 1000; //Port number
    //Sending application on the UE
    ApplicationContainer onOffApp;
    Ptr<Ipv4> ueIpv4;
    int32_t interface;
    Ipv4Address ueAddr;
    
    ueIpv4 = ueNodes.Get(0)->GetObject<Ipv4> ();
    interface =  ueIpv4->GetInterfaceForDevice (ueNodes.Get(0)->GetDevice (0)->GetObject <LteUeNetDevice> ());
    NS_ASSERT (interface >= 0);
    NS_ASSERT (ueIpv4->GetNAddresses (interface) == 1);
    ueAddr = ueIpv4->GetAddress (interface, 0).GetLocal ();

    OnOffHelper onOffHelper("ns3::UdpSocketFactory", InetSocketAddress(ueAddr, dlPort)); // where to send
    onOffHelper.SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=5000]"));
    onOffHelper.SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    onOffHelper.SetAttribute("DataRate", DataRateValue(DataRate("10.0Mbps"))); //Traffic Bit Rate
    onOffHelper.SetAttribute("PacketSize", UintegerValue(1024)); // Packet size
    onOffApp.Add(onOffHelper.Install(remoteHost));  //from where 
 
  	//Opening receiver socket on the UE
 	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  	Ptr<Socket> recvSink = Socket::CreateSocket (ueNodes.Get(0), tid);
  	InetSocketAddress local = InetSocketAddress (ueAddr, dlPort);
  	bool ipRecvTos = true;
  	recvSink->SetIpRecvTos (ipRecvTos);
  	bool ipRecvTtl = true;
  	recvSink->SetIpRecvTtl (ipRecvTtl);
  	recvSink->Bind (local);


  	lteHelper->EnableTraces ();

  Simulator::Stop (Seconds (20.0));
/////////////////////////////PCAP tracing/////////////////////////////   
   //Enable PCAP tracing for all devices
  //p2ph.EnablePcap("LTE", enbNodes, true);
  p2ph.EnablePcapAll("LTE");
  

  Simulator::Run ();
  Simulator::Destroy ();
}
