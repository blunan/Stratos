#include "service-request-response-header.h"

#include "ns3/address-utils.h"

#include "utilities.h"

TypeId ServiceRequestResponseHeader::GetTypeId() {
	static TypeId typeId = TypeId("ServiceRequestResponseHeader")
		.SetParent<Header>()
		.AddConstructor<ServiceRequestResponseHeader>();
	return typeId;
}

TypeId ServiceRequestResponseHeader::GetInstanceTypeId() const {
	return GetTypeId();
}

uint32_t ServiceRequestResponseHeader::GetSerializedSize() const {
	return 11 + serviceSize;
}

void ServiceRequestResponseHeader::Print(std::ostream &stream) const {
	std::string flag;
	std::string type;
	switch(this->flag) {
		case STRATOS_START_SERVICE:
			type = "request";
			flag = "startService";
			break;
		case STRATOS_SERVICE_STARTED:
			type = "response";
			flag = "serviceStarted";
			break;
		case STRATOS_DO_SERVICE:
			type = "request/response";
			flag = "doService";
			break;
		case STRATOS_STOP_SERVICE:
			type = "request";
			flag = "stopService";
			break;
		case STRATOS_SERVICE_STOPPED:
			type = "response";
			flag = "serviceStopped";
			break;
		default:
			type = "unknown";
			flag = "unknown";
	}
	stream << "Service " << type << " sent from " << senderAddress << " to " << destinationAddress << " for service " << service << " with flag " << flag;
}

uint32_t ServiceRequestResponseHeader::Deserialize(Buffer::Iterator start) {
	Buffer::Iterator i = start;
	flag = (Flag) i.ReadU8();
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

void ServiceRequestResponseHeader::Serialize(Buffer::Iterator serializer) const {
	serializer.WriteU8(flag);
	WriteTo(serializer, senderAddress);
	WriteTo(serializer, destinationAddress);
	serializer.WriteU16(serviceSize);
	for(int i = 0; i < serviceSize; i++) {
		serializer.WriteU8(service.at(i));
	}
}

ServiceRequestResponseHeader::ServiceRequestResponseHeader() {
	flag = STRATOS_NULL;
	service = "0";
	serviceSize = 1;
	senderAddress = Ipv4Address::GetAny();
	destinationAddress = Ipv4Address::GetAny();
}

Flag ServiceRequestResponseHeader::GetFlag() {
	return flag;
}

std::string ServiceRequestResponseHeader::GetService() {
	return service;
}

Ipv4Address ServiceRequestResponseHeader::GetSenderAddress() {
	return senderAddress;
}

Ipv4Address ServiceRequestResponseHeader::GetDestinationAddress() {
	return destinationAddress;
}

void ServiceRequestResponseHeader::SetFlag(Flag flag) {
	this->flag = flag;
}

void ServiceRequestResponseHeader::SetService(std::string service) {
	this->service = service;
	serviceSize = service.length();
}

void ServiceRequestResponseHeader::SetSenderAddress(Ipv4Address senderAddress) {
	this->senderAddress = senderAddress;
}

void ServiceRequestResponseHeader::SetDestinationAddress(Ipv4Address destinationAddress) {
	this->destinationAddress = destinationAddress;
}

std::ostream & operator<< (std::ostream & stream, ServiceRequestResponseHeader const & requestResponseHeader) {
	requestResponseHeader.Print(stream);
	return stream;
}