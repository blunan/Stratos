#include "service-application.h"

#include "utilities.h"
#include "definitions.h"
#include "type-header.h"

NS_OBJECT_ENSURE_REGISTERED(ServiceApplication);

TypeId ServiceApplication::GetTypeId() {
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
}

ServiceApplication::~ServiceApplication() {
}

void ServiceApplication::DoInitialize() {
	routeManager = DynamicCast<RouteApplication>(GetNode()->GetApplication(4));
	resultsManager = DynamicCast<ResultsApplication>(GetNode()->GetApplication(6));
	ontologyManager = DynamicCast<OntologyApplication>(GetNode()->GetApplication(1));
	neighborhoodManager = DynamicCast<NeighborhoodApplication>(GetNode()->GetApplication(0));
	localAddress = GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
	socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
	socket->SetAllowBroadcast(false);
	InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), SERVICE_PORT);
	socket->Bind(local);
	Application::DoInitialize();
}

void ServiceApplication::DoDispose() {
	if(socket != NULL) {
		socket->Close();
	}
	Application::DoDispose();
}

void ServiceApplication::StartApplication() {
	socket->SetRecvCallback(MakeCallback(&ServiceApplication::ReceiveMessage, this));
}

void ServiceApplication::StopApplication() {
	if(socket != NULL) {
		socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
	}
}

void ServiceApplication::CreateAndSendRequest(Ipv4Address destinationAddress, std::string service) {
	ServiceRequestResponseHeader request = CreateRequest(destinationAddress, service);
	SendRequest(request);
	//std::cout << Utilities::GetCurrentRawDateTime() << " -> " << request << std::endl;
	status[std::make_pair(destinationAddress.Get(), service)] = STRATOS_START_SERVICE;
}

void ServiceApplication::ReceiveMessage(Ptr<Socket> socket) {
	Ptr<Packet> packet = socket->Recv();
	TypeHeader typeHeader;
	packet->RemoveHeader(typeHeader);
	if(!typeHeader.IsValid()) {
		return;
	}
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
			break;
	}
}

bool ServiceApplication::DoIProvideService(std::string service) {
	std::list<std::string>::iterator i;
	std::list<std::string> services = ontologyManager->GetOfferedServices();
	for(i = services.begin(); i != services.end(); i++) {
		if(service.compare(*i) == 0) {
			return true;
		}
	}
	return false;
}

bool ServiceApplication::IsInNeighborhood(uint destinationAddress) {
	std::list<uint> neighborhood = neighborhoodManager->GetNeighborhood();
	std::list<uint>::iterator i;
	for(i = neighborhood.begin(); i != neighborhood.end(); i++)  {
		if(destinationAddress == (*i)) {
			return true;
		}
	}
	return false;
}

void ServiceApplication::CancelService(std::pair<uint, std::string> key) {
	status[key] = STRATOS_NULL;
}

void ServiceApplication::SendUnicastMessage(Ptr<Packet> packet, uint destinationAddress) {
	InetSocketAddress remote = InetSocketAddress(Ipv4Address(destinationAddress), SERVICE_PORT);
	socket->Connect(remote);
	socket->Send(packet);
}

std::pair<uint, std::string> ServiceApplication::GetSenderKey(ServiceErrorHeader errorHeader) {
	std::string service = errorHeader.GetService();
	uint senderAddress = errorHeader.GetSenderAddress().Get();
	return std::make_pair(senderAddress, service);
}

void ServiceApplication::CreateAndExecuteSensingSchedule(std::list<SearchResponseHeader> responses) {
	int sensigTimePerNode = SENSING_TIME / responses.size();
	SearchResponseHeader response;
	while(responses.size() > 0) {
		response = responses.front();
		CreateAndSendRequest(response.GetResponseAddress(), response.GetOfferedService().service, sensigTimePerNode);
		//do magic here!!!
		responses.pop_front();
	}
}

std::pair<uint, std::string> ServiceApplication::GetSenderKey(ServiceRequestResponseHeader requestResponse) {
	std::string service = requestResponse.GetService();
	uint senderAddress = requestResponse.GetSenderAddress().Get();
	return std::make_pair(senderAddress, service);
}

std::pair<uint, std::string> ServiceApplication::GetDestinationKey(ServiceRequestResponseHeader requestResponse) {
	std::string service = requestResponse.GetService();
	uint destinationAddress = requestResponse.GetDestinationAddress().Get();
	return std::make_pair(destinationAddress, service);
}

void ServiceApplication::ReceiveRequest(Ptr<Packet> packet) {
	ServiceRequestResponseHeader requestHeader;
	packet->RemoveHeader(requestHeader);
	if(requestHeader.GetDestinationAddress() != localAddress) {
		ForwardRequest(requestHeader);
	} else if(DoIProvideService(requestHeader.GetService())) {
		Flag flag;
		Flag currentStatus = status[GetSenderKey(requestHeader)];
		if(currentStatus == STRATOS_NULL) {
			if(requestHeader.GetFlag() == STRATOS_START_SERVICE) {
				Simulator::Cancel(timers[GetSenderKey(requestHeader)]);
				flag = STRATOS_SERVICE_STARTED;
				status[GetSenderKey(requestHeader)] = STRATOS_DO_SERVICE;
				CreateAndSendResponse(requestHeader, flag);
			}
		} else if(currentStatus == STRATOS_DO_SERVICE) {
			if(requestHeader.GetFlag() == STRATOS_DO_SERVICE) {
				Simulator::Cancel(timers[GetSenderKey(requestHeader)]);
				/*flag = STRATOS_DO_SERVICE;
				int i = 1;
				for(; i <= NUMBER_OF_PACKETS_TO_SEND; i++) {
					Simulator::Schedule(MilliSeconds(PACKET_SEND_DELAY * i), &ServiceApplication::CreateAndSendResponse, this, requestHeader, flag);
				}
				flag = STRATOS_SERVICE_STOPPED;
				Simulator::Schedule(MilliSeconds(PACKET_SEND_DELAY * i), &ServiceApplication::CreateAndSendResponse, this, requestHeader, flag);
				return;*/
				if(packets[GetSenderKey(requestHeader)] < NUMBER_OF_PACKETS_TO_SEND) {
					flag = STRATOS_DO_SERVICE;
					packets[GetSenderKey(requestHeader)] += 1;
				} else {
					flag = STRATOS_SERVICE_STOPPED;
					status[GetSenderKey(requestHeader)] = STRATOS_SERVICE_STOPPED;
				}
			} else if(requestHeader.GetFlag() == STRATOS_STOP_SERVICE) {
				Simulator::Cancel(timers[GetSenderKey(requestHeader)]);
				flag = STRATOS_SERVICE_STOPPED;
				status[GetSenderKey(requestHeader)] = STRATOS_NULL;
			}
			CreateAndSendResponse(requestHeader, flag);
		}
	} else {
		CreateAndSendError(requestHeader);
	}
}

void ServiceApplication::SendRequest(ServiceRequestResponseHeader requestHeader) {
	uint nextHop = routeManager->GetRouteTo(requestHeader.GetDestinationAddress().Get());
	if(IsInNeighborhood(nextHop)) {
		//std::cout << requestHeader << std::endl;
		Ptr<Packet> packet = Create<Packet>(PACKET_LENGTH);
		packet->AddHeader(requestHeader);
		TypeHeader typeHeader(STRATOS_SERVICE_REQUEST);
		packet->AddHeader(typeHeader);
		Simulator::Schedule(Seconds(Utilities::GetJitter()), &ServiceApplication::SendUnicastMessage, this, packet, nextHop);
		timers[GetDestinationKey(requestHeader)] = Simulator::Schedule(Seconds(HELLO_TIME), &ServiceApplication::CancelService, this, GetDestinationKey(requestHeader));
	} else {
		status[GetDestinationKey(requestHeader)] = STRATOS_NULL;
	}
}

void ServiceApplication::ForwardRequest(ServiceRequestResponseHeader requestHeader) {
	uint nextHop = routeManager->GetRouteTo(requestHeader.GetDestinationAddress().Get());
	if(IsInNeighborhood(nextHop)) {
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
	SendRequest(CreateRequest(response, flag));
}

ServiceRequestResponseHeader ServiceApplication::CreateRequest(ServiceRequestResponseHeader response, Flag flag) {
	ServiceRequestResponseHeader request;
	request.SetFlag(flag);
	request.SetSenderAddress(localAddress);
	request.SetService(response.GetService());
	request.SetDestinationAddress(response.GetSenderAddress());
	return request;
}

ServiceRequestResponseHeader ServiceApplication::CreateRequest(Ipv4Address destinationAddress, std::string service) {
	ServiceRequestResponseHeader request;
	request.SetService(service);
	request.SetFlag(STRATOS_START_SERVICE);
	request.SetSenderAddress(localAddress);
	request.SetDestinationAddress(destinationAddress);
	return request;
}

void ServiceApplication::ReceiveError(Ptr<Packet> packet) {
	ServiceErrorHeader errorHeader;
	packet->RemoveHeader(errorHeader);
	if(errorHeader.GetDestinationAddress() == localAddress) {
		status[GetSenderKey(errorHeader)] = STRATOS_NULL;
	} else {
		SendError(errorHeader);
	}
}

void ServiceApplication::SendError(ServiceErrorHeader errorHeader) {
	uint nextHop = routeManager->GetRouteTo(errorHeader.GetDestinationAddress().Get());
	if(IsInNeighborhood(nextHop)) {
		//std::cout << errorHeader << std::endl;
		Ptr<Packet> packet = Create<Packet>();
		packet->AddHeader(errorHeader);
		TypeHeader typeHeader(STRATOS_SERVICE_ERROR);
		packet->AddHeader(typeHeader);
		Simulator::Schedule(Seconds(Utilities::GetJitter()), &ServiceApplication::SendUnicastMessage, this, packet, nextHop);
	}
}

void ServiceApplication::CreateAndSendError(ServiceRequestResponseHeader requestResponse) {
	SendError(CreateError(requestResponse));
}

ServiceErrorHeader ServiceApplication::CreateError(ServiceRequestResponseHeader requestResponse) {
	ServiceErrorHeader error;
	error.SetService(requestResponse.GetService());
	error.SetSenderAddress(requestResponse.GetDestinationAddress());
	error.SetDestinationAddress(requestResponse.GetSenderAddress());
	return error;
}

void ServiceApplication::ReceiveResponse(Ptr<Packet> packet) {
	ServiceRequestResponseHeader responseHeader;
	packet->RemoveHeader(responseHeader);
	if(responseHeader.GetDestinationAddress() != localAddress) {
		ForwardResponse(responseHeader);
	} else {
		Flag flag;
		Flag currentStatus = status[GetSenderKey(responseHeader)];
		if(currentStatus == STRATOS_START_SERVICE) {
			if(responseHeader.GetFlag() == STRATOS_SERVICE_STARTED) {
				Simulator::Cancel(timers[GetSenderKey(responseHeader)]);
				flag = STRATOS_DO_SERVICE; 
				status[GetSenderKey(responseHeader)] = STRATOS_DO_SERVICE;
				CreateAndSendRequest(responseHeader, flag);
			}
		} else if(currentStatus == STRATOS_DO_SERVICE) {
			if(responseHeader.GetFlag() == STRATOS_DO_SERVICE) {
				Simulator::Cancel(timers[GetSenderKey(responseHeader)]);
				/*if(packets[GetSenderKey(responseHeader)] < NUMBER_OF_PACKETS_TO_SEND) {
					packets[GetSenderKey(responseHeader)] += 1;
					std::cout << Now().GetMilliSeconds() << " -> " << localAddress << " received packet from " << responseHeader.GetSenderAddress() << std::endl;
					timers[GetDestinationKey(responseHeader)] = Simulator::Schedule(Seconds(HELLO_TIME), &ServiceApplication::CancelService, this, GetDestinationKey(responseHeader));
					return;
				}*/
				if(packets[GetSenderKey(responseHeader)] < NUMBER_OF_PACKETS_TO_SEND) {
					flag = STRATOS_DO_SERVICE;
					packets[GetSenderKey(responseHeader)] += 1;
					resultsManager->AddPacket(Now().GetMilliSeconds());
					//std::cout << Now().GetMilliSeconds() << " -> " << localAddress << " received packet from " << responseHeader.GetSenderAddress() << std::endl;
				} else {
					flag = STRATOS_STOP_SERVICE;
					status[GetSenderKey(responseHeader)] = STRATOS_STOP_SERVICE;
				}
				CreateAndSendRequest(responseHeader, flag);
			} else if(responseHeader.GetFlag() == STRATOS_SERVICE_STOPPED) {
				Simulator::Cancel(timers[GetSenderKey(responseHeader)]);
				status[GetSenderKey(responseHeader)] = STRATOS_NULL;
			}
		} else {
			CreateAndSendError(responseHeader);
		}
	}
}

void ServiceApplication::SendResponse(ServiceRequestResponseHeader responseHeader) {
	uint nextHop = routeManager->GetRouteTo(responseHeader.GetDestinationAddress().Get());
	if(IsInNeighborhood(nextHop)) {
		//std::cout << responseHeader << std::endl;
		Ptr<Packet> packet = Create<Packet>(PACKET_LENGTH);
		packet->AddHeader(responseHeader);
		TypeHeader typeHeader(STRATOS_SERVICE_RESPONSE);
		packet->AddHeader(typeHeader);
		Simulator::Schedule(Seconds(Utilities::GetJitter()), &ServiceApplication::SendUnicastMessage, this, packet, nextHop);
		timers[GetDestinationKey(responseHeader)] = Simulator::Schedule(Seconds(HELLO_TIME), &ServiceApplication::CancelService, this, GetDestinationKey(responseHeader));
	} else {
		status[GetDestinationKey(responseHeader)] = STRATOS_NULL;
	}
}

void ServiceApplication::ForwardResponse(ServiceRequestResponseHeader responseHeader) {
	uint nextHop = routeManager->GetRouteTo(responseHeader.GetDestinationAddress().Get());
	if(IsInNeighborhood(nextHop)) {
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
	SendResponse(CreateResponse(request, flag));
}

ServiceRequestResponseHeader ServiceApplication::CreateResponse(ServiceRequestResponseHeader request, Flag flag) {
	ServiceRequestResponseHeader response;
	response.SetFlag(flag);
	response.SetSenderAddress(localAddress);
	response.SetService(request.GetService());
	response.SetDestinationAddress(request.GetSenderAddress());
	return response;
}

ServiceHelper::ServiceHelper() {
	objectFactory.SetTypeId("ServiceApplication");
}