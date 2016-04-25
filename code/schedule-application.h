#ifndef SCHEDULE_APPLICATION_H
#define SCHEDULE_APPLICATION_H

#include <map>
#include <pthread.h>

#include "application-helper.h"
#include "results-application.h"
#include "service-application.h"
#include "search-response-header.h"

using namespace ns3;

class ServiceApplication;

class ScheduleApplication : public Application {

	public:
		static TypeId GetTypeId();
		
		ScheduleApplication();
		~ScheduleApplication();

	protected:
		virtual void DoInitialize();
		virtual void DoDispose();

	private:
		virtual void StartApplication();
		virtual void StopApplication();

	private:
		int scheduleSize;
		int packetsByNode;
		Ptr<ResultsApplication> resultsManager;
		Ptr<ServiceApplication> serviceManager;
		std::list<SearchResponseHeader> schedule;

		static std::list<SearchResponseHeader> DeleteElement(std::list<SearchResponseHeader> list, SearchResponseHeader element);

		void ExecuteSchedule();
		void CreateSchedule(std::list<SearchResponseHeader> responses);

	public:
		void ContinueSchedule();
		void CreateAndExecuteSchedule(std::list<SearchResponseHeader> responses);
};

class ScheduleHelper : public ApplicationHelper {

	public:
		ScheduleHelper();
};

#endif