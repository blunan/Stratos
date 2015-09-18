#include "stratos.h"

#include "ns3/wifi-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"

#include "utilities.h"
#include "definitions.h"
#include "route-application.h"
#include "search-application.h"
#include "service-application.h"
#include "results-application.h"
#include "ontology-application.h"
#include "position-application.h"
#include "schedule-application.h"
#include "neighborhood-application.h"

Stratos::Stratos(int argc, char *argv[]) {
	NUMBER_OF_MOBILE_NODES = 50; //0, 25, 50*, 100
	NUMBER_OF_REQUESTER_NODES = 4; //1, 2, 4*, 8, 16, 24, 32
	NUMBER_OF_PACKETS_TO_SEND = 10; //1, 10*, 20, 40, 60
	NUMBER_OF_SERVICES_OFFERED = 2; //1, 2*, 4, 8

	CommandLine cmd;
	cmd.AddValue("nMobile", "Number of mobile nodes.", NUMBER_OF_MOBILE_NODES);
	cmd.AddValue("nRequesters", "Number of requester nodes.", NUMBER_OF_REQUESTER_NODES);
	cmd.AddValue("nPackets", "Number of service packets to send.", NUMBER_OF_PACKETS_TO_SEND);
	cmd.AddValue("nServices", "Number of services offered by a node.", NUMBER_OF_SERVICES_OFFERED);
	cmd.Parse(argc, argv);
	SeedManager::SetSeed(time(NULL));
}

void Stratos::Run() {
	std::map<int, int> nodos;
	for(; nodos.size() < (uint) NUMBER_OF_REQUESTER_NODES;) {
		int nodo = Utilities::Random(0, TOTAL_NUMBER_OF_NODES - 1);
		nodos[nodo] = nodo;
	}
	Ptr<SearchApplication> searchApp;
	Ptr<ResultsApplication> resultsApp;
	Ptr<ResultsApplication> requesterResultsApp;
	for(std::map<int, int>::iterator i = nodos.begin(); i != nodos.end(); i++) {
		double requestTime = Utilities::Random(2, MAX_REQUEST_TIME);
		searchApp = DynamicCast<SearchApplication>(wifiNodes.Get(i->first)->GetApplication(3));
		requesterResultsApp = DynamicCast<ResultsApplication>(wifiNodes.Get(i->first)->GetApplication(7));
		Simulator::Schedule(Seconds(requestTime), &SearchApplication::CreateAndSendRequest, searchApp);
		for(int j = 0; j < TOTAL_NUMBER_OF_NODES; j++) {
			resultsApp = DynamicCast<ResultsApplication>(wifiNodes.Get(j)->GetApplication(7));
			Simulator::Schedule(Seconds(requestTime), &ResultsApplication::EvaluateNode, resultsApp, requesterResultsApp);
		}
	}
	Ptr<FlowMonitor> flowMonitor;
	FlowMonitorHelper flowHelper;
	flowMonitor = flowHelper.InstallAll();
	Simulator::Stop(Seconds(TOTAL_SIMULATION_TIME));
	Simulator::Run();
	double bytes = 0;
	std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats();
	for(std::map<FlowId, FlowMonitor::FlowStats>::iterator i = stats.begin(); i != stats.end(); i++) {
		bytes += i->second.txBytes;
	}
	std::cout << bytes << std::endl;
	Simulator::Destroy();
}

void Stratos::CreateNodes() {
	CreateMobileNodes();
	CreateStaticNodes();
	wifiNodes.Add(mobileNodes);
	wifiNodes.Add(staticNodes);
}

void Stratos::CreateDevices() {
	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
	wifiPhy.SetChannel(wifiChannel.Create());
	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default();
	wifiMac.SetType("ns3::AdhocWifiMac");
	WifiHelper wifi = WifiHelper::Default();
	wifiDevices = wifi.Install(wifiPhy, wifiMac, wifiNodes);
}

void Stratos::InstallInternetStack() {
	InternetStackHelper internetStack;
	internetStack.Install(wifiNodes);
	Ipv4AddressHelper addressHelper;
	addressHelper.SetBase("10.0.0.0", "255.0.0.0");
	addressHelper.Assign(wifiDevices);
}

void Stratos::InstallApplications() {
	ApplicationContainer applications;
	NeighborhoodHelper neigboors;
	applications.Add(neigboors.Install(wifiNodes));
	OntologyHelper ontology;
	ontology.SetAttribute("nServices", IntegerValue(NUMBER_OF_SERVICES_OFFERED));
	applications.Add(ontology.Install(wifiNodes));
	PositionHelper position;
	applications.Add(position.Install(wifiNodes));
	SearchHelper search;
	applications.Add(search.Install(wifiNodes));
	RouteHelper route;
	applications.Add(route.Install(wifiNodes));
	ServiceHelper service;
	service.SetAttribute("nPackets", IntegerValue(NUMBER_OF_PACKETS_TO_SEND));
	applications.Add(service.Install(wifiNodes));
	ScheduleHelper schedule;
	applications.Add(schedule.Install(wifiNodes));
	ResultsHelper results;
	applications.Add(results.Install(wifiNodes));
	applications.Start(Seconds(1));
	applications.Stop(Seconds(TOTAL_SIMULATION_TIME - 1));

}

void Stratos::CreateMobileNodes() {
	mobileNodes.Create(NUMBER_OF_MOBILE_NODES);
	for(int i = 0; i < NUMBER_OF_MOBILE_NODES; ++i) {
		std::ostringstream name;
		name << "Mobile Node - " << i;
		Names::Add(name.str(), mobileNodes.Get(i));
	}
	MobilityHelper mobility;
	Ptr<PositionAllocator> positionAllocator = GetPositionAllocator();
	mobility.SetPositionAllocator(positionAllocator);
	mobility.SetMobilityModel("ns3::RandomWaypointMobilityModel",
							"PositionAllocator", PointerValue(positionAllocator),
							"Speed", StringValue("ns3::UniformRandomVariable[Min=1|Max=4]"), //[1m/s, 4m/s]
							"Pause", StringValue("ns3::ConstantRandomVariable[Constant=40]")); //40s
	mobility.Install(mobileNodes);
}

void Stratos::CreateStaticNodes() {
	int nStaticNodes = TOTAL_NUMBER_OF_NODES - NUMBER_OF_MOBILE_NODES;
	staticNodes.Create(nStaticNodes);
	for(int i = 0; i < (nStaticNodes); ++i) {
		std::ostringstream name;
		name << "Static Node - " << i;
		Names::Add(name.str(), staticNodes.Get(i));
	}
	MobilityHelper mobility;
	Ptr<PositionAllocator> positionAllocator = GetPositionAllocator();
	mobility.SetPositionAllocator(positionAllocator);
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility.Install(staticNodes);
}

Ptr<PositionAllocator> Stratos::GetPositionAllocator() {
	Ptr<UniformRandomVariable> random = CreateObject<UniformRandomVariable>();
	random->SetAttribute("Min", DoubleValue(0));
	random->SetAttribute("Max", DoubleValue(MAX_DISTANCE));
	Ptr<RandomRectanglePositionAllocator> positionAllocator = CreateObject<RandomRectanglePositionAllocator>();
	positionAllocator->SetX(random);
	positionAllocator->SetY(random);
	return positionAllocator;
}