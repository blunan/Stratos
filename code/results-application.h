#ifndef RESULTS_APPLICATION_H
#define RESULTS_APPLICATION_H

#include "ns3/internet-module.h"

#include <map>
#include <pthread.h>

#include "definitions.h"
#include "application-helper.h"
#include "position-application.h"
#include "ontology-application.h"

using namespace ns3;

class ResultsApplication : public Application {

	public:
		static TypeId GetTypeId();

		ResultsApplication();
		~ResultsApplication();

	protected:
		virtual void DoInitialize();
		virtual void DoDispose();

	private:
		virtual void StartApplication();
		virtual void StopApplication();

	private:
		bool active;
		int foundSomeone;
		uint localAddress;
		double requestTime;
		pthread_mutex_t mutex;
		double requestDistance;
		POSITION requestPosition;
		std::string requestService;
		int responseSemanticDistance;
		std::list<double> packetsTime;
		std::map<uint, int> semanticDistances;
		Ptr<PositionApplication> positionManager;
		Ptr<OntologyApplication> ontologyManager;

	public:
		void Activate();
		void AddPacket(double receiveTime);
		void SetRequestTime(double requestTime);
		void SetRequestDistance(double requestDistance);
		void SetRequestPosition(POSITION requestPosition);
		void SetRequestService(std::string requestService);
		void EvaluateNode(Ptr<ResultsApplication> requester);
		void SetResponseSemanticDistance(int responseSemanticDistance);
		void Evaluate(uint nodeAddress, POSITION nodePosition, std::list<std::string> nodeServices);
};

class ResultsHelper : public ApplicationHelper {

	public:
		ResultsHelper();
};

#endif