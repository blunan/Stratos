#include "schedule-application.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

#include "search-application.h"

NS_LOG_COMPONENT_DEFINE("ScheduleApplication");

NS_OBJECT_ENSURE_REGISTERED(ScheduleApplication);

TypeId ScheduleApplication::GetTypeId() {
	NS_LOG_FUNCTION_NOARGS();
	static TypeId typeId = TypeId("ScheduleApplication")
		.SetParent<Application>()
		.AddConstructor<ScheduleApplication>()
		.AddAttribute("nSchedule",
						"Max number of nodes in a schedule.",
						IntegerValue(3),
						MakeIntegerAccessor(&ScheduleApplication::MAX_SCHEDULE_SIZE),
						MakeIntegerChecker<int>());
	return typeId;
}

ScheduleApplication::ScheduleApplication() {
	NS_LOG_FUNCTION(this);
}

void ScheduleApplication::DoInitialize() {
	NS_LOG_FUNCTION(this);
	serviceManager = DynamicCast<ServiceApplication>(GetNode()->GetApplication(5));
	resultsManager = DynamicCast<ResultsApplication>(GetNode()->GetApplication(7));
	Application::DoInitialize();
}

ScheduleApplication::~ScheduleApplication() {
	NS_LOG_FUNCTION(this);
}

void ScheduleApplication::DoDispose() {
	NS_LOG_FUNCTION(this);
	Application::DoDispose();
}

void ScheduleApplication::StartApplication() {
	NS_LOG_FUNCTION(this);
	schedule.clear();
}

void ScheduleApplication::StopApplication() {
	NS_LOG_FUNCTION(this);
	schedule.clear();
}

void ScheduleApplication::ExecuteSchedule() {
	NS_LOG_FUNCTION(this);
	SearchResponseHeader node = schedule.front();
	NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> first node in schedule is: " << node);
	packetsByNode = serviceManager->NUMBER_OF_PACKETS_TO_SEND / schedule.size();
	NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> schedule size is " << schedule.size());
	NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> service packages per node in schedule are " << packetsByNode);
	int requestExtraPackets = serviceManager->NUMBER_OF_PACKETS_TO_SEND % schedule.size();
	NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> there are " << requestExtraPackets << " packets that will be added to this request to fill the " << serviceManager->NUMBER_OF_PACKETS_TO_SEND << " total packages needed");
	serviceManager->SetCallback(MakeCallback(&ScheduleApplication::ContinueSchedule, this));
	serviceManager->CreateAndSendRequest(node.GetResponseAddress(), node.GetOfferedService().service,packetsByNode + requestExtraPackets);
	schedule.pop_front();
}

void ScheduleApplication::CreateSchedule(std::list<SearchResponseHeader> responses) {
	NS_LOG_FUNCTION(this << &responses);
	scheduleSize = 1;
	SearchResponseHeader bestResponse = SearchApplication::SelectBestResponse(responses);
	int bestSemanticDistance = bestResponse.GetOfferedService().semanticDistance;
	resultsManager->SetResponseSemanticDistance(bestSemanticDistance);
	NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> only adding responses with semantic distance <= " << bestSemanticDistance);
	schedule.push_back(bestResponse);
	NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> added response to schedule: " << bestResponse);
	responses = DeleteElement(responses, bestResponse);
	while(!responses.empty() && scheduleSize < MAX_SCHEDULE_SIZE) {
		bestResponse = SearchApplication::SelectBestResponse(responses);
		if(bestResponse.GetOfferedService().semanticDistance > bestSemanticDistance) {
			NS_LOG_INFO(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> only adding responses with semantic distance <= " << bestSemanticDistance);
			responses = DeleteElement(responses, bestResponse);
			continue;
		}
		schedule.push_back(bestResponse);
		NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> added response to schedule: " << bestResponse);
		responses = DeleteElement(responses, bestResponse);
		scheduleSize++;
	}
	NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> schedule size is " << scheduleSize << " of " << MAX_SCHEDULE_SIZE);
}

std::list<SearchResponseHeader> ScheduleApplication::DeleteElement(std::list<SearchResponseHeader> list, SearchResponseHeader element) {
	NS_LOG_FUNCTION(&list << element);
	for(std::list<SearchResponseHeader>::iterator i = list.begin(); i != list.end(); i++) {
		if((*i).GetRequestTimestamp() == element.GetRequestTimestamp() && (*i).GetResponseAddress() == element.GetResponseAddress()) {
			NS_LOG_DEBUG("Deleting response from list: " << (*i));
			list.erase(i);
			break;
		}
	}
	return list;
}

void ScheduleApplication::ContinueSchedule() {
	NS_LOG_FUNCTION(this);
	if(!schedule.empty()) {
		SearchResponseHeader node = schedule.front();
		NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> next node in schedule is " << node);
		serviceManager->CreateAndSendRequest(node.GetResponseAddress(), node.GetOfferedService().service, packetsByNode);
		schedule.pop_front();
		return;
	}
	NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> no more nodes in schedule");
}

void ScheduleApplication::CreateAndExecuteSchedule(std::list<SearchResponseHeader> responses) {
	NS_LOG_FUNCTION(this << &responses);
	CreateSchedule(responses);
	ExecuteSchedule();
}

ScheduleHelper::ScheduleHelper() {
	NS_LOG_FUNCTION(this);
	objectFactory.SetTypeId("ScheduleApplication");
}