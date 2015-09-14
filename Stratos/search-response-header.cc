#include "search-response-header.h"

#include "ns3/address-utils.h"

#include "utilities.h"

TypeId SearchResponseHeader::GetTypeId() {
	static TypeId typeId = TypeId("SearchResponseHeader")
		.SetParent<Header>()
		.AddConstructor<SearchResponseHeader>();
	return typeId;
}

TypeId SearchResponseHeader::GetInstanceTypeId() const {
	return GetTypeId();
}

uint32_t SearchResponseHeader::GetSerializedSize() const {
	return 22 + offeredServiceSize;
}

void SearchResponseHeader::Print(std::ostream &stream) const {
	stream << "Search response to " << requestAddress << " at " << requestTimestamp << ", response sent from " << responseAddress << " at " << distance << "m and " << hopDistance << " hops far, provided service is " << offeredService.service << " with " << offeredService.semanticDistance << " semantic distance";
}

uint32_t SearchResponseHeader::Deserialize(Buffer::Iterator start) {
	Buffer::Iterator i = start;
	distance = i.ReadU32();
	hopDistance = i.ReadU16();
	ReadFrom(i, requestAddress);
	ReadFrom(i, responseAddress);
	requestTimestamp = i.ReadU32();
	offeredService.semanticDistance = i.ReadU16();
	offeredServiceSize = i.ReadU16();
	char tmp[offeredServiceSize + 1];
	for(int j = 0; j < offeredServiceSize; j++) {
		tmp[j] = i.ReadU8();
	}
	tmp[offeredServiceSize] = '\0';
	offeredService.service = std::string(tmp);
	uint32_t size = i.GetDistanceFrom(start);
	return size;
}

void SearchResponseHeader::Serialize(Buffer::Iterator serializer) const {
	serializer.WriteU32(distance);
	serializer.WriteU16(hopDistance);
	WriteTo(serializer, requestAddress);
	WriteTo(serializer, responseAddress);
	serializer.WriteU32(requestTimestamp);
	serializer.WriteU16(offeredService.semanticDistance);
	serializer.WriteU16(offeredServiceSize);
	for(int i = 0; i < offeredServiceSize; i++) {
		serializer.WriteU8(offeredService.service.at(i));
	}
}

SearchResponseHeader::SearchResponseHeader() {
	offeredServiceSize = 1;
	offeredService.service = "0";
	requestAddress = Ipv4Address::GetAny();
	responseAddress = Ipv4Address::GetAny();
	distance = std::numeric_limits<double>::max();
	hopDistance = std::numeric_limits<int>::max();
	requestTimestamp = Utilities::GetCurrentRawDateTime();
	offeredService.semanticDistance = std::numeric_limits<int>::max();
}

double SearchResponseHeader::GetDistance() {
	return distance;
}

int SearchResponseHeader::GetHopDistance() {
	return hopDistance;
}

double SearchResponseHeader::GetRequestTimestamp() {
	return requestTimestamp;
}

Ipv4Address SearchResponseHeader::GetRequestAddress() {
	return requestAddress;
}

Ipv4Address SearchResponseHeader::GetResponseAddress() {
	return responseAddress;
}

OFFERED_SERVICE SearchResponseHeader::GetOfferedService() {
	return offeredService;
}

void SearchResponseHeader::SetDistance(double distance) {
	this->distance = distance;
}

void SearchResponseHeader::SetHopDistance(int hopDistance) {
	this->hopDistance = hopDistance;
}

void SearchResponseHeader::SetRequestTimestamp(double requestTimestamp) {
	this->requestTimestamp = requestTimestamp;
}

void SearchResponseHeader::SetRequestAddress(Ipv4Address requestAddress) {
	this->requestAddress = requestAddress;
}

void SearchResponseHeader::SetResponseAddress(Ipv4Address responseAddress) {
	this->responseAddress = responseAddress;
}

void SearchResponseHeader::SetOfferedService(OFFERED_SERVICE offeredService) {
	this->offeredService = offeredService;
	offeredServiceSize = offeredService.service.length();
}

std::ostream & operator<< (std::ostream & stream, SearchResponseHeader const & responseHeader) {
	responseHeader.Print(stream);
	return stream;
}