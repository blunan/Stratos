#include "results-application.h"

#include <limits>

#include "utilities.h"

NS_OBJECT_ENSURE_REGISTERED(ResultsApplication);

TypeId ResultsApplication::GetTypeId() {
	static TypeId typeId = TypeId("ResultsApplication")
		.SetParent<Application>()
		.AddConstructor<ResultsApplication>();
	return typeId;
}

ResultsApplication::ResultsApplication() {
}

ResultsApplication::~ResultsApplication() {
}

void ResultsApplication::DoInitialize() {
	pthread_mutex_init(&mutex, NULL);
	localAddress = GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal().Get();
	ontologyManager = DynamicCast<OntologyApplication>(GetNode()->GetApplication(1));
	positionManager = DynamicCast<PositionApplication>(GetNode()->GetApplication(2));
	Application::DoInitialize();
}

void ResultsApplication::DoDispose() {
	pthread_mutex_destroy(&mutex);
	Application::DoDispose();
}

void ResultsApplication::StartApplication() {
	active = false;
	foundSomeone = 0;
	responseSemanticDistance = std::numeric_limits<int>::max();
}

void ResultsApplication::StopApplication() {
	if(active) {
		int exito = 1;
		double tiempo = -1;
		int paquetes = packetsTime.size();
		if(paquetes > 0) {
			tiempo = packetsTime.front() - requestTime;
		}
		for(std::map<uint, int>::iterator i = semanticDistances.begin(); i != semanticDistances.end(); i++) {
			if(i->second < responseSemanticDistance) {
				exito = 0;
				break;
			}
		}
		std::cout << tiempo << "|" << exito << "|" << foundSomeone << "|" << paquetes << std::endl;
	}
}

void ResultsApplication::Activate() {
	active = true;
}

void ResultsApplication::AddPacket(double receiveTime) {
	pthread_mutex_lock(&mutex);
	packetsTime.push_back(receiveTime);
	pthread_mutex_unlock(&mutex);
}

void ResultsApplication::SetRequestTime(double requestTime) {
	this->requestTime = requestTime;
}

void ResultsApplication::SetRequestDistance(double requestDistance) {
	this->requestDistance = requestDistance;
}

void ResultsApplication::SetRequestPosition(POSITION requestPosition) {
	this->requestPosition = requestPosition;
}

void ResultsApplication::SetRequestService(std::string requestService) {
	this->requestService = requestService;
}

void ResultsApplication::EvaluateNode(Ptr<ResultsApplication> requester) {
	POSITION position = positionManager->GetCurrentPosition();
	std::list<std::string> services = ontologyManager->GetOfferedServices();
	uint address = GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal().Get();
	requester->Evaluate(address, position, services);
}

void ResultsApplication::SetResponseSemanticDistance(int responseSemanticDistance) {
	foundSomeone = 1;
	this->responseSemanticDistance = responseSemanticDistance;
}

void ResultsApplication::Evaluate(uint nodeAddress, POSITION nodePosition, std::list<std::string> nodeServices) {
	if(localAddress == nodeAddress) {
		return;
	}
	double distance = positionManager->CalculateDistanceFromTo(nodePosition, requestPosition);
	if(distance > requestDistance) {
		return;
	}
	OFFERED_SERVICE service = ontologyManager->GetBestOfferedService(requestService, nodeServices);
	semanticDistances[nodeAddress] = service.semanticDistance;
}

ResultsHelper::ResultsHelper() {
	objectFactory.SetTypeId("ResultsApplication");
}