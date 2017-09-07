/*
Network topology

n0-----------------n1-----------------n2------------------n3
   p2p 10Mbps 1ms      csma 2Mbps 1ms      p2p 10Mbps 1ms


*/




#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/csma-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("Assignment");

int main(int argc, char *argv[]){

#if 0
	LogComponentEnable("Assignment", LOG_LEVEL_INFO);
#endif


	CommandLine cmd;
	cmd.Parse(argc, argv);
	

	//1: Create nodes : node0, 1, 2, 3
	NS_LOG_INFO("Create Nodes");
	
	NodeContainer c;
	c.Create(4);
	NodeContainer n0n1 = NodeContainer(c.Get(0), c.Get(1)); 
	NodeContainer n1n2 = NodeContainer(c.Get(1), c.Get(2)); 
	NodeContainer n2n3 = NodeContainer(c.Get(2), c.Get(3)); 
	
	InternetStackHelper internet;
	internet.Install(c);
	
	//2: Create Channels n0-n1, n1-n2, n2-n3
	NS_LOG_INFO("Create Channels");
	
	//p2p: n0-n1, n2-n3
	
	PointToPointHelper p2p;
 	
	p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  	p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
	
  	NetDeviceContainer d0d1 = p2p.Install (n0n1);
  	NetDeviceContainer d2d3 = p2p.Install (n2n3);
	
	p2p.EnablePcapAll("p2p");

	//CSMA: n1-n2
	
	CsmaHelper csma;
	csma.SetChannelAttribute ("DataRate", StringValue ("2Mbps"));
  	csma.SetChannelAttribute ("Delay", StringValue ("1ms"));
	/*
	csma.SetChannelAttribute("DataRate", DataRateValue(DataRate("2Mbps")));
	csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(1)));
	*/
	NetDeviceContainer d1d2 = csma.Install(n1n2);

	csma.EnablePcapAll("csma");


	//3: Add IP Addresses
	NS_LOG_INFO("Assign IP Addresses");
	
	Ipv4AddressHelper ipv4;
	ipv4.SetBase("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer i0i1 = ipv4.Assign(d0d1);
	
	ipv4.SetBase("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer i1i2 = ipv4.Assign(d1d2);
	
	ipv4.SetBase("10.1.3.0", "255.255.255.0");
	Ipv4InterfaceContainer i2i3 = ipv4.Assign(d2d3);

  	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


	//4: Set Applications
	NS_LOG_INFO("Create Applications");

	//onoff application
	cout<<i1i2.GetAddress(1)<<endl;
	OnOffHelper onoff1("ns3::UdpSocketFactory", Address(InetSocketAddress(i1i2.GetAddress(1), 9)));
	onoff1.SetAttribute("DataRate", StringValue("1Mbps"));	
	onoff1.SetAttribute("PacketSize", UintegerValue(1472));
	onoff1.SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
	onoff1.SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
	ApplicationContainer onoff_apps = onoff1.Install(c.Get(0));
	onoff_apps.Start(Seconds(2.0));
	onoff_apps.Stop(Seconds(5.0));

	//onoff application 2
	cout<<i1i2.GetAddress(0)<<endl;
	OnOffHelper onoff2("ns3::UdpSocketFactory", Address(InetSocketAddress(i1i2.GetAddress(0), 9)));
	onoff2.SetAttribute("DataRate", StringValue("1Mbps"));	
	onoff2.SetAttribute("PacketSize", UintegerValue(1472));
	onoff2.SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
	onoff2.SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
	
	ApplicationContainer onoff_apps2 = onoff2.Install(c.Get(3));
	onoff_apps2.Start(Seconds(4.0));
	onoff_apps2.Stop(Seconds(7.0));

	//PacketSink Applicaion 1

	PacketSinkHelper sink1 ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (i1i2.GetAddress(0), 9)));
  	ApplicationContainer sink_apps1 = sink1.Install (c.Get (1));
	sink_apps1.Start (Seconds (1.0));
	

	//PacketSink Applicaion 2

	PacketSinkHelper sink2 ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (i1i2.GetAddress(1), 9)));
  	ApplicationContainer sink_apps2 = sink2.Install (c.Get (2));
	sink_apps2.Start (Seconds (1.0));

  	NS_LOG_INFO ("Run Simulation.");
  	Simulator::Stop (Seconds (11));
  	Simulator::Run ();

	Simulator::Destroy();

	return 0;
}
