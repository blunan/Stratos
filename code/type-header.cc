#include "type-header.h"

TypeId TypeHeader::GetTypeId() {
	static TypeId typeId = TypeId("TypeHeader")
		.SetParent<Header>()
		.AddConstructor<TypeHeader>();
	return typeId;
}

TypeId TypeHeader::GetInstanceTypeId() const {
	return GetTypeId();
}

uint32_t TypeHeader::GetSerializedSize() const {
	return 1;
}

void TypeHeader::Print(std::ostream &stream) const {
	switch(messageType) {
		case STRATOS_HELLO:
			stream << "Hello Message";
			break;
		case STRATOS_SEARCH_REQUEST:
			stream << "Search Request Message";
			break;
		case STRATOS_SEARCH_RESPONSE:
			stream << "Search Response Message";
			break;
		case STRATOS_SEARCH_ERROR:
			stream << "Search Error Message";
			break;
		case STRATOS_SERVICE_REQUEST:
			stream << "Service Request Message";
			break;
		case STRATOS_SERVICE_RESPONSE:
			stream << "Service Response Message";
			break;
		case STRATOS_SERVICE_ERROR:
			stream << "Service Error Message";
			break;
		default:
			stream << "Unknown Message";
	}
}

uint32_t TypeHeader::Deserialize(Buffer::Iterator start) {
	Buffer::Iterator i = start;
	uint8_t messageType = i.ReadU8();
	this->isValid = true;
	switch(messageType) {
		case STRATOS_HELLO:
		case STRATOS_SEARCH_REQUEST:
		case STRATOS_SEARCH_RESPONSE:
		case STRATOS_SEARCH_ERROR:
		case STRATOS_SERVICE_REQUEST:
		case STRATOS_SERVICE_RESPONSE:
		case STRATOS_SERVICE_ERROR:
			this->messageType = (MessageType) messageType;
			break;
		default:
			this->isValid = false;
	}
	uint32_t size = i.GetDistanceFrom(start);
	return size;
}

void TypeHeader::Serialize(Buffer::Iterator serializer) const {
	serializer.WriteU8(messageType);
}

TypeHeader::TypeHeader(MessageType messageType) {
	isValid = true,
	this->messageType = messageType;
}

bool TypeHeader::IsValid() const {
	return isValid;
}

MessageType TypeHeader::GetType() const {
	return messageType;
}

std::ostream & operator<< (std::ostream & stream, TypeHeader const & typeHeader) {
	typeHeader.Print(stream);
	return stream;
}