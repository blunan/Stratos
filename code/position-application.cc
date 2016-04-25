#include "position-application.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

NS_LOG_COMPONENT_DEFINE("PositionApplication");

NS_OBJECT_ENSURE_REGISTERED(PositionApplication);

TypeId PositionApplication::GetTypeId() {
	NS_LOG_FUNCTION_NOARGS();
	static TypeId typeId = TypeId("PositionApplication")
		.SetParent<Application>()
		.AddConstructor<PositionApplication>();
	return typeId;
}

PositionApplication::PositionApplication() {
	NS_LOG_FUNCTION(this);
}

PositionApplication::~PositionApplication() {
	NS_LOG_FUNCTION(this);
}

void PositionApplication::DoInitialize() {
	NS_LOG_FUNCTION(this);
	NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> installing RandomWaypointMobilityModel by default");
	mobility = GetNode()->GetObject<RandomWaypointMobilityModel>();
	if(mobility == NULL) {
		NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> replacing with ConstantPositionMobilityModel");
		mobility = GetNode()->GetObject<ConstantPositionMobilityModel>();
	}
	Application::DoInitialize();
}

void PositionApplication::DoDispose() {
	NS_LOG_FUNCTION(this);
	mobility = NULL;
	Application::DoDispose();
}

void PositionApplication::StartApplication() {
	NS_LOG_FUNCTION(this);
}

void PositionApplication::StopApplication() {
	NS_LOG_FUNCTION(this);
}

double PositionApplication::CalculateDistanceFromTo(POSITION from, POSITION to){
	NS_LOG_FUNCTION(&from << &to);
	double distance = sqrt(pow(to.x - from.x, 2) + pow(to.y - from.y, 2));
	NS_LOG_DEBUG("Distance from (" << from.x << ", " << from.y << ") to (" << to.x << ", " << to.y << ") is " << distance);
	return distance;
}

POSITION PositionApplication::GetCurrentPosition() {
	NS_LOG_FUNCTION(this);
	Vector rawPosition = mobility->GetPosition();
	POSITION position;
	position.x = rawPosition.x;
	position.y = rawPosition.y;
	NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> current position is (" << position.x << ", " << position.y << ")");
	return position;
}

PositionHelper::PositionHelper() {
	NS_LOG_FUNCTION(this);
	objectFactory.SetTypeId("PositionApplication");
}