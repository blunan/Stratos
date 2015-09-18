#include "search-application.h"

#include "utilities.h"
#include "definitions.h"
#include "type-header.h"

NS_OBJECT_ENSURE_REGISTERED(SearchApplication);

TypeId SearchApplication::GetTypeId() {
	static TypeId typeId = TypeId("SearchApplication")
		.SetParent<Application>()
		.AddConstructor<SearchApplication>();
	return typeId;
}

SearchApplication::SearchApplication() {
}

SearchApplication::~SearchApplication() {
}

void SearchApplication::DoInitialize() {
	pthread_mutex_init(&mutex, NULL);
	routeManager = DynamicCast<RouteApplication>(GetNode()->GetApplication(4));
	serviceManager = DynamicCast<ServiceApplication>(GetNode()->GetApplication(5));
	resultsManager = DynamicCast<ResultsApplication>(GetNode()->GetApplication(6));
	ontologyManager = DynamicCast<OntologyApplication>(GetNode()->GetApplication(1));
	positionManager = DynamicCast<PositionApplication>(GetNode()->GetApplication(2));
	neighborhoodManager = DynamicCast<NeighborhoodApplication>(GetNode()->GetApplication(0));
	localAddress = GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
	socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
	InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), SEARCH_PORT);
	socket->Bind(local);
	Application::DoInitialize();
}

void SearchApplication::DoDispose() {
	if(socket != NULL) {
		socket->Close();
	}
	pthread_mutex_destroy(&mutex);
	Application::DoDispose();
}

void SearchApplication::StartApplication() {
	socket->SetRecvCallback(MakeCallback(&SearchApplication::ReceiveMessage, this));
}

void SearchApplication::StopApplication() {
	if(socket != NULL) {
		socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
	}
}

void SearchApplication::CreateAndSendRequest() {
	SearchRequestHeader request = CreateRequest();
	seenRequests[GetRequestKey(request)] = request.GetCurrentHops();
	SendRequest(request);
	resultsManager->Activate();
	resultsManager->SetRequestTime(request.GetRequestTimestamp());
	resultsManager->SetRequestPosition(request.GetRequestPosition());
	resultsManager->SetRequestService(request.GetRequestedService());
	resultsManager->SetRequestDistance(request.GetMaxDistanceAllowed());
}

void SearchApplication::ReceiveMessage(Ptr<Socket> socket) {
	Address sourceAddress;
	Ptr<Packet> packet = socket->RecvFrom(sourceAddress);
	InetSocketAddress inetSourceAddress = InetSocketAddress::ConvertFrom(sourceAddress);
	Ipv4Address senderAddress = inetSourceAddress.GetIpv4();
	TypeHeader typeHeader;
	packet->RemoveHeader(typeHeader);
	if(!typeHeader.IsValid()) {
		return;
	}
	switch(typeHeader.GetType()) {
		case STRATOS_SEARCH_ERROR:
			ReceiveError(packet, senderAddress.Get());
			break;
		case STRATOS_SEARCH_REQUEST:
			ReceiveRequest(packet, senderAddress.Get());
			break;
		case STRATOS_SEARCH_RESPONSE:
			ReceiveResponse(packet, senderAddress.Get());
			break;
		default:
			break;
	}
}

void SearchApplication::SendBroadcastMessage(Ptr<Packet> packet) {
	InetSocketAddress remote = InetSocketAddress(Ipv4Address::GetBroadcast(), SEARCH_PORT);
	socket->SetAllowBroadcast(true);
	socket->Connect(remote);
	socket->Send(packet);
}

bool SearchApplication::IsValidRequest(SearchRequestHeader request) {
	POSITION requesterPosition = request.GetRequestPosition();
	POSITION myPosition = positionManager->GetCurrentPosition();
	double distance = positionManager->CalculateDistanceFromTo(requesterPosition, myPosition);
	if(distance > request.GetMaxDistanceAllowed()) {
		return false;
	}
	if(request.GetCurrentHops() > request.GetMaxHopsAllowed()) {
		return false;
	}
	if(seenRequests.find(GetRequestKey(request)) != seenRequests.end()) {
		return false;
	}
	return true;
}

void SearchApplication::SendUnicastMessage(Ptr<Packet> packet, uint destinationAddress) {
	InetSocketAddress remote = InetSocketAddress(Ipv4Address(destinationAddress), SEARCH_PORT);
	socket->SetAllowBroadcast(false);
	socket->Connect(remote);
	socket->Send(packet);
}

SearchRequestHeader SearchApplication::CreateRequest() {
	SearchRequestHeader request;
	request.SetCurrentHops(1);
	request.SetMaxHopsAllowed(MAX_HOPS);
	request.SetRequestAddress(localAddress);
	request.SetRequestTimestamp(Utilities::GetCurrentRawDateTime());
	//request.SetMaxHopsAllowed(Utilities::Random(MIN_HOPS, MAX_HOPS));
	request.SetRequestPosition(positionManager->GetCurrentPosition());
	request.SetRequestedService(OntologyApplication::GetRandomService());
	request.SetMaxDistanceAllowed(Utilities::Random(MIN_REQUEST_DISTANCE, MAX_REQUEST_DISTANCE));
	return request;
}

void SearchApplication::SendRequest(SearchRequestHeader requestHeader) {
	Ptr<Packet> packet = Create<Packet>();
	packet->AddHeader(requestHeader);
	TypeHeader typeHeader(STRATOS_SEARCH_REQUEST);
	packet->AddHeader(typeHeader);
	pendings[GetRequestKey(requestHeader)] = neighborhoodManager->GetNeighborhood();
	Simulator::Schedule(Seconds(Utilities::GetJitter()), &SearchApplication::SendBroadcastMessage, this, packet);
	Simulator::Schedule(Seconds(VERIFY_TIME), &SearchApplication::VerifyResponses, this, GetRequestKey(requestHeader));
	//std::cout << requestHeader << std::endl;
}

void SearchApplication::ForwardRequest(SearchRequestHeader requestHeader) {
	Ptr<Packet> packet = Create<Packet>();
	requestHeader.SetCurrentHops(requestHeader.GetCurrentHops() + 1);
	packet->AddHeader(requestHeader);
	TypeHeader typeHeader(STRATOS_SEARCH_REQUEST);
	packet->AddHeader(typeHeader);
	Simulator::Schedule(Seconds(Utilities::GetJitter()), &SearchApplication::SendBroadcastMessage, this, packet);
	Simulator::Schedule(Seconds(VERIFY_TIME), &SearchApplication::VerifyResponses, this, GetRequestKey(requestHeader));
}

void SearchApplication::ReceiveRequest(Ptr<Packet> packet, uint senderAddress) {
	SearchRequestHeader requestHeader;
	packet->RemoveHeader(requestHeader);
	pthread_mutex_lock(&mutex);
	if(!IsValidRequest(requestHeader)) {
		if(requestHeader.GetCurrentHops() < seenRequests[GetRequestKey(requestHeader)]) {
			CreateAndSendError(requestHeader, senderAddress);
		} else if(requestHeader.GetCurrentHops() == (seenRequests[GetRequestKey(requestHeader)] + 1)) {
			//std::cout << localAddress << " <-" << Ipv4Address(senderAddress) << std::endl;
			std::list<uint> pending = pendings[GetRequestKey(requestHeader)];
			pending.push_back(senderAddress);
			pendings[GetRequestKey(requestHeader)] = pending;
		}
		pthread_mutex_unlock(&mutex);
		return;
	}
	parents[GetRequestKey(requestHeader)] = senderAddress;
	seenRequests[GetRequestKey(requestHeader)] = requestHeader.GetCurrentHops() + 1;
	routeManager->SetAsRouteTo(senderAddress, requestHeader.GetRequestAddress().Get());
	//pendings[GetRequestKey(requestHeader)] = neighborhoodManager->GetNeighborhood();
	//std::cout << Ipv4Address(senderAddress) << " <- " << localAddress << std::endl;
	pthread_mutex_unlock(&mutex);
	CreateAndSaveResponse(requestHeader);
	ForwardRequest(requestHeader);
}

std::pair<uint, double> SearchApplication::GetRequestKey(SearchRequestHeader request) {
	uint address = request.GetRequestAddress().Get();
	double timestamp = request.GetRequestTimestamp();
	return std::make_pair(address, timestamp);
}

void SearchApplication::ReceiveError(Ptr<Packet> packet, uint senderAddress) {
	SearchErrorHeader errorHeader;
	packet->RemoveHeader(errorHeader);
	pthread_mutex_lock(&mutex);
	std::list<uint> pending = pendings[GetRequestKey(errorHeader)];
	pending.remove(senderAddress);
	pendings[GetRequestKey(errorHeader)] = pending;
	pthread_mutex_unlock(&mutex);
}

SearchErrorHeader SearchApplication::CreateError(SearchRequestHeader request) {
	SearchErrorHeader error;
	error.SetRequestAddress(request.GetRequestAddress());
	error.SetRequestTimestamp(request.GetRequestTimestamp());
	return error;
}

std::pair<uint, double> SearchApplication::GetRequestKey(SearchErrorHeader error) {
	uint address = error.GetRequestAddress().Get();
	double timestamp = error.GetRequestTimestamp();
	return std::make_pair(address, timestamp);
}

void SearchApplication::SendError(SearchErrorHeader errorHeader, uint receiverAddress) {
	Ptr<Packet> packet = Create<Packet>();
	packet->AddHeader(errorHeader);
	TypeHeader typeHeader(STRATOS_SEARCH_ERROR);
	packet->AddHeader(typeHeader);
	Simulator::Schedule(Seconds(Utilities::GetJitter()), &SearchApplication::SendUnicastMessage, this, packet, receiverAddress);
}

void SearchApplication::CreateAndSendError(SearchRequestHeader request, uint senderAddress) {
	SendError(CreateError(request), senderAddress);
}

void SearchApplication::SaveResponse(SearchResponseHeader response) {
	pthread_mutex_lock(&mutex);
	std::list<SearchResponseHeader> responses = this->responses[GetRequestKey(response)];
	responses.push_back(response);
	this->responses[GetRequestKey(response)] = responses;
	pthread_mutex_unlock(&mutex);
}

void SearchApplication::VerifyResponses(std::pair<uint, double> request) {
	bool found;
	std::list<uint>::iterator i;
	std::list<uint>::iterator j;
	pthread_mutex_lock(&mutex);
	std::list<uint> pending = pendings[request];
	std::list<uint> neighborhood = neighborhoodManager->GetNeighborhood();
	for(i = pending.begin(); i != pending.end(); i++) {
		found = false;
		for(j = neighborhood.begin(); j != neighborhood.end(); j++) {
			if((*i) == (*j)) {
				found = true;
				break;
			}
		}
		if(!found) {
			pending.erase(i--);
		}
	}
	pendings[request] = pending;
	pthread_mutex_unlock(&mutex);
	int hops = seenRequests[request];
	double timeElapsed = (Now().GetMilliSeconds() - request.second) / 1000;
	if(pending.empty() || timeElapsed >= ((MAX_HOPS - hops + 1) * VERIFY_TIME)) {
		SelectAndSendBestResponse(request);
		return;
	}
	Simulator::Schedule(Seconds(VERIFY_TIME), &SearchApplication::VerifyResponses, this, request);
}

void SearchApplication::ReceiveResponse(Ptr<Packet> packet, uint senderAddress) {
	SearchResponseHeader responseHeader;
	packet->RemoveHeader(responseHeader);
	pthread_mutex_lock(&mutex);
	std::list<SearchResponseHeader> responses = this->responses[GetRequestKey(responseHeader)];
	responses.push_back(responseHeader);
	this->responses[GetRequestKey(responseHeader)] = responses;
	std::list<uint> pending = pendings[GetRequestKey(responseHeader)];
	pending.remove(senderAddress);
	pendings[GetRequestKey(responseHeader)] = pending;
	routeManager->SetAsRouteTo(senderAddress, responseHeader.GetResponseAddress().Get());
	//std::cout << localAddress << " -> " << Ipv4Address(senderAddress) << std::endl;
	pthread_mutex_unlock(&mutex);
}

void SearchApplication::CreateAndSaveResponse(SearchRequestHeader request) {
	SaveResponse(CreateResponse(request));
}

void SearchApplication::SelectAndSendBestResponse(std::pair<uint, double> request) {
	pthread_mutex_lock(&mutex);
	std::list<SearchResponseHeader> responses = this->responses[request];
	pthread_mutex_unlock(&mutex);
	if(responses.empty() && request.first == localAddress.Get()) {
		//std::cout << Now().GetMilliSeconds() << " -> Search response to " << localAddress << " no responses for request" << std::endl;
		return;
	}
	SearchResponseHeader response = responses.front();
	if(response.GetRequestAddress() == localAddress) {
		//std::cout << response << std::endl;
		serviceManager->CreateAndExecuteSensingSchedule(responses);
		//serviceManager->CreateAndSendRequest(response.GetResponseAddress(), response.GetOfferedService().service);
		//resultsManager->SetResponseSemanticDistance(response.GetOfferedService().semanticDistance);
	} else {
		response = SelectBestResponse(responses);
		SendResponse(response, parents[request]);
	}
}

SearchResponseHeader SearchApplication::CreateResponse(SearchRequestHeader request) {
	POSITION requester = request.GetRequestPosition();
	POSITION me = positionManager->GetCurrentPosition();
	double distance = positionManager->CalculateDistanceFromTo(requester, me);
	SearchResponseHeader response;
	response.SetDistance(distance);
	response.SetResponseAddress(localAddress);
	response.SetHopDistance(request.GetCurrentHops());
	response.SetRequestAddress(request.GetRequestAddress());
	response.SetRequestTimestamp(request.GetRequestTimestamp());
	response.SetOfferedService(ontologyManager->GetBestOfferedService(request.GetRequestedService()));
	return response;
}

std::pair<uint, double> SearchApplication::GetRequestKey(SearchResponseHeader response) {
	uint address = response.GetRequestAddress().Get();
	double timestamp = response.GetRequestTimestamp();
	return std::make_pair(address, timestamp);
}

void SearchApplication::SendResponse(SearchResponseHeader responseHeader, uint parent) {
	Ptr<Packet> packet = Create<Packet>();
	packet->AddHeader(responseHeader);
	TypeHeader typeHeader(STRATOS_SEARCH_RESPONSE);
	packet->AddHeader(typeHeader);
	Simulator::Schedule(Seconds(Utilities::GetJitter()), &SearchApplication::SendUnicastMessage, this, packet, parent);
}

SearchResponseHeader SearchApplication::SelectBestResponse(std::list<SearchResponseHeader> responses) {
	SearchResponseHeader response;
	SearchResponseHeader bestResponse = responses.front();
	responses.pop_front();
	std::list<SearchResponseHeader>::iterator i;
	for(i = responses.begin(); i != responses.end(); i++) {
		response = *i;
		if(response.GetOfferedService().semanticDistance < bestResponse.GetOfferedService().semanticDistance) {
			bestResponse = response;
		} else if(response.GetOfferedService().semanticDistance == bestResponse.GetOfferedService().semanticDistance) {
			if(response.GetHopDistance() < bestResponse.GetHopDistance()) {
				bestResponse = response;
			} else if(response.GetHopDistance() == bestResponse.GetHopDistance()) {
				if(response.GetResponseAddress() < bestResponse.GetResponseAddress()) {
					bestResponse = response;
				}
			}
		}
	}
	return bestResponse;
}

SearchHelper::SearchHelper() {
	objectFactory.SetTypeId("SearchApplication");
}