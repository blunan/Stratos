#include "search-request-header.h"

#include "ns3/address-utils.h"

#include "utilities.h"

TypeId SearchRequestHeader::GetTypeId() {
	static TypeId typeId = TypeId("SearchRequestHeader")
		.SetParent<Header>()
		.AddConstructor<SearchRequestHeader>();
	return typeId;
}

TypeId SearchRequestHeader::GetInstanceTypeId() const {
	return GetTypeId();
}

uint32_t SearchRequestHeader::GetSerializedSize() const {
	return 26 + requestedServiceSize;
}

void SearchRequestHeader::Print(std::ostream &stream) const {
	stream << "Search request sent from " << requestAddress << " at " << requestTimestamp << " in (" << requestPosition.x << ", " << requestPosition.y << ") with " << currentHops << " hops, looking for " << requestedService << " within " << maxDistanceAllowed << "m and " << maxHopsAllowed << " hops.";
}

uint32_t SearchRequestHeader::Deserialize(Buffer::Iterator start) {
	Buffer::Iterator i = start;
	currentHops = i.ReadU16();
	maxHopsAllowed = i.ReadU16();
	ReadFrom(i, requestAddress);
	requestTimestamp = i.ReadU32();
	requestPosition.x = i.ReadU32();
	requestPosition.y = i.ReadU32();
	maxDistanceAllowed = i.ReadU32();
	requestedServiceSize = i.ReadU16();
	char tmp[requestedServiceSize + 1];
	for(int j = 0; j < requestedServiceSize; j++) {
		tmp[j] = i.ReadU8();
	}
	tmp[requestedServiceSize] = '\0';
	requestedService = std::string(tmp);
	uint32_t size = i.GetDistanceFrom(start);
	return size;
}

void SearchRequestHeader::Serialize(Buffer::Iterator serializer) const {
	serializer.WriteU16(currentHops);
	serializer.WriteU16(maxHopsAllowed);
	WriteTo(serializer, requestAddress);
	serializer.WriteU32(requestTimestamp);
	serializer.WriteU32(requestPosition.x);
	serializer.WriteU32(requestPosition.y);
	serializer.WriteU32(maxDistanceAllowed);
	serializer.WriteU16(requestedServiceSize);
	for(int i = 0; i < requestedServiceSize; i++) {
		serializer.WriteU8(requestedService.at(i));
	}
}

SearchRequestHeader::SearchRequestHeader() {
	currentHops = 0;
	maxHopsAllowed = 0;
	requestPosition.x = 0;
	requestPosition.y = 0;
	requestedService = "0";
	maxDistanceAllowed = 0;
	requestedServiceSize = 1;
	requestAddress = Ipv4Address::GetAny();
	requestTimestamp = Utilities::GetCurrentRawDateTime();
}

int SearchRequestHeader::GetCurrentHops() {
	return currentHops;
}

int SearchRequestHeader::GetMaxHopsAllowed() {
	return maxHopsAllowed;
}

double SearchRequestHeader::GetRequestTimestamp() {
	return requestTimestamp;
}

POSITION SearchRequestHeader::GetRequestPosition() {
	return requestPosition;
}

double SearchRequestHeader::GetMaxDistanceAllowed() {
	return maxDistanceAllowed;
}

Ipv4Address SearchRequestHeader::GetRequestAddress() {
	return requestAddress;
}

std::string SearchRequestHeader::GetRequestedService() {
	return requestedService;
}

void SearchRequestHeader::SetCurrentHops(int currentHops) {
	this->currentHops = currentHops;
}

void SearchRequestHeader::SetMaxHopsAllowed(int maxHopsAllowed) {
	this->maxHopsAllowed = maxHopsAllowed;
}

void SearchRequestHeader::SetRequestTimestamp(double requestTimestamp) {
	this->requestTimestamp = requestTimestamp;
}

void SearchRequestHeader::SetRequestPosition(POSITION requestPosition) {
	this->requestPosition = requestPosition;
}

void SearchRequestHeader::SetRequestAddress(Ipv4Address requestAddress) {
	this->requestAddress = requestAddress;
}

void SearchRequestHeader::SetMaxDistanceAllowed(double maxDistanceAllowed) {
	this->maxDistanceAllowed = maxDistanceAllowed;
}

void SearchRequestHeader::SetRequestedService(std::string requestedService) {
	this->requestedService = requestedService;
	requestedServiceSize = requestedService.length();
}

std::ostream & operator<< (std::ostream & stream, SearchRequestHeader const & requestHeader) {
	requestHeader.Print(stream);
	return stream;
}