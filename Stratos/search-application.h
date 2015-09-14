#ifndef SEARCH_APPLICATION_H
#define SEARCH_APPLICATION_H

#include "ns3/internet-module.h"

#include <map>
#include <pthread.h>

#include "route-application.h"
#include "application-helper.h"
#include "service-application.h"
#include "search-error-header.h"
#include "results-application.h"
#include "position-application.h"
#include "ontology-application.h"
#include "search-request-header.h"
#include "search-response-header.h"
#include "neighborhood-application.h"

using namespace ns3;

class SearchApplication : public Application {

	public:
		static TypeId GetTypeId();

		SearchApplication();
		~SearchApplication();

	protected:
		virtual void DoInitialize();
		virtual void DoDispose();

	private:
		virtual void StartApplication();
		virtual void StopApplication();

	public:
		void CreateAndSendRequest();

	private:
		pthread_mutex_t mutex;
		std::map<std::pair<uint, double>, uint> parents;
		std::map<std::pair<uint, double>, int> seenRequests;
		std::map<std::pair<uint, double>, std::list<uint> > pendings;
		std::map<std::pair<uint, double>, std::list<SearchResponseHeader> > responses;

		Ptr<Socket> socket;
		Ipv4Address localAddress;
		Ptr<RouteApplication> routeManager;
		Ptr<ResultsApplication> resultsManager;
		Ptr<ServiceApplication> serviceManager;
		Ptr<PositionApplication> positionManager;
		Ptr<OntologyApplication> ontologyManager;
		Ptr<NeighborhoodApplication> neighborhoodManager;
		
		void ReceiveMessage(Ptr<Socket> socket);
		void SendBroadcastMessage(Ptr<Packet> packet);
		bool IsValidRequest(SearchRequestHeader request);
		void SendUnicastMessage(Ptr<Packet> packet, uint destinationAddress);

		SearchRequestHeader CreateRequest();
		void SendRequest(SearchRequestHeader requestHeader);
		void ForwardRequest(SearchRequestHeader requestHeader);
		void ReceiveRequest(Ptr<Packet> packet, uint senderAddress);
		std::pair<uint, double> GetRequestKey(SearchRequestHeader request);
		
		void ReceiveError(Ptr<Packet> packet, uint senderAddress);
		SearchErrorHeader CreateError(SearchRequestHeader request);
		std::pair<uint, double> GetRequestKey(SearchErrorHeader request);
		void SendError(SearchErrorHeader errorHeader, uint receiverAddress);
		void CreateAndSendError(SearchRequestHeader request, uint senderAddress);

		void SaveResponse(SearchResponseHeader response);
		void VerifyResponses(std::pair<uint, double> request);
		void CreateAndSaveResponse(SearchRequestHeader request);
		void SelectAndSendBestResponse(std::pair<uint, double> request);
		void ReceiveResponse(Ptr<Packet> packet, uint senderAddress);
		SearchResponseHeader CreateResponse(SearchRequestHeader request);
		std::pair<uint, double> GetRequestKey(SearchResponseHeader response);
		void SendResponse(SearchResponseHeader responseHeader, uint parent);
		SearchResponseHeader SelectBestResponse(std::list<SearchResponseHeader> responses);
};

class SearchHelper : public ApplicationHelper {

	public:
		SearchHelper();
};

#endif