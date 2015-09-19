#ifndef STRATOS_H
#define STRATOS_H

#include "ns3/network-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

class Stratos {

	private:
		NodeContainer wifiNodes;
		NodeContainer mobileNodes;
		NodeContainer staticNodes;
		NetDeviceContainer wifiDevices;

		int NUMBER_OF_MOBILE_NODES;
		int NUMBER_OF_PACKETS_TO_SEND;
		int NUMBER_OF_REQUESTER_NODES;
		int NUMBER_OF_SERVICES_OFFERED;

	public:
		Stratos(int argc, char *argv[]);
		void Run();
		void CreateNodes();
		void CreateDevices();
		void InstallInternetStack();
		void InstallApplications();

	private:
		void CreateMobileNodes();
		void CreateStaticNodes();
		Ptr<PositionAllocator> GetPositionAllocator();
};

#endif