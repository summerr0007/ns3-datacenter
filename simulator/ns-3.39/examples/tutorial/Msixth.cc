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

#include "tutorial-app.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <fstream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MSixthScriptExample");

// ===========================================================================
//
//         node 0                 node 1
//   +----------------+    +----------------+
//   |    ns-3 TCP    |    |    ns-3 TCP    |
//   +----------------+    +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |
//   +----------------+    +----------------+
//   | point-to-point |    | point-to-point |
//   +----------------+    +----------------+
//           |                     |
//           +---------------------+
//                5 Mbps, 2 ms
//
//
// We want to look at changes in the ns-3 TCP congestion window.  We need
// to crank up a flow and hook the CongestionWindow attribute on the socket
// of the sender.  Normally one would use an on-off application to generate a
// flow, but this has a couple of problems.  First, the socket of the on-off
// application is not created until Application Start time, so we wouldn't be
// able to hook the socket (now) at configuration time.  Second, even if we
// could arrange a call after start time, the socket is not public so we
// couldn't get at it.
//
// So, we can cook up a simple version of the on-off application that does what
// we want.  On the plus side we don't need all of the complexity of the on-off
// application.  On the minus side, we don't have a helper, so we have to get
// a little more involved in the details, but this is trivial.
//
// So first, we create a socket and do the trace connect on it; then we pass
// this socket into the constructor of our simple application which we then
// install in the source node.
// ===========================================================================
//

/**
 * Congestion window change callback
 *
 * \param stream The output stream file.
 * \param oldCwnd Old congestion window.
 * \param newCwnd New congestion window.
 */
static void
CwndChange (Ptr<OutputStreamWrapper> stream , uint32_t oldCwnd , uint32_t newCwnd)
{
    NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
    *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd
        << std::endl;
}

/**
 * Rx drop callback
 *
 * \param file The output PCAP file.
 * \param p The dropped packet.
 */
static void
RxDrop (Ptr<PcapFileWrapper> file , Ptr<const Packet> p)
{
    NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
    file->Write (Simulator::Now () , p);
}

int
main (int argc , char* argv[])
{
    CommandLine cmd (__FILE__);
    cmd.Parse (argc , argv);

    NodeContainer serverNode;
    NodeContainer switchNode;
    NodeContainer nodes;
    uint16_t sinkPort = 8080;

    for (uint32_t i = 0;i < 4; i++) {
        Ptr<Node> node = CreateObject<Node> ();
        serverNode.Add (node);
        nodes.Add (node);
    }

    for (uint32_t i = 0;i < 2; i++) {
        Ptr<Node> sw = CreateObject<SwitchNode> ();
        switchNode.Add (sw);
        nodes.Add (sw);
        sw->SetAttribute ("EcnEnabled" , BooleanValue (true));
        sw->SetNodeType (1); //torNodes
    }

    QbbHelper qbb;
    qbb.SetDeviceAttribute ("DataRate" , StringValue ("5Mbps"));
    qbb.SetChannelAttribute ("Delay" , StringValue ("2ms"));
    char ipaddr[5][10] = { "10.0.1.0", "10.0.2.0", "10.0.3.0", "10.0.4.0", "10.0.5.0" };

    InternetStackHelper stack;
    stack.Install (nodes);


    for (uint32_t i = 0;i < 4;i++) {
        Ipv4AddressHelper address;
        address.SetBase (ipaddr[i] , "255.255.255.0");

        if (i < 2)
        {
            NetDeviceContainer devices = qbb.Install (serverNode.Get (i) , switchNode.Get (0));
            Ipv4InterfaceContainer interfaces = address.Assign (devices);
            BulkSendHelper bulkSendHelper ("ns3::TcpSocketFactory" , InetSocketAddress (Ipv4Address ("10.0.3.1") , sinkPort));
            ApplicationContainer bulkSendApp = bulkSendHelper.Install (serverNode.Get (i));
            bulkSendApp.Start (Seconds (0.));
            bulkSendApp.Stop (Seconds (20.));
        }
        else {
            NetDeviceContainer devices = qbb.Install (serverNode.Get (i) , switchNode.Get (1));
            Ipv4InterfaceContainer interfaces = address.Assign (devices);
            PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory" , InetSocketAddress (Ipv4Address::GetAny () , sinkPort));
            ApplicationContainer sinkApps = packetSinkHelper.Install (serverNode.Get (i));
            sinkApps.Start (Seconds (0.));
            sinkApps.Stop (Seconds (20.));
        }
    }

    NetDeviceContainer devices = qbb.Install (switchNode.Get (0) , switchNode.Get (1));
    Ipv4AddressHelper address;
    address.SetBase (ipaddr[4] , "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign (devices);

    PcapHelper pcapHelper;
    Ptr<PcapFileWrapper> file =
        pcapHelper.CreateFile ("Msixth.pcap" , std::ios::out , PcapHelper::DLT_PPP);
    // devices.Get (1)->TraceConnectWithoutContext ("PhyRxDrop" , MakeBoundCallback (&RxDrop , file));

    Simulator::Stop (Seconds (20));
    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}
