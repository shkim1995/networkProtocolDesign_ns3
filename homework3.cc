/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA    02111-1307    USA
 */

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


NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

class MyApp : public Application 
{
public:

    MyApp ();
    virtual ~MyApp();

    void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
    virtual void StartApplication (void);
    virtual void StopApplication (void);

    void ScheduleTx (void);
    void SendPacket (void);

    Ptr<Socket> m_socket;
    Address     m_peer;
    uint32_t    m_packetSize;
    uint32_t    m_nPackets;
    DataRate    m_dataRate;
    EventId     m_sendEvent;
    bool        m_running;
    uint32_t    m_packetsSent;
};

MyApp::MyApp ()
    : m_socket (0), 
      m_peer (), 
      m_packetSize (0), 
      m_nPackets (0), 
      m_dataRate (0), 
      m_sendEvent (), 
      m_running (false), 
      m_packetsSent (0)
{
}

MyApp::~MyApp()
{
    m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
    m_socket = socket;
    m_peer = address;
    m_packetSize = packetSize;
    m_nPackets = nPackets;
    m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
    m_running = true;
    m_packetsSent = 0;
    m_socket->Bind ();
    m_socket->Connect (m_peer);
    SendPacket ();
}

void 
MyApp::StopApplication (void)
{
    m_running = false;

    if (m_sendEvent.IsRunning ())
    {
        Simulator::Cancel (m_sendEvent);
	}

    if (m_socket)
    {
        m_socket->Close ();
    }
}

void 
MyApp::SendPacket (void)
{
    Ptr<Packet> packet = Create<Packet> (m_packetSize);
    m_socket->Send (packet);

    if (++m_packetsSent < m_nPackets)
    {
        ScheduleTx ();
    }
}

void 
MyApp::ScheduleTx (void)
{
    if (m_running)
    {
        Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
        m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

class MyQueue{

public:

	float time [100];
	int size [100];
	int num;
	int head;

	float bandwidth;

	MyQueue(){
		for(int i=0; i<100; i++){
			time[i] = 0;
			size[i] = 0;
		}
		num = 0;
		head = 0;
		bandwidth = 0;
	}

	void push(float _time, int _size){
		if(num==0){ // initialization
			time[head] = _time;
			size[head] = _size;
			head++;
			num++;
		}
		if(num!=100){ // not full yet
			time[head] = _time;
			size[head] = size[head-1]+_size;
			head = (head+1)%100;
			num++;
		}
		else{
			float t_interval = time[(head+99)%100] - time[head];
			int s_interval = size[(head+99)%100] - size[head];
			bandwidth = s_interval / t_interval;
		
    		//NS_LOG_UNCOND (s_interval<<" "<<t_interval<<" "<<time[head]<<" "<<time[(head+99)%100]);
			
			time[head] = _time;
			size[head] = size[(head+99)%100]+_size;
			head = (head+1)%100;
			
		}

	}
};


//Call Back Functions

int UDPsize = 0;
int TCPsize = 0;

const int startTime = 3;
const int finTime = 7;

MyQueue tcpWin;
MyQueue udpWin;

#define TIMING 0

static void 
callBack1 ( Ptr<const Packet> pkt, const Address& addr){
	
#if	TIMING
	udpWin.push(Simulator::Now().GetSeconds(), pkt->GetSize());

	if(udpWin.num==100){
    	NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << udpWin.bandwidth*8/1000000);
	}

#else
	if(Simulator::Now ().GetSeconds () < startTime
			|| Simulator::Now ().GetSeconds () >finTime) 
		return;

	UDPsize = UDPsize + pkt->GetSize();

#endif
}

static void 
callBack2 ( Ptr<const Packet> pkt, const Address& addr){

#if TIMING
	tcpWin.push(Simulator::Now().GetSeconds(), pkt->GetSize());

	if(tcpWin.num==100){
    	NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << tcpWin.bandwidth*8/1000000);
	}

#else
	if(Simulator::Now ().GetSeconds () < startTime
			|| Simulator::Now ().GetSeconds () >finTime) 
		return;

	TCPsize = TCPsize + pkt->GetSize();
#endif
}



int 
main (int argc, char *argv[])
{
    uint32_t nCsma = 3;
    uint32_t nWifi = 9;
	
	int thre = 0;
	int numUdp = 8;
	int tcp = 1;

    CommandLine cmd;

	cmd.AddValue ("thre", "RTS-CTS threshold", thre);
	cmd.AddValue ("numUdp", "# of UDP sources", numUdp);
	cmd.AddValue ("tcp", "0 if udp, 1 if tcp. else, both tcp and udp", tcp);
	
	cmd.Parse (argc,argv);

    // Check for valid number of csma or wifi nodes
    // 250 should be enough, otherwise IP addresses 
    // soon become an issue


    // CREATE p2p Nodes
    // p2pNodes.Get(0) : AP

    NodeContainer p2pNodes;
    p2pNodes.Create (2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("20Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install (p2pNodes);

	
	// CREATE CSMA nodes

    NodeContainer csmaNodes;
    csmaNodes.Add (p2pNodes.Get (1));
    csmaNodes.Create (nCsma);

    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
    csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install (csmaNodes);



	// Create Wifi Station Nodes

    NodeContainer wifiStaNodes;
    wifiStaNodes.Create (nWifi);
    NodeContainer wifiApNode = p2pNodes.Get (0);

    YansWifiChannelHelper channel;
	channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel"); 
	channel.AddPropagationLoss("ns3::JakesPropagationLossModel");
    
	YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
    phy.SetChannel (channel.Create ());

    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::ArfWifiManager",
			"RtsCtsThreshold", UintegerValue(thre));

    WifiMacHelper mac;
    Ssid ssid = Ssid ("ns-3-ssid");
    mac.SetType ("ns3::StaWifiMac",
         "Ssid", SsidValue (ssid),
         "ActiveProbing", BooleanValue (false));

    NetDeviceContainer staDevices;
    staDevices = wifi.Install (phy, mac, wifiStaNodes);

    mac.SetType ("ns3::ApWifiMac",
        "Ssid", SsidValue (ssid));

    NetDeviceContainer apDevices;
    apDevices = wifi.Install (phy, mac, wifiApNode);
	
	
	// Set Mobility

	// AP mobility
	MobilityHelper mobilityAP;
	mobilityAP.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator>();
	posAlloc->Add(Vector(0.0, 0.0, 0.0));
	mobilityAP.SetPositionAllocator(posAlloc);
	mobilityAP.Install(wifiApNode);

	// STA mobility

	MobilityHelper mobilitySTA;
	mobilitySTA.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue(Rectangle(-30, 30, -30, 30)));
	mobilitySTA.SetPositionAllocator("ns3::UniformDiscPositionAllocator", 
									"rho", DoubleValue(30.0));
	mobilitySTA.Install(wifiStaNodes);



	//Internet Stack

    InternetStackHelper stack;
    stack.Install (csmaNodes);
    stack.Install (wifiApNode);
    stack.Install (wifiStaNodes);

    Ipv4AddressHelper address;

    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign (p2pDevices);

    address.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign (csmaDevices);

    address.SetBase ("10.1.3.0", "255.255.255.0");
    address.Assign (staDevices);
    address.Assign (apDevices);


	
	//Install Sink Applications

	//TCP sink at Node 4
	uint16_t tcpSinkPort = 8080;
	Address tcpSinkAddress(InetSocketAddress(csmaInterfaces.GetAddress(3), tcpSinkPort));
	PacketSinkHelper tcpPacketSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), tcpSinkPort));
	ApplicationContainer tcpSinkApp = tcpPacketSinkHelper.Install(csmaNodes.Get(3));
	tcpSinkApp.Start(Seconds(0.));
	tcpSinkApp.Stop(Seconds(11.0));


	//UDP sink at Node 3
	uint16_t udpSinkPort = 8081;
	Address udpSinkAddress(InetSocketAddress(csmaInterfaces.GetAddress(2), udpSinkPort));
	PacketSinkHelper udpPacketSinkHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), udpSinkPort));
	ApplicationContainer udpSinkApp = udpPacketSinkHelper.Install(csmaNodes.Get(2));
	udpSinkApp.Start(Seconds(0.));
	udpSinkApp.Stop(Seconds(11.0));
	
	if(tcp==0){
    	udpSinkApp.Get(0)->TraceConnectWithoutContext ("Rx", MakeCallback(&callBack1));
	} else if (tcp==1){
		tcpSinkApp.Get(0)->TraceConnectWithoutContext ("Rx", MakeCallback(&callBack2));
	} else{
		tcpSinkApp.Get(0)->TraceConnectWithoutContext ("Rx", MakeCallback(&callBack2));
    	udpSinkApp.Get(0)->TraceConnectWithoutContext ("Rx", MakeCallback(&callBack1));
	}

	//TCP source at Node 6
	Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(wifiStaNodes.Get(0), TcpSocketFactory::GetTypeId());
    
	Ptr<MyApp> tcpApp = CreateObject<MyApp> ();
    tcpApp->Setup (ns3TcpSocket, tcpSinkAddress, 400, 100000, DataRate ("5Mbps"));
    wifiStaNodes.Get (0)->AddApplication (tcpApp);
    tcpApp->SetStartTime (Seconds (1.));
    tcpApp->SetStopTime (Seconds (10.));


	//UDP source at Node 7-14
	Ptr<Socket> ns3UdpSocket;
	
	for(int i=1; i<numUdp+1; i++){
		ns3UdpSocket = Socket::CreateSocket(wifiStaNodes.Get(i), UdpSocketFactory::GetTypeId());
    
		Ptr<MyApp> udpApp = CreateObject<MyApp> ();
	    udpApp->Setup (ns3UdpSocket, udpSinkAddress, 400, 100000, DataRate ("3Mbps"));
    	wifiStaNodes.Get (i)->AddApplication (udpApp);
    	udpApp->SetStartTime (Seconds (2.));
    	udpApp->SetStopTime (Seconds (7.));
	}
	

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    Simulator::Stop (Seconds (10.0));
    Simulator::Run ();
    Simulator::Destroy ();
#if TIMING 
#else
	cout<<"treshold : "<<thre<<", # of UDP sources : "<<numUdp
		<<"\nTCP : " << (TCPsize*8/(finTime-startTime)/1000000.0) 
		<< "Mbps, UDP : " 
		<< (UDPsize*8/(finTime-startTime)/1000000.0) << "Mbps\n\n";
#endif	
	return 0;
}
