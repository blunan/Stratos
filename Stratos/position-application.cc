#include "position-application.h"

NS_OBJECT_ENSURE_REGISTERED(PositionApplication);

TypeId PositionApplication::GetTypeId() {
	static TypeId typeId = TypeId("PositionApplication")
		.SetParent<Application>()
		.AddConstructor<PositionApplication>();
	return typeId;
}

PositionApplication::PositionApplication() {
}

PositionApplication::~PositionApplication() {
}

void PositionApplication::DoInitialize() {
	mobility = GetNode()->GetObject<RandomWaypointMobilityModel>();
	if(mobility == NULL) {
		mobility = GetNode()->GetObject<ConstantPositionMobilityModel>();
	}
	Application::DoInitialize();
}

void PositionApplication::DoDispose() {
	mobility = NULL;
	Application::DoDispose();
}

void PositionApplication::StartApplication() {
}

void PositionApplication::StopApplication() {
}

double PositionApplication::CalculateDistanceFromTo(POSITION from, POSITION to){
	return sqrt(pow(to.x - from.x, 2) + pow(to.y - from.y, 2));
}

POSITION PositionApplication::GetCurrentPosition() {
	Vector rawPosition = mobility->GetPosition();
	POSITION position;
	position.x = rawPosition.x;
	position.y = rawPosition.y;
	return position;
}

PositionHelper::PositionHelper() {
	objectFactory.SetTypeId("PositionApplication");
}