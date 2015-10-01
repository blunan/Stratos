#include "ontology-application.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

#include <limits>

#include "utilities.h"

NS_LOG_COMPONENT_DEFINE("OntologyApplication");

NS_OBJECT_ENSURE_REGISTERED(OntologyApplication);

const std::string OntologyApplication::SERVICES[] = {"0", "00", "000", "0000", "00000", "00001", "0001", "0002", "00020", "00021", "00022", "0003", "00030", "00031", "001", "0010", "00100", "0011", "00110", "00111", "01", "010", "0100", "01000", "0101", "01010", "01011", "01012", "01013", "011", "0110", "02", "020", "021", "022", "023"};

const int OntologyApplication::TOTAL_NUMBER_OF_SERVICES = 35;

TypeId OntologyApplication::GetTypeId() {
	NS_LOG_FUNCTION_NOARGS();
	static TypeId typeId = TypeId("OntologyApplication")
		.SetParent<Application>()
		.AddConstructor<OntologyApplication>()
		.AddAttribute("nServices",
						"Number of services offered by a node.",
						IntegerValue(2),
						MakeIntegerAccessor(&OntologyApplication::NUMBER_OF_SERVICES_OFFERED),
						MakeIntegerChecker<int>());
	return typeId;
}

OntologyApplication::OntologyApplication() {
	NS_LOG_FUNCTION(this);
}

OntologyApplication::~OntologyApplication() {
	NS_LOG_FUNCTION(this);
}

void OntologyApplication::DoInitialize() {
	NS_LOG_FUNCTION(this);
	bool alreadyOffered;
	std::string service;
	std::list<std::string>::iterator j;
	for(int i = 0; i < NUMBER_OF_SERVICES_OFFERED; i++) {
		alreadyOffered = false;
		service = GetRandomService();
		for(j = offeredServices.begin(); j != offeredServices.end(); j++) {
			if(service.compare(*j) == 0) {
				alreadyOffered = true;
				break;
			}
		}
		if(alreadyOffered) {
			i--;
		} else {
			NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> adding " << service << " to offered services");
			offeredServices.push_back(service);
		}
	}
	Application::DoInitialize();
}

void OntologyApplication::DoDispose() {
	NS_LOG_FUNCTION(this);
	offeredServices.clear();
	Application::DoDispose();
}

void OntologyApplication::StartApplication() {
	NS_LOG_FUNCTION(this);
}

void OntologyApplication::StopApplication() {
	NS_LOG_FUNCTION(this);
}

int OntologyApplication::SemanticDistance(std::string requiredService, std::string offeredService) {
	NS_LOG_FUNCTION(requiredService << offeredService);
	if(offeredService.compare(requiredService) == 0) {
		NS_LOG_DEBUG(requiredService << " - " << offeredService << " = 0, they are the same service");
		return 0;
	}
	std::string commonPrefix = GetCommonPrefix(requiredService, offeredService);
	NS_LOG_INFO(requiredService << " and " << offeredService << " common prefix is " << commonPrefix);
	if(offeredService.compare(commonPrefix) == 0) {
		NS_LOG_DEBUG(offeredService << " - " << commonPrefix << " = 0, " << offeredService << " is the first common ancestor with " << requiredService);
		return 0;
	}
	int distanceFromOfferedToCommon = offeredService.length() - commonPrefix.length();
	NS_LOG_INFO(offeredService << " - " << commonPrefix << " = " << distanceFromOfferedToCommon);
	int distanceFromRequiredToCommon = requiredService.length() - commonPrefix.length();
	NS_LOG_INFO(requiredService << " - " << commonPrefix << " = " << distanceFromRequiredToCommon);
	NS_LOG_DEBUG(requiredService << " - " << offeredService << " = " << (distanceFromOfferedToCommon > distanceFromRequiredToCommon ? distanceFromOfferedToCommon : distanceFromRequiredToCommon));
	return distanceFromOfferedToCommon > distanceFromRequiredToCommon ? distanceFromOfferedToCommon : distanceFromRequiredToCommon;
}

std::string OntologyApplication::GetCommonPrefix(std::string requiredService, std::string offeredService) {
	NS_LOG_FUNCTION(requiredService << offeredService);
	int minLength = requiredService.length() > offeredService.length() ? offeredService.length() : requiredService.length();
	NS_LOG_INFO("Shorter string between " << requiredService << " and " << offeredService << " is " << (requiredService.length() > offeredService.length() ? offeredService : requiredService));
	std::string commonPrefix;
	for(int i = 0; i < minLength; i++) {
		if(requiredService.at(i) == offeredService.at(i)) {
			commonPrefix.push_back(requiredService.at(i));
		} else {
			break;
		}
	}
	NS_LOG_DEBUG(requiredService << " and " << offeredService << " common prefix is " << commonPrefix);
	return commonPrefix;
}

std::string OntologyApplication::GetRandomService() {
	NS_LOG_FUNCTION_NOARGS();
	return SERVICES[(int) Utilities::Random(1, TOTAL_NUMBER_OF_SERVICES)];
}

OFFERED_SERVICE OntologyApplication::GetBestOfferedService(std::string requiredService, std::list<std::string> offeredServices) {
	//NS_LOG_FUNCTION(requiredService << offeredServices);
	std::string service;
	int semanticDistance;
	std::string bestOfferedService;
	int minSemanticDistance = std::numeric_limits<int>::max();
	std::list<std::string>::iterator i;
	for(i = offeredServices.begin(); i != offeredServices.end(); i++) {
		service = *i;
		semanticDistance = SemanticDistance(requiredService, service);
		if(semanticDistance < minSemanticDistance) {
			bestOfferedService = service;
			minSemanticDistance = semanticDistance;
		}
	}
	OFFERED_SERVICE result;
	result.service = bestOfferedService;
	result.semanticDistance = minSemanticDistance;
	return result;
}

bool OntologyApplication::DoIProvideService(std::string service) {
	NS_LOG_FUNCTION(this << service);
	std::list<std::string>::iterator i;
	for(i = offeredServices.begin(); i != offeredServices.end(); i++) {
		if(service.compare(*i) == 0) {
			return true;
		}
	}
	return false;
}

std::list<std::string> OntologyApplication::GetOfferedServices() {
	NS_LOG_FUNCTION(this);
	return offeredServices;
}

OFFERED_SERVICE OntologyApplication::GetBestOfferedService(std::string requiredService) {
	NS_LOG_FUNCTION(this << requiredService);
	std::string service;
	int semanticDistance;
	std::string bestOfferedService;
	int minSemanticDistance = std::numeric_limits<int>::max();
	std::list<std::string>::iterator i;
	for(i = offeredServices.begin(); i != offeredServices.end(); i++) {
		service = *i;
		semanticDistance = SemanticDistance(requiredService, service);
		if(semanticDistance < minSemanticDistance) {
			bestOfferedService = service;
			minSemanticDistance = semanticDistance;
		}
	}
	OFFERED_SERVICE result;
	result.service = bestOfferedService;
	result.semanticDistance = minSemanticDistance;
	return result;
}

OntologyHelper::OntologyHelper() {
	NS_LOG_FUNCTION(this);
	objectFactory.SetTypeId("OntologyApplication");
}