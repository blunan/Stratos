#ifndef SEARCH_RESPONSE_HEADER_H
#define SEARCH_RESPONSE_HEADER_H

#include "ns3/header.h"
#include "ns3/internet-module.h"

#include "definitions.h"

using namespace ns3;

class SearchResponseHeader : public Header {

	public:
		static TypeId GetTypeId();
		virtual TypeId GetInstanceTypeId() const;
		virtual uint32_t GetSerializedSize() const;
		virtual void Print(std::ostream &stream) const;
		virtual uint32_t Deserialize(Buffer::Iterator start);
		virtual void Serialize(Buffer::Iterator serializer) const;

	private:
		int offeredServiceSize;

		double distance;
		int hopDistance;
		double requestTimestamp;
		Ipv4Address requestAddress;
		Ipv4Address responseAddress;
		OFFERED_SERVICE offeredService;

	public:
		SearchResponseHeader();

		double GetDistance();
		int GetHopDistance();
		double GetRequestTimestamp();
		Ipv4Address GetRequestAddress();
		Ipv4Address GetResponseAddress();
		OFFERED_SERVICE GetOfferedService();

		void SetDistance(double distance);
		void SetHopDistance(int hopDistance);
		void SetRequestTimestamp(double requestTimestamp);
		void SetRequestAddress(Ipv4Address requestAddress);
		void SetResponseAddress(Ipv4Address responseAddress);
		void SetOfferedService(OFFERED_SERVICE offeredService);
};
std::ostream & operator<< (std::ostream & stream, SearchResponseHeader const & responseHeader);

#endif