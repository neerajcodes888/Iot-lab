#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv6-module.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/lowpan-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
using namespace ns3;

int main (int argc, char *argv[])
{
  // Parse command line arguments here if needed

  // Create a network simulation object
  NS_LOG_INFO ("Creating the network...");
  NodeContainer nodes;
  nodes.Create (10);

  // Create a 6LowPAN network
  LowpanHelper lowpan;
  lowpan.Install (nodes);

  // Create an Internet stack and assign IPv6 addresses
  InternetStackHelper internetv6;
  internetv6.SetIpv4StackInstall (false);
  internetv6.Install (nodes);

  Ipv6AddressHelper ipv6;
  ipv6.SetBase (Ipv6Address ("2001:db8::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer interfaces = ipv6.Assign (lowpan.GetNetDevices ());

  // Enable global routing
  Ipv6StaticRoutingHelper routingHelper;
  Ptr<Ipv6StaticRouting> staticRouting = routingHelper.GetStaticRouting (nodes.Get (0)->GetObject<Ipv6> ());
  staticRouting->SetDefaultRoute (1); // The interface index of the default route

  // Create UDP applications
  uint16_t port = 9; // You can choose any available port
  UdpEchoServerHelper server (port);
  ApplicationContainer serverApps = server.Install (nodes.Get (0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper client (interfaces.GetAddress (0), port);
  client.SetAttribute ("MaxPackets", UintegerValue (1));
  client.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  client.SetAttribute ("PacketSize", UintegerValue (1024));
  ApplicationContainer clientApps = client.Install (nodes.Get (1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  // Enable packet capture for Wireshark
  AsciiTraceHelper ascii;
  lowpan.EnableAsciiAll (ascii.CreateFileStream ("6lowpan-packets.tr"));
  lowpan.EnablePcapAll ("6lowpan-packets");

  // Create a simulation and run it
  NS_LOG_INFO ("Running the simulation...");
  Simulator::Stop (Seconds (12.0)); // Set the simulation stop time as needed
  AnimationInterface anim("animation.xml"); // For visualization with NetAnim
  Simulator::Run ();

  // Clean up and close the simulation
  Simulator::Destroy ();
  NS_LOG_INFO ("Simulation completed.");

  return 0;
}
