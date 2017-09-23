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
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

static void
PacketTx1 (Ptr<const Packet> p){
	NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "\t" << "A new packet is sent at Node 1");
}

static void
PacketTx2 (Ptr<const Packet> p){
	NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "\t" << "A new packet is sent at Node 2");
}

	int
main (int argc, char *argv[])
{
    Time::SetResolution (Time::NS);
    //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    //LogComponentEnable ("FirstScriptExample", LOG_INFO);

    NS_LOG_INFO("Creating Topology");

    uint32_t nPackets = 10;
    uint32_t Interval = 1;
    uint32_t PacketSize = 1024;

    CommandLine cmd;
    cmd.AddValue("nPackets", "Number of packets to echo", nPackets);
    cmd.AddValue("Interval", "Interval of packets to echo", Interval);
    cmd.AddValue("PacketSize", "size of packet to echo", PacketSize);
    cmd.Parse (argc, argv);

    NodeContainer nodes;
    nodes.Create (3);

    NodeContainer n0n1;
    n0n1.Add(nodes.Get(0));
    n0n1.Add(nodes.Get(1));
    NodeContainer n1n2;
    n1n2.Add(nodes.Get(1));
    n1n2.Add(nodes.Get(2));

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ms"));

    NetDeviceContainer d0d1;
    d0d1 = pointToPoint.Install (n0n1);

    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("100ms"));

    NetDeviceContainer d1d2;
    d1d2 = pointToPoint.Install (n1n2);

    InternetStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address1,address2;
    address1.SetBase ("10.1.1.0", "255.255.255.0");
    address2.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer i0i1 = address1.Assign (d0d1);
    Ipv4InterfaceContainer i1i2 = address2.Assign (d1d2);

    UdpEchoServerHelper echoServer (9);
    ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (10.0));

    UdpEchoClientHelper echoClient1 (i0i1.GetAddress (1), 9);
    echoClient1.SetAttribute ("MaxPackets", UintegerValue (nPackets));
    echoClient1.SetAttribute ("Interval", TimeValue (Seconds (Interval)));
    echoClient1.SetAttribute ("PacketSize", UintegerValue (PacketSize));

    ApplicationContainer clientApps1 = echoClient1.Install (nodes.Get (0));
	clientApps1.Get(0)->TraceConnectWithoutContext("Tx", MakeCallback(&PacketTx1));
    clientApps1.Start (Seconds (2.0));
    clientApps1.Stop (Seconds (10.0));

    UdpEchoClientHelper echoClient2 (i1i2.GetAddress (0), 9);
    echoClient2.SetAttribute ("MaxPackets", UintegerValue (nPackets));
    echoClient2.SetAttribute ("Interval", TimeValue (Seconds (Interval)));
    echoClient2.SetAttribute ("PacketSize", UintegerValue (PacketSize));

    ApplicationContainer clientApps2 = echoClient2.Install (nodes.Get (2));

	clientApps1.Get(0)->TraceConnectWithoutContext("Tx", MakeCallback(&PacketTx2));
    
	clientApps2.Start (Seconds (4.0));
    clientApps2.Stop (Seconds (10.0));

    pointToPoint.EnablePcapAll ("first");
    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}
