#ifndef TYPE_HEADER_H
#define TYPE_HEADER_H

#include "ns3/header.h"

#include "definitions.h"

using namespace ns3;

class TypeHeader : public Header {

	public:
		static TypeId GetTypeId();
		virtual TypeId GetInstanceTypeId() const;
		virtual uint32_t GetSerializedSize() const;
		virtual void Print(std::ostream &stream) const;
		virtual uint32_t Deserialize(Buffer::Iterator start);
		virtual void Serialize(Buffer::Iterator serializer) const;

	private:
		bool isValid;
		MessageType messageType;

	public:
		TypeHeader(MessageType messageType = STRATOS);
		bool IsValid() const;
		MessageType GetType() const;	
};
std::ostream & operator<< (std::ostream & stream, TypeHeader const & typeHeader);

#endif