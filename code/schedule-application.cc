#include "schedule-application.h"

#include "search-application.h"

NS_OBJECT_ENSURE_REGISTERED(ScheduleApplication);

TypeId ScheduleApplication::GetTypeId() {
	static TypeId typeId = TypeId("ScheduleApplication")
		.SetParent<Application>()
		.AddConstructor<ScheduleApplication>();
	return typeId;
}

ScheduleApplication::ScheduleApplication() {
}

void ScheduleApplication::DoInitialize() {
	serviceManager = DynamicCast<ServiceApplication>(GetNode()->GetApplication(5));
	resultsManager = DynamicCast<ResultsApplication>(GetNode()->GetApplication(7));
	Application::DoInitialize();
}

ScheduleApplication::~ScheduleApplication() {
}

void ScheduleApplication::DoDispose() {
	Application::DoDispose();
}

void ScheduleApplication::StartApplication() {
	schedule.clear();
}

void ScheduleApplication::StopApplication() {
	schedule.clear();
}

void ScheduleApplication::ExecuteSchedule() {
	SearchResponseHeader node = schedule.front();
	packetsByNode = serviceManager->NUMBER_OF_PACKETS_TO_SEND / schedule.size();
	int requestPackets = packetsByNode + (serviceManager->NUMBER_OF_PACKETS_TO_SEND % schedule.size());
	serviceManager->CreateAndSendRequest(node.GetResponseAddress(), node.GetOfferedService().service,requestPackets);
	schedule.pop_front();
	//std::cout << node << std::endl;
}

void ScheduleApplication::CreateSchedule(std::list<SearchResponseHeader> responses) {
	scheduleSize = 1;
	SearchResponseHeader bestResponse = SearchApplication::SelectBestResponse(responses);
	int bestSemanticDistance = bestResponse.GetOfferedService().semanticDistance;
	schedule.push_back(bestResponse);
	responses = PopElement(responses, bestResponse);
	while(!responses.empty() && scheduleSize < MAX_SCHEDULE_SIZE) {
		bestResponse = SearchApplication::SelectBestResponse(responses);
		if(bestResponse.GetOfferedService().semanticDistance < bestSemanticDistance) {
			break;
		}
		schedule.push_back(bestResponse);
		responses = PopElement(responses, bestResponse);
		scheduleSize++;
	}
	//std::cout << "Best semantic distance " << bestSemanticDistance << std::endl;
	//std::cout << "Schedule size " << scheduleSize << std::endl;
	resultsManager->SetResponseSemanticDistance(bestSemanticDistance);
}

void ScheduleApplication::ContinueSchedule() {
	if(!schedule.empty()) {
		SearchResponseHeader node = schedule.front();
		serviceManager->CreateAndSendRequest(node.GetResponseAddress(), node.GetOfferedService().service,packetsByNode);
		schedule.pop_front();
		//std::cout << node << std::endl;
	}
}

void ScheduleApplication::CreateAndExecuteSchedule(std::list<SearchResponseHeader> responses) {
	CreateSchedule(responses);
	ExecuteSchedule();
}

std::list<SearchResponseHeader> ScheduleApplication::PopElement(std::list<SearchResponseHeader> list, SearchResponseHeader element) {
	std::list<SearchResponseHeader>::iterator i;
	for(std::list<SearchResponseHeader>::iterator i = list.begin(); i != list.end(); i++) {
		if((*i).GetRequestTimestamp() == element.GetRequestTimestamp() && (*i).GetResponseAddress() == element.GetResponseAddress()) {
			list.erase(i);
			break;
		}
	}
	return list;
}

ScheduleHelper::ScheduleHelper() {
	objectFactory.SetTypeId("ScheduleApplication");
}