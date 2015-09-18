#include "application-helper.h"

#include "ns3/names.h"

Ptr<Application> ApplicationHelper::InstallPriv(Ptr<Node> node) const {
	Ptr<Application> application = objectFactory.Create<Application>();
	node->AddApplication(application);
	return application;
}

ApplicationContainer ApplicationHelper::Install(Ptr<Node> node) const {
	return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer ApplicationHelper::Install(NodeContainer nodes) const {
	ApplicationContainer applications;
	for(NodeContainer::Iterator i = nodes.Begin(); i != nodes.End(); ++i) {
		applications.Add(InstallPriv(*i));
	}
	return applications;
}

ApplicationContainer ApplicationHelper::Install(std::string nodeName) const {
	Ptr<Node> node = Names::Find<Node>(nodeName);
	return ApplicationContainer(InstallPriv(node));
}

void ApplicationHelper::SetAttribute(std::string name, const AttributeValue &value) {
	objectFactory.Set(name, value);
}