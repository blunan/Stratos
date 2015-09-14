#ifndef NEIGHBORHOOD_APPLICATION_H
#define NEIGHBORHOOD_APPLICATION_H

#include "ns3/internet-module.h"

#include <pthread.h>

#include "definitions.h"
#include "application-helper.h"

using namespace ns3;

class NeighborhoodApplication : public Application {

	public:
		static TypeId GetTypeId();
		
		NeighborhoodApplication();
		~NeighborhoodApplication();

	protected:
		virtual void DoInitialize();
		virtual void DoDispose();

	private:
		virtual void StartApplication();
		virtual void StopApplication();

	private:
		Ptr<Socket> socket;
		pthread_mutex_t mutex;
		EventId sendHelloMessage;
		EventId updateNeighborhood;
		std::list<NEIGHBOR> neighborhood;

		void SendHelloMessage();
		void UpdateNeighborhood();
		void ScheduleNextUpdate();
		void ScheduleNextHelloMessage();
		void ReceiveHelloMessage(Ptr<Socket> socket);
		void AddUpdateNeighborhood(uint address, double time);

	public:
		std::list<uint> GetNeighborhood();
};

class NeighborhoodHelper : public ApplicationHelper {

	public:
		NeighborhoodHelper();
};

#endif