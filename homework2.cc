#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"


using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("Week2Example");

int main (int argc, char *argv[])
{
	int nWifi = 4;
	uint32_t pktSize = 400;
	uint32_t thre = 0;
	uint32_t rate = 0;
	
	// GET COMMAND LINE

	CommandLine cmd;
	cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
	cmd.AddValue ("pktSize", "Size of UDP packet", pktSize);
	cmd.AddValue ("thre", "RTC/CTS threshold", thre);
	cmd.AddValue ("rate", "Rate Algoritm : 0(ARF), 1(CARA), else(Ideal)", rate);
	cmd.Parse (argc,argv);

	NodeContainer wifiApNode;
	wifiApNode.Create (1);
	
	NodeContainer wifiStaNodes;
	wifiStaNodes.Create (nWifi);

	NodeContainer nodes (wifiApNode, wifiStaNodes);




	//CREATE CHANNEL & PHY

	//YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
	YansWifiChannelHelper channel;
	channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel"); 
	channel.AddPropagationLoss("ns3::JakesPropagationLossModel");
	
	YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
	phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
	phy.SetChannel (channel.Create ());




	//CREATE MAC and make net devices

	NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();

	Ssid ssid = Ssid ("ns-3-ssid");
	mac.SetType ("ns3::ApWifiMac",
			"Ssid", SsidValue (ssid));

	WifiHelper wifi = WifiHelper::Default ();
	wifi.SetRemoteStationManager (rate==0 ? "ns3::ArfWifiManager" : rate==1 ? "ns3::CaraWifiManager" : "ns3::IdealWifiManager", 
			"RtsCtsThreshold", 
			UintegerValue (thre) );
	
	NetDeviceContainer apDevices = wifi.Install (phy, mac, wifiApNode);
	
	mac.SetType ("ns3::StaWifiMac",
			"Ssid", SsidValue (ssid),
			"ActiveProbing", BooleanValue (false));

	NetDeviceContainer staDevices = wifi.Install (phy, mac, wifiStaNodes);




	//SET MOBILITY
	
	//mobitity for AP

	MobilityHelper mobilityAP;
	mobilityAP.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator>();
	posAlloc->Add(Vector(0.0, 0.0, 0.0));
	mobilityAP.SetPositionAllocator(posAlloc);
	mobilityAP.Install(wifiApNode);

	cout<<"0"<<endl;
	
	//mobility for STA

	MobilityHelper mobilitySTA;
	mobilitySTA.SetMobilityModel ("ns3::RandomDirection2dMobilityModel", 
			"Bounds", RectangleValue(Rectangle(-30, 30, -30, 30)),
			"Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.5]"));
	/*
	UniformDiscPositionAllocator discAlloc;
	discAlloc.SetRho(30.0);
	*/
	mobilitySTA.SetPositionAllocator("ns3::UniformDiscPositionAllocator", 
									"rho", DoubleValue(30.0));
	mobilitySTA.Install(wifiStaNodes);
	
	cout<<"1"<<endl;

	InternetStackHelper stack;
	stack.Install (nodes);

	Ipv4AddressHelper address;
	address.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer wifiApInterface = address.Assign (apDevices);
	Ipv4InterfaceContainer wifiStaInterfaces = address.Assign (staDevices);

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	
	cout<<"2"<<endl;
	
	uint16_t port = 9;

	OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress (wifiApInterface.GetAddress(0), port)));
	onoff.SetAttribute ("OnTime",StringValue ("ns3::ConstantRandomVariable[Constant=1]") );
	onoff.SetAttribute ("OffTime",StringValue ("ns3::ConstantRandomVariable[Constant=0]") );
	onoff.SetAttribute ("PacketSize", UintegerValue(pktSize));
	onoff.SetAttribute ("DataRate", StringValue ("30Mbps"));
	
	
	cout<<"3"<<endl;

	for(int i = 0; i < nWifi; i++){
		ApplicationContainer app = onoff.Install(wifiStaNodes.Get(i));
		app.Start(Seconds(2));
		app.Stop(Seconds(5));
	}

//	LogComponentEnable("PacketSink", LOG_LEVEL_INFO);

	PacketSinkHelper udpsink ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
	ApplicationContainer sinkapp = udpsink.Install (wifiApNode.Get (0));
	sinkapp.Start (Seconds (1.0));
	sinkapp.Stop (Seconds (10.0));

	Simulator::Stop (Seconds (10.0));

	phy.EnablePcap ("week2_thre_"+to_string(thre)
			        +"_nWifi"+to_string(nWifi)
					+"_rate_"+(rate==0?"ARF" : rate==1?"CARA" : "Ideal"), apDevices.Get(0));

	Simulator::Run ();
	Simulator::Destroy ();
}
