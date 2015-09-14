#include "service-error-header.h"

#include "ns3/address-utils.h"

#include "utilities.h"

TypeId ServiceErrorHeader::GetTypeId() {
	static TypeId typeId = TypeId("ServiceErrorHeader")
		.SetParent<Header>()
		.AddConstructor<ServiceErrorHeader>();
	return typeId;
}

TypeId ServiceErrorHeader::GetInstanceTypeId() const {
	return GetTypeId();
}

uint32_t ServiceErrorHeader::GetSerializedSize() const {
	return 10 + serviceSize;
}

void ServiceErrorHeader::Print(std::ostream &stream) const {
	stream << "Service error sent from " << senderAddress << " to " << destinationAddress << " for service " << service << ".";
}

uint32_t ServiceErrorHeader::Deserialize(Buffer::Iterator start) {
	Buffer::Iterator i = start;
	ReadFrom(i, senderAddress);
	ReadFrom(i, destinationAddress);
	serviceSize = i.ReadU16();
	char tmp[serviceSize + 1];
	for(int j = 0; j < serviceSize; j++) {
		tmp[j] = i.ReadU8();
	}
	tmp[serviceSize] = '\0';
	service = std::string(tmp);
	uint32_t size = i.GetDistanceFrom(start);
	return size;
}

void ServiceErrorHeader::Serialize(Buffer::Iterator serializer) const {
	WriteTo(serializer, senderAddress);
	WriteTo(serializer, destinationAddress);
	serializer.WriteU16(serviceSize);
	for(int i = 0; i < serviceSize; i++) {
		serializer.WriteU8(service.at(i));
	}
}

ServiceErrorHeader::ServiceErrorHeader() {
	service = "0";
	serviceSize = 1;
	senderAddress = Ipv4Address::GetAny();
	destinationAddress = Ipv4Address::GetAny();
}

std::string ServiceErrorHeader::GetService() {
	return service;
}

Ipv4Address ServiceErrorHeader::GetSenderAddress() {
	return senderAddress;
}

Ipv4Address ServiceErrorHeader::GetDestinationAddress() {
	return destinationAddress;
}

void ServiceErrorHeader::SetService(std::string service) {
	this->service = service;
	serviceSize = service.length();
}

void ServiceErrorHeader::SetSenderAddress(Ipv4Address senderAddress) {
	this->senderAddress = senderAddress;
}

void ServiceErrorHeader::SetDestinationAddress(Ipv4Address destinationAddress) {
	this->destinationAddress = destinationAddress;
}

std::ostream & operator<< (std::ostream & stream, ServiceErrorHeader const & requestResponseHeader) {
	requestResponseHeader.Print(stream);
	return stream;
}