#ifndef APPLICATION_HELPER_H
#define APPLICATION_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"

using namespace ns3;

class ApplicationHelper {

	protected:
		ObjectFactory objectFactory;

	public:
		Ptr<Application> InstallPriv(Ptr<Node> node) const;
		ApplicationContainer Install(Ptr<Node> node) const;
		ApplicationContainer Install(NodeContainer nodes) const;
		ApplicationContainer Install(std::string nodeName) const;
		void SetAttribute(std::string name, const AttributeValue &value);
};

#endif