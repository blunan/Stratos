#ifndef SEARCH_REQUEST_HEADER_H
#define SEARCH_REQUEST_HEADER_H

#include "ns3/header.h"
#include "ns3/internet-module.h"

#include "definitions.h"

using namespace ns3;

class SearchRequestHeader : public Header {

	public:
		static TypeId GetTypeId();
		virtual TypeId GetInstanceTypeId() const;
		virtual uint32_t GetSerializedSize() const;
		virtual void Print(std::ostream &stream) const;
		virtual uint32_t Deserialize(Buffer::Iterator start);
		virtual void Serialize(Buffer::Iterator serializer) const;

	private:
		int requestedServiceSize;

		int currentHops; 
		int maxHopsAllowed;
		double requestTimestamp;
		POSITION requestPosition;
		double maxDistanceAllowed;
		Ipv4Address requestAddress;
		std::string requestedService;

	public:
		SearchRequestHeader();

		int GetCurrentHops();
		int GetMaxHopsAllowed();
		double GetRequestTimestamp();
		POSITION GetRequestPosition();
		double GetMaxDistanceAllowed();
		Ipv4Address GetRequestAddress();
		std::string GetRequestedService();

		void SetCurrentHops(int currentHops);
		void SetMaxHopsAllowed(int maxHopsAllowed);
		void SetRequestTimestamp(double requestTimestamp);
		void SetRequestPosition(POSITION requestPosition);
		void SetRequestAddress(Ipv4Address requestAddress);
		void SetMaxDistanceAllowed(double maxDistanceAllowed);
		void SetRequestedService(std::string requestedService);
};
std::ostream & operator<< (std::ostream & stream, SearchRequestHeader const & requestHeader);

#endif