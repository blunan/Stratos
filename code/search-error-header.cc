#include "search-error-header.h"

#include "ns3/address-utils.h"

#include "utilities.h"

TypeId SearchErrorHeader::GetTypeId() {
	static TypeId typeId = TypeId("SearchErrorHeader")
		.SetParent<Header>()
		.AddConstructor<SearchErrorHeader>();
	return typeId;
}

TypeId SearchErrorHeader::GetInstanceTypeId() const {
	return GetTypeId();
}

uint32_t SearchErrorHeader::GetSerializedSize() const {
	return 8;
}

void SearchErrorHeader::Print(std::ostream &stream) const {
	stream << "Search error for request sent from " << requestAddress << " at " << requestTimestamp;
}

uint32_t SearchErrorHeader::Deserialize(Buffer::Iterator start) {
	Buffer::Iterator i = start;
	ReadFrom(i, requestAddress);
	requestTimestamp = i.ReadU32();
	uint32_t size = i.GetDistanceFrom(start);
	return size;
}

void SearchErrorHeader::Serialize(Buffer::Iterator serializer) const {
	WriteTo(serializer, requestAddress);
	serializer.WriteU32(requestTimestamp);
}

SearchErrorHeader::SearchErrorHeader() {
	requestAddress = Ipv4Address::GetAny();
	requestTimestamp = Utilities::GetCurrentRawDateTime();
}

double SearchErrorHeader::GetRequestTimestamp() {
	return requestTimestamp;
}

Ipv4Address SearchErrorHeader::GetRequestAddress() {
	return requestAddress;
}

void SearchErrorHeader::SetRequestTimestamp(double requestTimestamp) {
	this->requestTimestamp = requestTimestamp;
}

void SearchErrorHeader::SetRequestAddress(Ipv4Address requestAddress) {
	this->requestAddress = requestAddress;
}

std::ostream & operator<< (std::ostream & stream, SearchErrorHeader const & searchErrorHeader) {
	searchErrorHeader.Print(stream);
	return stream;
}