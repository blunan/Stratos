#ifndef ROUTE_APPLICATION_H
#define ROUTE_APPLICATION_H

#include <map>
#include <pthread.h>

#include "application-helper.h"

using namespace ns3;

class RouteApplication : public Application {

	public:
		static TypeId GetTypeId();
		
		RouteApplication();
		~RouteApplication();

	protected:
		virtual void DoInitialize();
		virtual void DoDispose();

	private:
		virtual void StartApplication();
		virtual void StopApplication();

	private:
		pthread_mutex_t mutex;
		std::map<uint, uint> routes;

	public:
		uint GetRouteTo(uint destination);
		void SetAsRouteTo(uint nextStep, uint destination);
};

class RouteHelper : public ApplicationHelper {

	public:
		RouteHelper();
};

#endif