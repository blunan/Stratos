#include "neighborhood-application.h"

#include "utilities.h"
#include "type-header.h"

NS_LOG_COMPONENT_DEFINE("NeighborhoodApplication");

NS_OBJECT_ENSURE_REGISTERED(NeighborhoodApplication);

TypeId NeighborhoodApplication::GetTypeId() {
	NS_LOG_FUNCTION_NOARGS();
	static TypeId typeId = TypeId("NeighborhoodApplication")
		.SetParent<Application>()
		.AddConstructor<NeighborhoodApplication>();
	return typeId;
}

NeighborhoodApplication::NeighborhoodApplication() {
	NS_LOG_FUNCTION(this);
}

void NeighborhoodApplication::DoInitialize() {
	NS_LOG_FUNCTION(this);
	neighborhood.clear();
	pthread_mutex_init(&mutex, NULL);
	socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
	InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), HELLO_PORT);
	socket->Bind(local);
	InetSocketAddress remote = InetSocketAddress(Ipv4Address::GetBroadcast(), HELLO_PORT);
	socket->SetAllowBroadcast(true);
	socket->Connect(remote);
	Application::DoInitialize();
}

NeighborhoodApplication::~NeighborhoodApplication() {
	NS_LOG_FUNCTION(this);
}

void NeighborhoodApplication::DoDispose() {
	NS_LOG_FUNCTION(this);
	neighborhood.clear();
	if(socket != NULL) {
		socket->Close();
	}
	pthread_mutex_destroy(&mutex);
	Application::DoDispose();
}

void NeighborhoodApplication::StartApplication() {
	NS_LOG_FUNCTION(this);
	ScheduleNextUpdate();
	socket->SetRecvCallback(MakeCallback(&NeighborhoodApplication::ReceiveHelloMessage, this));
	Simulator::Schedule(Seconds(Utilities::GetJitter()), &NeighborhoodApplication::ScheduleNextHelloMessage, this);
}

void NeighborhoodApplication::StopApplication() {
	NS_LOG_FUNCTION(this);
	Simulator::Cancel(sendHelloMessage);
	Simulator::Cancel(updateNeighborhood);
	if(socket != NULL) {
		socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
	}
}

void NeighborhoodApplication::SendHelloMessage() {
	NS_LOG_FUNCTION(this);
	Ptr<Packet> packet = Create<Packet>();
	TypeHeader typeHeader(STRATOS_HELLO);
	packet->AddHeader(typeHeader);
	socket->Send(packet);
	ScheduleNextHelloMessage();
}

void NeighborhoodApplication::UpdateNeighborhood() {
	NS_LOG_FUNCTION(this);
	double seconds;
	NEIGHBOR neighbor;
	std::list<NEIGHBOR>::iterator i;
	double now = Utilities::GetCurrentRawDateTime();
	pthread_mutex_lock(&mutex);
	for(i = neighborhood.begin(); i != neighborhood.end(); i++) {
		neighbor = *i;
		seconds = Utilities::GetSecondsElapsedSinceUntil(neighbor.lastSeen, now);
		if(seconds >= MAX_TIMES_NOT_SEEN * HELLO_TIME) {
			neighborhood.erase(i--);
			NS_LOG_INFO(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> the node " << Ipv4Address(neighbor.address) << " left neighborhood");
		}
	}
	pthread_mutex_unlock(&mutex);
	ScheduleNextUpdate();
}

void NeighborhoodApplication::ScheduleNextUpdate() {
	NS_LOG_FUNCTION(this);
	updateNeighborhood = Simulator::Schedule(Seconds(HELLO_TIME), &NeighborhoodApplication::UpdateNeighborhood, this);
}

void NeighborhoodApplication::ScheduleNextHelloMessage() {
	NS_LOG_FUNCTION(this);
	sendHelloMessage = Simulator::Schedule(Seconds(Utilities::GetJitter() + HELLO_TIME), &NeighborhoodApplication::SendHelloMessage, this);
}

void NeighborhoodApplication::ReceiveHelloMessage(Ptr<Socket> socket) {
	NS_LOG_FUNCTION(this << socket);
	Address sourceAddress;
	Ptr<Packet> packet = socket->RecvFrom(sourceAddress);
	TypeHeader typeHeader;
	packet->RemoveHeader(typeHeader);
	if(!typeHeader.IsValid() || typeHeader.GetType() != STRATOS_HELLO) {
		NS_LOG_WARN(this << " received invalid package, might be corrupted or not a hello package");
		return;
	}
	InetSocketAddress inetSourceAddress = InetSocketAddress::ConvertFrom(sourceAddress);
	Ipv4Address sender = inetSourceAddress.GetIpv4();
	NS_LOG_INFO(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> received hello from " << Ipv4Address(sender.Get()));
	AddUpdateNeighborhood(sender.Get(), Utilities::GetCurrentRawDateTime());
}

void NeighborhoodApplication::AddUpdateNeighborhood(uint address, double now) {
	NS_LOG_FUNCTION(this << address << now);
	pthread_mutex_lock(&mutex);
	std::list<NEIGHBOR>::iterator i;
	for(i = neighborhood.begin(); i != neighborhood.end(); i++) {
		if((*i).address == address) {
			NS_LOG_INFO(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> node " << Ipv4Address(address) << " was already in neighborhood, last seen will be updated");
			neighborhood.erase(i);
			break;
		}
	}
	NEIGHBOR neighbor;
	neighbor.lastSeen = now;
	neighbor.address = address;
	neighborhood.push_back(neighbor);
	NS_LOG_INFO(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> node " << Ipv4Address(address) << " last seen at " << now);
	pthread_mutex_unlock(&mutex);
}

std::list<uint> NeighborhoodApplication::GetNeighborhood() {
	NS_LOG_FUNCTION(this);
	std::list<uint> neighborhood;
	std::list<NEIGHBOR>::iterator i;
	pthread_mutex_lock(&mutex);
	for(i = this->neighborhood.begin(); i != this->neighborhood.end(); i++) {
		NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> node " << Ipv4Address((*i).address) << " is a neighbor");
		neighborhood.push_back((*i).address);
	}
	pthread_mutex_unlock(&mutex);
	return neighborhood;
}

bool NeighborhoodApplication::IsInNeighborhood(uint address) {
	NS_LOG_FUNCTION(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << address);
	std::list<uint> neighborhood = GetNeighborhood();
	std::list<uint>::iterator i;
	for(i = neighborhood.begin(); i != neighborhood.end(); i++)  {
		if(address == (*i)) {
			NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> node " << Ipv4Address(address) << " is in neighborhood");
			return true;
		}
	}
	NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> node " << Ipv4Address(address) << " is not in neighborhood");
	return false;
}

NeighborhoodHelper::NeighborhoodHelper() {
	NS_LOG_FUNCTION(this);
	objectFactory.SetTypeId("NeighborhoodApplication");
}