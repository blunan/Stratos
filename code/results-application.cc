#include "results-application.h"

#include "ns3/internet-module.h"

#include <limits>

#include "utilities.h"

NS_LOG_COMPONENT_DEFINE("ResultsApplication");

NS_OBJECT_ENSURE_REGISTERED(ResultsApplication);

TypeId ResultsApplication::GetTypeId() {
	NS_LOG_FUNCTION_NOARGS();
	static TypeId typeId = TypeId("ResultsApplication")
		.SetParent<Application>()
		.AddConstructor<ResultsApplication>();
	return typeId;
}

ResultsApplication::ResultsApplication() {
	NS_LOG_FUNCTION(this);
}

ResultsApplication::~ResultsApplication() {
	NS_LOG_FUNCTION(this);
}

void ResultsApplication::DoInitialize() {
	NS_LOG_FUNCTION(this);
	pthread_mutex_init(&mutex, NULL);
	localAddress = GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal().Get();
	ontologyManager = DynamicCast<OntologyApplication>(GetNode()->GetApplication(1));
	positionManager = DynamicCast<PositionApplication>(GetNode()->GetApplication(2));
	Application::DoInitialize();
}

void ResultsApplication::DoDispose() {
	NS_LOG_FUNCTION(this);
	pthread_mutex_destroy(&mutex);
	Application::DoDispose();
}

void ResultsApplication::StartApplication() {
	NS_LOG_FUNCTION(this);
	active = false;
	foundSomeone = 0;
	responseSemanticDistance = std::numeric_limits<int>::max();
}

void ResultsApplication::StopApplication() {
	NS_LOG_FUNCTION(this);
	if(active) {
		int success = 1;
		int nPackets = packetsTimes.size();
		double elapsedTimeFromRequestResponseToFirstServiceResponse = -1;
		NS_LOG_DEBUG(Ipv4Address(localAddress) << " -> received " << nPackets << " packets");
		if(nPackets > 0) {
			elapsedTimeFromRequestResponseToFirstServiceResponse = packetsTimes.front() - requestTime;
		}
		for(std::map<uint, int>::iterator i = semanticDistances.begin(); i != semanticDistances.end(); i++) {
			if(i->second < responseSemanticDistance) {
				NS_LOG_DEBUG(Ipv4Address(localAddress) << " -> at least the node " << Ipv4Address(i->first) << " was a better option providing a service with semantic distance " << i->second << " for service " << requestService);
				success = 0;
				break;
			}
		}
		NS_LOG_DEBUG(Ipv4Address(localAddress) << " -> results: \n\t elapsedTimeFromRequestResponseToFirstServiceResponse = " << elapsedTimeFromRequestResponseToFirstServiceResponse << "\n\t success = " << success << "\n\t foundSomeone = " << foundSomeone << "\n\t nPackets = " << nPackets);
		std::cout << elapsedTimeFromRequestResponseToFirstServiceResponse << "|" << success << "|" << foundSomeone << "|" << nPackets << std::endl;
	}
}

void ResultsApplication::Activate() {
	NS_LOG_FUNCTION(this);
	active = true;
	NS_LOG_DEBUG(Ipv4Address(localAddress) << " -> results will be printed");
}

void ResultsApplication::AddPacket(double receiveTime) {
	NS_LOG_FUNCTION(this);
	pthread_mutex_lock(&mutex);
	packetsTimes.push_back(receiveTime);
	pthread_mutex_unlock(&mutex);
	NS_LOG_DEBUG(Ipv4Address(localAddress) << " -> received service packet at " << receiveTime);
}

void ResultsApplication::SetRequestTime(double requestTime) {
	NS_LOG_FUNCTION(this);
	this->requestTime = requestTime;
	NS_LOG_DEBUG(Ipv4Address(localAddress) << " -> request response time is " << requestTime);
}

void ResultsApplication::SetRequestDistance(double requestDistance) {
	NS_LOG_FUNCTION(this);
	this->requestDistance = requestDistance;
	NS_LOG_DEBUG(Ipv4Address(localAddress) << " -> request response semantic ditance is " << requestDistance);
}

void ResultsApplication::SetRequestPosition(POSITION requestPosition) {
	NS_LOG_FUNCTION(this);
	this->requestPosition = requestPosition;
	NS_LOG_DEBUG(Ipv4Address(localAddress) << " -> my positiion when I did the request was (" << requestPosition.x << ", " << requestPosition.y << ")");
}

void ResultsApplication::SetRequestService(std::string requestService) {
	NS_LOG_FUNCTION(this);
	this->requestService = requestService;
	NS_LOG_DEBUG(Ipv4Address(localAddress) << " -> requested service was " << requestService);
}

void ResultsApplication::EvaluateNode(Ptr<ResultsApplication> requester) {
	NS_LOG_FUNCTION(this);
	POSITION position = positionManager->GetCurrentPosition();
	std::list<std::string> services = ontologyManager->GetOfferedServices();
	NS_LOG_DEBUG(Ipv4Address(localAddress) << " -> calling " << requester->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " to evaluate me");
	requester->Evaluate(localAddress, position, services);
}

void ResultsApplication::SetResponseSemanticDistance(int responseSemanticDistance) {
	NS_LOG_FUNCTION(this);
	foundSomeone = 1;
	this->responseSemanticDistance = responseSemanticDistance;
	NS_LOG_DEBUG(Ipv4Address(localAddress) << " -> requested service was " << requestService);
}

void ResultsApplication::Evaluate(uint nodeAddress, POSITION nodePosition, std::list<std::string> nodeServices) {
	NS_LOG_FUNCTION(this);
	if(localAddress == nodeAddress) {
		NS_LOG_DEBUG(Ipv4Address(localAddress) << " -> won't evaluate myself");
		return;
	}
	double distance = PositionApplication::CalculateDistanceFromTo(nodePosition, requestPosition);
	if(distance > requestDistance) {
		NS_LOG_DEBUG(Ipv4Address(localAddress) << " -> " << Ipv4Address(nodeAddress) << " is not in the area of interes");
		return;
	}
	OFFERED_SERVICE service = OntologyApplication::GetBestOfferedService(requestService, nodeServices);
	semanticDistances[nodeAddress] = service.semanticDistance;
	NS_LOG_DEBUG(Ipv4Address(localAddress) << " -> " << Ipv4Address(nodeAddress) << " best provided service for " << requestService << " is " << service.service << " with " << service.semanticDistance << " semantic distance");
}

ResultsHelper::ResultsHelper() {
	NS_LOG_FUNCTION(this);
	objectFactory.SetTypeId("ResultsApplication");
}