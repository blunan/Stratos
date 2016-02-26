#include "service-application.h"

#include "utilities.h"
#include "definitions.h"
#include "type-header.h"

NS_LOG_COMPONENT_DEFINE("ServiceApplication");

NS_OBJECT_ENSURE_REGISTERED(ServiceApplication);

TypeId ServiceApplication::GetTypeId() {
	NS_LOG_FUNCTION_NOARGS();
	static TypeId typeId = TypeId("ServiceApplication")
		.SetParent<Application>()
		.AddConstructor<ServiceApplication>()
		.AddAttribute("nPackets",
						"Number of service packets to send.",
						IntegerValue(10),
						MakeIntegerAccessor(&ServiceApplication::NUMBER_OF_PACKETS_TO_SEND),
						MakeIntegerChecker<int>());
	return typeId;
}

ServiceApplication::ServiceApplication() {
	NS_LOG_FUNCTION(this);
}

ServiceApplication::~ServiceApplication() {
	NS_LOG_FUNCTION(this);
}

void ServiceApplication::DoInitialize() {
	NS_LOG_FUNCTION(this);
	routeManager = DynamicCast<RouteApplication>(GetNode()->GetApplication(4));
	resultsManager = DynamicCast<ResultsApplication>(GetNode()->GetApplication(7));
	ontologyManager = DynamicCast<OntologyApplication>(GetNode()->GetApplication(1));
	scheduleManager = DynamicCast<ScheduleApplication>(GetNode()->GetApplication(6));
	neighborhoodManager = DynamicCast<NeighborhoodApplication>(GetNode()->GetApplication(0));
	localAddress = GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
	socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
	socket->SetAllowBroadcast(false);
	InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), SERVICE_PORT);
	socket->Bind(local);
	Application::DoInitialize();
}

void ServiceApplication::DoDispose() {
	NS_LOG_FUNCTION(this);
	if(socket != NULL) {
		socket->Close();
	}
	Application::DoDispose();
}

void ServiceApplication::StartApplication() {
	NS_LOG_FUNCTION(this);
	socket->SetRecvCallback(MakeCallback(&ServiceApplication::ReceiveMessage, this));
}

void ServiceApplication::StopApplication() {
	NS_LOG_FUNCTION(this);
	if(socket != NULL) {
		socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
	}
}

void ServiceApplication::CreateAndSendRequest(Ipv4Address destinationAddress, std::string service, int requestPackets) {
	NS_LOG_FUNCTION(this << destinationAddress << service << requestPackets);
	ServiceRequestResponseHeader request = CreateRequest(destinationAddress, service);
	SendRequest(request);
	std::pair<uint, std::string> key = GetDestinationKey(request);
	maxPackets[key] = requestPackets;
	status[key] = STRATOS_START_SERVICE;
	NS_LOG_DEBUG(localAddress << " -> Service for " << destinationAddress << " requesting " << requestPackets << " packets is in state " << STRATOS_START_SERVICE);
}

void ServiceApplication::ReceiveMessage(Ptr<Socket> socket) {
	NS_LOG_FUNCTION(this << socket);
	Ptr<Packet> packet = socket->Recv();
	TypeHeader typeHeader;
	packet->RemoveHeader(typeHeader);
	if(!typeHeader.IsValid()) {
		NS_LOG_DEBUG(localAddress << " -> Received service message is invalid");
		return;
	}
	NS_LOG_DEBUG(localAddress << " -> Processing service message");
	switch(typeHeader.GetType()) {
		case STRATOS_SERVICE_ERROR:
			ReceiveError(packet);
			break;
		case STRATOS_SERVICE_REQUEST:
			ReceiveRequest(packet);
			break;
		case STRATOS_SERVICE_RESPONSE:
			ReceiveResponse(packet);
			break;
		default:
			NS_LOG_WARN(localAddress << " -> Service message is unknown!");
			break;
	}
}

void ServiceApplication::CancelService(std::pair<uint, std::string> key) {
	NS_LOG_FUNCTION(this << &key);
	status[key] = STRATOS_SERVICE_STOPPED;
	NS_LOG_DEBUG(localAddress << " -> Service for " << key.first << " is in state " << STRATOS_SERVICE_STOPPED);
	scheduleManager->ContinueSchedule();
}

void ServiceApplication::SendUnicastMessage(Ptr<Packet> packet, uint destinationAddress) {
	NS_LOG_FUNCTION(this << packet << destinationAddress);
	InetSocketAddress remote = InetSocketAddress(Ipv4Address(destinationAddress), SERVICE_PORT);
	socket->Connect(remote);
	socket->Send(packet);
}

std::pair<uint, std::string> ServiceApplication::GetSenderKey(ServiceErrorHeader errorHeader) {
	NS_LOG_FUNCTION(this << errorHeader);
	std::string service = errorHeader.GetService();
	uint senderAddress = errorHeader.GetSenderAddress().Get();
	NS_LOG_DEBUG(localAddress << " -> Sender key from error is [" << senderAddress << ", " << service << "] " << errorHeader);
	return std::make_pair(senderAddress, service);
}

std::pair<uint, std::string> ServiceApplication::GetSenderKey(ServiceRequestResponseHeader requestResponse) {
	NS_LOG_FUNCTION(this << requestResponse);
	std::string service = requestResponse.GetService();
	uint senderAddress = requestResponse.GetSenderAddress().Get();
	NS_LOG_DEBUG(localAddress << " -> Sender key is [" << senderAddress << ", " << service << "] " << requestResponse);
	return std::make_pair(senderAddress, service);
}

std::pair<uint, std::string> ServiceApplication::GetDestinationKey(ServiceRequestResponseHeader requestResponse) {
	NS_LOG_FUNCTION(this << requestResponse);
	std::string service = requestResponse.GetService();
	uint destinationAddress = requestResponse.GetDestinationAddress().Get();
	NS_LOG_DEBUG(localAddress << " -> Destination key is [" << destinationAddress << ", " << service << "] " << requestResponse);
	return std::make_pair(destinationAddress, service);
}

void ServiceApplication::ReceiveRequest(Ptr<Packet> packet) {
	NS_LOG_FUNCTION(this << packet);
	ServiceRequestResponseHeader requestHeader;
	packet->RemoveHeader(requestHeader);
	if(requestHeader.GetDestinationAddress() != localAddress) {
		ForwardRequest(requestHeader);
		return;
	} else if(!ontologyManager->DoIProvideService(requestHeader.GetService())) {
		CreateAndSendError(requestHeader);
		return;
	}
	Flag flag;
	std::pair<uint, std::string> requester = GetSenderKey(requestHeader);
	Simulator::Cancel(timers[requester]);
	Flag currentStatus = status[requester];
	switch(requestHeader.GetFlag()) {
		case STRATOS_START_SERVICE:
			if(currentStatus == STRATOS_NULL) {
				flag = STRATOS_SERVICE_STARTED;
				status[requester] = STRATOS_DO_SERVICE;
				CreateAndSendResponse(requestHeader, flag);
			} else {
				CreateAndSendError(requestHeader);
			}
		break;
		case STRATOS_DO_SERVICE:
			if(currentStatus == STRATOS_DO_SERVICE) {
				if(packets[requester] < NUMBER_OF_PACKETS_TO_SEND) {
					flag = STRATOS_DO_SERVICE;
					packets[requester] += 1;
				} else {
					flag = STRATOS_SERVICE_STOPPED;
					status[requester] = STRATOS_SERVICE_STOPPED;
				}
				CreateAndSendResponse(requestHeader, flag);
			} else {
				CreateAndSendError(requestHeader);
			}
		break;
		case STRATOS_STOP_SERVICE:
			flag = STRATOS_SERVICE_STOPPED;
			status[requester] = STRATOS_SERVICE_STOPPED;
			CreateAndSendResponse(requestHeader, flag);
			Simulator::Cancel(timers[requester]);
		break;
		default: break;
	}
}

void ServiceApplication::SendRequest(ServiceRequestResponseHeader requestHeader) {
	NS_LOG_FUNCTION(this << requestHeader);
	uint nextHop = routeManager->GetRouteTo(requestHeader.GetDestinationAddress().Get());
	if(neighborhoodManager->IsInNeighborhood(nextHop)) {
		//std::cout << requestHeader << std::endl;
		Ptr<Packet> packet = Create<Packet>(PACKET_LENGTH);
		packet->AddHeader(requestHeader);
		TypeHeader typeHeader(STRATOS_SERVICE_REQUEST);
		packet->AddHeader(typeHeader);
		Simulator::Schedule(Seconds(Utilities::GetJitter()), &ServiceApplication::SendUnicastMessage, this, packet, nextHop);
		timers[GetDestinationKey(requestHeader)] = Simulator::Schedule(Seconds(HELLO_TIME), &ServiceApplication::CancelService, this, GetDestinationKey(requestHeader));
	} else {
		CancelService(GetDestinationKey(requestHeader));
	}
}

void ServiceApplication::ForwardRequest(ServiceRequestResponseHeader requestHeader) {
	NS_LOG_FUNCTION(this << requestHeader);
	uint nextHop = routeManager->GetRouteTo(requestHeader.GetDestinationAddress().Get());
	if(neighborhoodManager->IsInNeighborhood(nextHop)) {
		Ptr<Packet> packet = Create<Packet>(PACKET_LENGTH);
		packet->AddHeader(requestHeader);
		TypeHeader typeHeader(STRATOS_SERVICE_REQUEST);
		packet->AddHeader(typeHeader);
		Simulator::Schedule(Seconds(Utilities::GetJitter()), &ServiceApplication::SendUnicastMessage, this, packet, nextHop);
	} else {
		CreateAndSendError(requestHeader);
	}
}

void ServiceApplication::CreateAndSendRequest(ServiceRequestResponseHeader response, Flag flag) {
	NS_LOG_FUNCTION(this << response << flag);
	SendRequest(CreateRequest(response, flag));
}

ServiceRequestResponseHeader ServiceApplication::CreateRequest(ServiceRequestResponseHeader response, Flag flag) {
	NS_LOG_FUNCTION(this << response << flag);
	ServiceRequestResponseHeader request;
	request.SetFlag(flag);
	request.SetSenderAddress(localAddress);
	request.SetService(response.GetService());
	request.SetDestinationAddress(response.GetSenderAddress());
	return request;
}

ServiceRequestResponseHeader ServiceApplication::CreateRequest(Ipv4Address destinationAddress, std::string service) {
	NS_LOG_FUNCTION(this << destinationAddress << service);
	ServiceRequestResponseHeader request;
	request.SetService(service);
	request.SetFlag(STRATOS_START_SERVICE);
	request.SetSenderAddress(localAddress);
	request.SetDestinationAddress(destinationAddress);
	return request;
}

void ServiceApplication::ReceiveError(Ptr<Packet> packet) {
	NS_LOG_FUNCTION(this << packet);
	ServiceErrorHeader errorHeader;
	packet->RemoveHeader(errorHeader);
	if(errorHeader.GetDestinationAddress() == localAddress) {
		CancelService(GetSenderKey(errorHeader));
	} else {
		SendError(errorHeader);
	}
}

void ServiceApplication::SendError(ServiceErrorHeader errorHeader) {
	NS_LOG_FUNCTION(this << errorHeader);
	uint nextHop = routeManager->GetRouteTo(errorHeader.GetDestinationAddress().Get());
	if(neighborhoodManager->IsInNeighborhood(nextHop)) {
		//std::cout << errorHeader << std::endl;
		Ptr<Packet> packet = Create<Packet>();
		packet->AddHeader(errorHeader);
		TypeHeader typeHeader(STRATOS_SERVICE_ERROR);
		packet->AddHeader(typeHeader);
		Simulator::Schedule(Seconds(Utilities::GetJitter()), &ServiceApplication::SendUnicastMessage, this, packet, nextHop);
	}
}

void ServiceApplication::CreateAndSendError(ServiceRequestResponseHeader requestResponse) {
	NS_LOG_FUNCTION(this << requestResponse);
	SendError(CreateError(requestResponse));
}

ServiceErrorHeader ServiceApplication::CreateError(ServiceRequestResponseHeader requestResponse) {
	NS_LOG_FUNCTION(this << requestResponse);
	ServiceErrorHeader error;
	error.SetService(requestResponse.GetService());
	error.SetSenderAddress(requestResponse.GetDestinationAddress());
	error.SetDestinationAddress(requestResponse.GetSenderAddress());
	return error;
}

void ServiceApplication::ReceiveResponse(Ptr<Packet> packet) {
	NS_LOG_FUNCTION(this << packet);
	ServiceRequestResponseHeader responseHeader;
	packet->RemoveHeader(responseHeader);
	if(responseHeader.GetDestinationAddress() != localAddress) {
		ForwardResponse(responseHeader);
		return;
	}
	Flag flag;
	std::pair<uint, std::string> responser = GetSenderKey(responseHeader);
	Simulator::Cancel(timers[responser]);
	Flag currentStatus = status[responser];;
	switch(responseHeader.GetFlag()) {
		case STRATOS_SERVICE_STARTED:
			if(currentStatus == STRATOS_START_SERVICE) {
				flag = STRATOS_DO_SERVICE;
				status[responser] = STRATOS_DO_SERVICE;
				CreateAndSendRequest(responseHeader, flag);
			} else {
				CreateAndSendError(responseHeader);
			}
		break;
		case STRATOS_DO_SERVICE:
			if(currentStatus == STRATOS_DO_SERVICE) {
				if((packets[responser] + 1) <= maxPackets[responser]) {
					flag = STRATOS_DO_SERVICE;
					packets[responser] += 1;
					resultsManager->AddPacket(Now().GetMilliSeconds());
					//std::cout << Now().GetMilliSeconds() << " -> " << localAddress << " received packet from " << responseHeader.GetSenderAddress() << std::endl;
				} else {
					flag = STRATOS_STOP_SERVICE;
					status[responser] = STRATOS_STOP_SERVICE;
				}
				CreateAndSendRequest(responseHeader, flag);
			} else {
				CreateAndSendError(responseHeader);
			}
		break;
		case STRATOS_SERVICE_STOPPED:
			CancelService(responser);
		break;
		default: break;
	}
}

void ServiceApplication::SendResponse(ServiceRequestResponseHeader responseHeader) {
	NS_LOG_FUNCTION(this << responseHeader);
	uint nextHop = routeManager->GetRouteTo(responseHeader.GetDestinationAddress().Get());
	if(neighborhoodManager->IsInNeighborhood(nextHop)) {
		//std::cout << responseHeader << std::endl;
		Ptr<Packet> packet = Create<Packet>(PACKET_LENGTH);
		packet->AddHeader(responseHeader);
		TypeHeader typeHeader(STRATOS_SERVICE_RESPONSE);
		packet->AddHeader(typeHeader);
		Simulator::Schedule(Seconds(Utilities::GetJitter()), &ServiceApplication::SendUnicastMessage, this, packet, nextHop);
		timers[GetDestinationKey(responseHeader)] = Simulator::Schedule(Seconds(HELLO_TIME), &ServiceApplication::CancelService, this, GetDestinationKey(responseHeader));
	} else {
		CancelService(GetDestinationKey(responseHeader));
	}
}

void ServiceApplication::ForwardResponse(ServiceRequestResponseHeader responseHeader) {
	NS_LOG_FUNCTION(this << responseHeader);
	uint nextHop = routeManager->GetRouteTo(responseHeader.GetDestinationAddress().Get());
	if(neighborhoodManager->IsInNeighborhood(nextHop)) {
		Ptr<Packet> packet = Create<Packet>(PACKET_LENGTH);
		packet->AddHeader(responseHeader);
		TypeHeader typeHeader(STRATOS_SERVICE_RESPONSE);
		packet->AddHeader(typeHeader);
		Simulator::Schedule(Seconds(Utilities::GetJitter()), &ServiceApplication::SendUnicastMessage, this, packet, nextHop);
	} else {
		CreateAndSendError(responseHeader);
	}
}

void ServiceApplication::CreateAndSendResponse(ServiceRequestResponseHeader request, Flag flag) {
	NS_LOG_FUNCTION(this << request << flag);
	SendResponse(CreateResponse(request, flag));
}

ServiceRequestResponseHeader ServiceApplication::CreateResponse(ServiceRequestResponseHeader request, Flag flag) {
	NS_LOG_FUNCTION(this << request << flag);
	ServiceRequestResponseHeader response;
	response.SetFlag(flag);
	response.SetSenderAddress(localAddress);
	response.SetService(request.GetService());
	response.SetDestinationAddress(request.GetSenderAddress());
	return response;
}

ServiceHelper::ServiceHelper() {
	NS_LOG_FUNCTION(this);
	objectFactory.SetTypeId("ServiceApplication");
}