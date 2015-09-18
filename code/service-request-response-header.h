#ifndef SERVICE_REQUEST_RESPONSE_HEADER_H
#define SERVICE_REQUEST_RESPONSE_HEADER_H

#include "ns3/header.h"
#include "ns3/internet-module.h"

#include "definitions.h"

using namespace ns3;

class ServiceRequestResponseHeader : public Header {

	public:
		static TypeId GetTypeId();
		virtual TypeId GetInstanceTypeId() const;
		virtual uint32_t GetSerializedSize() const;
		virtual void Print(std::ostream &stream) const;
		virtual uint32_t Deserialize(Buffer::Iterator start);
		virtual void Serialize(Buffer::Iterator serializer) const;

	private:
		int serviceSize;

		Flag flag;
		std::string service;
		Ipv4Address senderAddress;
		Ipv4Address destinationAddress;

	public:
		ServiceRequestResponseHeader();

		Flag GetFlag();
		std::string GetService();
		Ipv4Address GetSenderAddress();
		Ipv4Address GetDestinationAddress();

		void SetFlag(Flag flag);
		void SetService(std::string service);
		void SetSenderAddress(Ipv4Address senderAddress);
		void SetDestinationAddress(Ipv4Address destinationAddress);
};
std::ostream & operator<< (std::ostream & stream, ServiceRequestResponseHeader const & requestResponseHeader);

#endif