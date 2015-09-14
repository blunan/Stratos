#ifndef ONTOLOGY_APPLICATION_H
#define ONTOLOGY_APPLICATION_H

#include "definitions.h"
#include "application-helper.h"

using namespace ns3;

class OntologyApplication : public Application {

	public:
		static TypeId GetTypeId();
		
		OntologyApplication();
		~OntologyApplication();

	protected:
		virtual void DoInitialize();
		virtual void DoDispose();

	private:
		virtual void StartApplication();
		virtual void StopApplication();

	private:
		static const std::string SERVICES[];
		static const int TOTAL_NUMBER_OF_SERVICES;

		int NUMBER_OF_SERVICES_OFFERED;
		std::list<std::string> offeredServices;

		static int SemanticDistance(std::string requiredService, std::string offeredService);
		static std::string GetCommonPrefix(std::string requiredService, std::string offeredService);

	public:
		static std::string GetRandomService();
		static OFFERED_SERVICE GetBestOfferedService(std::string requiredService, std::list<std::string> offeredServices);

		bool DoIProvideService(std::string service);
		std::list<std::string> GetOfferedServices();
		OFFERED_SERVICE GetBestOfferedService(std::string requiredService);		
};

class OntologyHelper : public ApplicationHelper {

	public:
		OntologyHelper();
};

#endif