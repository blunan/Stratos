#ifndef SEARCH_ERROR_HEADER_H
#define SEARCH_ERROR_HEADER_H

#include "ns3/header.h"
#include "ns3/internet-module.h"

using namespace ns3;

class SearchErrorHeader : public Header {

	public:
		static TypeId GetTypeId();
		virtual TypeId GetInstanceTypeId() const;
		virtual uint32_t GetSerializedSize() const;
		virtual void Print(std::ostream &stream) const;
		virtual uint32_t Deserialize(Buffer::Iterator start);
		virtual void Serialize(Buffer::Iterator serializer) const;

	private:
		double requestTimestamp;
		Ipv4Address requestAddress;

	public:
		SearchErrorHeader();

		double GetRequestTimestamp();
		Ipv4Address GetRequestAddress();

		void SetRequestTimestamp(double sentTimestamp);
		void SetRequestAddress(Ipv4Address sourceAddress);
};
std::ostream & operator<< (std::ostream & stream, SearchErrorHeader const & searchErrorHeader);

#endif