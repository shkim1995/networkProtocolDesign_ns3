#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include <string>
#define WINDOW_SIZE 100

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FifthScriptExample");

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

        Ptr<Socket>     m_socket;
        Address         m_peer;
        uint32_t        m_packetSize;
        uint32_t        m_nPackets;
        DataRate        m_dataRate;
        EventId         m_sendEvent;
        bool            m_running;
        uint32_t        m_packetsSent;
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

int packetCount_TCP = -1;
int packetCount_UDP = -1;

double txTime_TCP[WINDOW_SIZE + 1];
double txTime_UDP[WINDOW_SIZE + 1];

int dataCount_TCP[WINDOW_SIZE + 1];
int dataCount_UDP[WINDOW_SIZE + 1];

int dataCount_TCP_inst = 0;
int dataCount_UDP_inst = 0;

int TCPsize = 0;
int UDPsize = 0;

    static void
Rx_TCP (Ptr<const Packet> p, const Address &addr)
{
    /*packetCount_TCP++;
    txTime_TCP[packetCount_TCP % (WINDOW_SIZE + 1)] = Simulator::Now ().GetSeconds ();
    dataCount_TCP[packetCount_TCP % (WINDOW_SIZE + 1)] = p->GetSize ();

    if (packetCount_TCP == (WINDOW_SIZE - 1))
        for (int i = 0; i < WINDOW_SIZE; i++) dataCount_TCP_inst += dataCount_TCP[i];

    else if (packetCount_TCP >= WINDOW_SIZE)
    { 
        dataCount_TCP_inst += dataCount_TCP[packetCount_TCP % (WINDOW_SIZE + 1)];
        dataCount_TCP_inst -= dataCount_TCP[(packetCount_TCP + 1) % (WINDOW_SIZE + 1)];
        double time = txTime_TCP[packetCount_TCP % (WINDOW_SIZE + 1)] - txTime_TCP[(packetCount_TCP + 1) % (WINDOW_SIZE + 1)];
        double throughput = (double)dataCount_TCP_inst * 8.0 / time / 1000000.0;

        NS_LOG_UNCOND (txTime_TCP[packetCount_TCP % (WINDOW_SIZE + 1)] << "\t" << throughput << "\t" << dataCount_TCP[packetCount_TCP % (WINDOW_SIZE + 1)] << "\t" << dataCount_TCP[(packetCount_TCP + 1) % (WINDOW_SIZE + 1)] << "\t"  << dataCount_TCP_inst << "\t" << packetCount_TCP);
    }*/

	NS_LOG_UNCOND(Simulator::Now().GetSeconds());
	/*
	if(Simulator::Now ().GetSeconds () < 5
			|| Simulator::Now ().GetSeconds () >9) 
		return;

	TCPsize = TCPsize + p->GetSize();
	*/
}

    static void
Rx_UDP (Ptr<const Packet> p, const Address &addr)
{
	
	/*
	   if(Simulator::Now ().GetSeconds () < 5
			|| Simulator::Now ().GetSeconds () >9) 
		return;

	UDPsize = UDPsize + p->GetSize();
	*/
	NS_LOG_UNCOND(Simulator::Now().GetSeconds());
 /*   packetCount_UDP++;
    txTime_UDP[packetCount_UDP % (WINDOW_SIZE + 1)] = Simulator::Now ().GetSeconds ();
    dataCount_UDP[packetCount_UDP % (WINDOW_SIZE + 1)] = p->GetSize ();
   
    if (packetCount_UDP == (WINDOW_SIZE - 1))
        for (int i = 0; i < WINDOW_SIZE; i++) dataCount_UDP_inst += dataCount_UDP[i];

    else if (packetCount_UDP >= WINDOW_SIZE)
    { 
        dataCount_UDP_inst += dataCount_UDP[packetCount_UDP % (WINDOW_SIZE + 1)];
        dataCount_UDP_inst -= dataCount_UDP[(packetCount_UDP + 1) % (WINDOW_SIZE + 1)];
        double time = txTime_UDP[packetCount_UDP % (WINDOW_SIZE + 1)] - txTime_UDP[(packetCount_UDP + 1) % (WINDOW_SIZE + 1)];
        double throughput = (double)dataCount_UDP_inst * 8.0 / time / 1000000.0;

        NS_LOG_UNCOND (txTime_UDP[packetCount_UDP % (WINDOW_SIZE + 1)] << "\t" << throughput << "\t" << dataCount_UDP[packetCount_UDP % (WINDOW_SIZE + 1)] << "\t" << dataCount_UDP[(packetCount_UDP + 1) % (WINDOW_SIZE + 1)] << "\t"  << dataCount_UDP_inst);
    }
	*/
}

int rtsFailed = 0;

    static void
MacTxRtsFailed (Mac48Address addr)
{
     rtsFailed++;
}

    int 
main (int argc, char *argv[])
{
    int nWifi = 9;
    int thre = 0;
    int nTraffic = 2;
    bool traceTCP = true;
    bool traceUDP = true;
    bool traceFailed = false;

    CommandLine cmd;
    cmd.AddValue ("thre", "RTS/CTS threshold", thre);
    cmd.AddValue ("nTraffic", "# of UDP traffic", nTraffic);
    cmd.AddValue ("traceTCP", "enable tracing TCP traffic", traceTCP);
    cmd.AddValue ("traceUDP", "enable tracing UDP traffic", traceUDP);
    cmd.AddValue ("traceFailed", "enable tracing RTS packets that failed", traceFailed);

    cmd.Parse (argc, argv);

    NodeContainer csmaNodes;
    csmaNodes.Create (4);

    // 0th node of csmaNodes -> p2pNodes
    NodeContainer p2pNodes;
    p2pNodes.Add (csmaNodes.Get (0));
    p2pNodes.Create (1);

    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
    csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install (csmaNodes);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("20Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install (p2pNodes);

    NodeContainer wifiStaNodes;
    wifiStaNodes.Create (nWifi);
    NodeContainer wifiApNode;
    wifiApNode.Add (p2pNodes.Get (1));

    YansWifiChannelHelper channel;
    channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss ("ns3::JakesPropagationLossModel");

    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
    phy.SetChannel (channel.Create ());

    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::ArfWifiManager", "RtsCtsThreshold", UintegerValue (thre));
    
    WifiMacHelper mac;
    Ssid ssid = Ssid ("ns-3-ssid");
    mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue ("false"));

    NetDeviceContainer staDevices;
    staDevices = wifi.Install (phy, mac, wifiStaNodes);

    mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));

    NetDeviceContainer apDevices;
    apDevices = wifi.Install (phy, mac, wifiApNode);

    MobilityHelper mobility_0;

    mobility_0.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (0.0, 0.0, 0.0));
    mobility_0.SetPositionAllocator (positionAlloc);
    mobility_0.Install (wifiApNode);

    MobilityHelper mobility_1;

    mobility_1.SetPositionAllocator ("ns3::UniformDiscPositionAllocator", "rho", DoubleValue (30.0));
    mobility_1.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue (Rectangle (-30, 30, -30, 30)));
    mobility_1.Install (wifiStaNodes);

    InternetStackHelper stack;
    stack.Install (csmaNodes);
    stack.Install (wifiApNode);
    stack.Install (wifiStaNodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces = address.Assign (p2pDevices);
    address.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces = address.Assign (csmaDevices);
    address.SetBase ("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer wifiApInterfaces = address.Assign (apDevices);
    Ipv4InterfaceContainer wifiStaInterfaces = address.Assign (staDevices);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    uint16_t sinkPort_0 = 8080;
    Address sinkAddress_0 (InetSocketAddress (csmaInterfaces.GetAddress (3), sinkPort_0));
    PacketSinkHelper packetSinkHelper_tcp ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort_0));
    ApplicationContainer sinkApps_0 = packetSinkHelper_tcp.Install (csmaNodes.Get (3));
    sinkApps_0.Start (Seconds (0.));
    sinkApps_0.Stop (Seconds (15.));

    uint16_t sinkPort_1 = 9;
    Address sinkAddress_1 (InetSocketAddress (csmaInterfaces.GetAddress (2), sinkPort_1));
    PacketSinkHelper packetSinkHelper_udp ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort_1));
    ApplicationContainer sinkApps_1 = packetSinkHelper_udp.Install (csmaNodes.Get (2));
    sinkApps_1.Start (Seconds (0.));
    sinkApps_1.Stop (Seconds (15.));

    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (wifiStaNodes.Get (0), TcpSocketFactory::GetTypeId ());

    Ptr<MyApp> app_0 = CreateObject<MyApp> ();
    app_0->Setup (ns3TcpSocket, sinkAddress_0, 400, 100000, DataRate ("5Mbps"));
    wifiStaNodes.Get (0)->AddApplication (app_0);
    app_0->SetStartTime (Seconds (1.));
    app_0->SetStopTime (Seconds (10.));

    Ptr<Socket> ns3UdpSocket[nTraffic]; 
    Ptr<MyApp> app_1[nTraffic]; 

    for(int i = 0; i < nTraffic; i++)
    {
        ns3UdpSocket[i] = Socket::CreateSocket (wifiStaNodes.Get (i+1), UdpSocketFactory::GetTypeId ());
        app_1[i] = CreateObject<MyApp> ();
        app_1[i]->Setup (ns3UdpSocket[i], sinkAddress_1, 400, 100000, DataRate ("3Mbps"));
        app_1[i]->SetStartTime (Seconds (2.));
        app_1[i]->SetStopTime (Seconds (7.));
        wifiStaNodes.Get (i+1)->AddApplication (app_1[i]);
    }
    for(int i = 0; i < nTraffic; i++){
    }
	
	Ptr<WifiRemoteStationManager> st1 = DynamicCast<WifiNetDevice> (staDevices.Get (0))->GetRemoteStationManager();  

	st1->TraceConnectWithoutContext ("MacTxRtsFailed", MakeCallback(&MacTxRtsFailed));


    if (traceTCP) sinkApps_0.Get (0)->TraceConnectWithoutContext ("Rx", MakeCallback (&Rx_TCP));
    if (traceUDP) sinkApps_1.Get (0)->TraceConnectWithoutContext ("Rx", MakeCallback (&Rx_UDP));
    //if (traceFailed) staDevices.Get (0)->GetRemoteStationManager ()->TraceConnectWithoutContext ("MacTxRtsFailed", MakeCallback (&MacTxRtsFailed)); 
    Simulator::Stop (Seconds (16));
    Simulator::Run ();
    Simulator::Destroy ();

	std::cout<<"treshold : "<<thre<<", # of UDP sources : "<<nTraffic
		<<"\nTCP : " << (TCPsize*8/4/1000000.0) 
		<< "Mbps, UDP : " 
		<< (UDPsize*8/4/1000000.0) << "Mbps\n"
		<< "RxDrop : "<<rtsFailed<<"\n\n";
	return 0;
}

