#include "search-application.h"

#include "utilities.h"
#include "definitions.h"
#include "type-header.h"

NS_LOG_COMPONENT_DEFINE("SearchApplication");

NS_OBJECT_ENSURE_REGISTERED(SearchApplication);

TypeId SearchApplication::GetTypeId() {
	NS_LOG_FUNCTION_NOARGS();
	static TypeId typeId = TypeId("SearchApplication")
		.SetParent<Application>()
		.AddConstructor<SearchApplication>();
	return typeId;
}

SearchApplication::SearchApplication() {
	NS_LOG_FUNCTION(this);
}

SearchApplication::~SearchApplication() {
	NS_LOG_FUNCTION(this);
}

void SearchApplication::DoInitialize() {
	NS_LOG_FUNCTION(this);
	pthread_mutex_init(&mutex, NULL);
	routeManager = DynamicCast<RouteApplication>(GetNode()->GetApplication(4));
	serviceManager = DynamicCast<ServiceApplication>(GetNode()->GetApplication(5));
	resultsManager = DynamicCast<ResultsApplication>(GetNode()->GetApplication(7));
	scheduleManager = DynamicCast<ScheduleApplication>(GetNode()->GetApplication(6));
	ontologyManager = DynamicCast<OntologyApplication>(GetNode()->GetApplication(1));
	positionManager = DynamicCast<PositionApplication>(GetNode()->GetApplication(2));
	neighborhoodManager = DynamicCast<NeighborhoodApplication>(GetNode()->GetApplication(0));
	socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
	localAddress = GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
	InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), SEARCH_PORT);
	socket->Bind(local);
	Application::DoInitialize();
}

void SearchApplication::DoDispose() {
	NS_LOG_FUNCTION(this);
	if(socket != NULL) {
		socket->Close();
	}
	pthread_mutex_destroy(&mutex);
	Application::DoDispose();
}

void SearchApplication::StartApplication() {
	NS_LOG_FUNCTION(this);
	socket->SetRecvCallback(MakeCallback(&SearchApplication::ReceiveMessage, this));
}

void SearchApplication::StopApplication() {
	NS_LOG_FUNCTION(this);
	if(socket != NULL) {
		socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
	}
}

SearchResponseHeader SearchApplication::SelectBestResponse(std::list<SearchResponseHeader> responses) {
	NS_LOG_FUNCTION(&responses);
	SearchResponseHeader response;
	SearchResponseHeader bestResponse = responses.front();
	responses.pop_front();
	std::list<SearchResponseHeader>::iterator i;
	NS_LOG_DEBUG("Best response is: " << bestResponse);
	for(i = responses.begin(); i != responses.end(); i++) {
		response = *i;
		if(response.GetOfferedService().semanticDistance < bestResponse.GetOfferedService().semanticDistance) {
			bestResponse = response;
			NS_LOG_DEBUG("New best response selected by semantic distance: " << bestResponse);
		} else if(response.GetOfferedService().semanticDistance == bestResponse.GetOfferedService().semanticDistance) {
			if(response.GetHopDistance() < bestResponse.GetHopDistance()) {
				bestResponse = response;
				NS_LOG_DEBUG("New best response selected by hop distance: " << bestResponse);
			} else if(response.GetHopDistance() == bestResponse.GetHopDistance()) {
				if(response.GetResponseAddress() < bestResponse.GetResponseAddress()) {
					bestResponse = response;
					NS_LOG_DEBUG("New best response selected by address: " << bestResponse);
				}
			}
		}
	}
	return bestResponse;
}

void SearchApplication::CreateAndSendRequest() {
	NS_LOG_FUNCTION(this);
	SearchRequestHeader request = CreateRequest();
	seenRequests[GetRequestKey(request)] = request.GetCurrentHops();
	SendRequest(request);
	resultsManager->Activate();
	resultsManager->SetRequestTime(request.GetRequestTimestamp());
	resultsManager->SetRequestService(request.GetRequestedService());
	resultsManager->SetRequestPosition(request.GetRequestPosition());
	resultsManager->SetRequestDistance(request.GetMaxDistanceAllowed());
}

void SearchApplication::ReceiveMessage(Ptr<Socket> socket) {
	NS_LOG_FUNCTION(this << socket);
	Address sourceAddress;
	Ptr<Packet> packet = socket->RecvFrom(sourceAddress);
	InetSocketAddress inetSourceAddress = InetSocketAddress::ConvertFrom(sourceAddress);
	Ipv4Address senderAddress = inetSourceAddress.GetIpv4();
	TypeHeader typeHeader;
	packet->RemoveHeader(typeHeader);
	if(!typeHeader.IsValid()) {
		NS_LOG_DEBUG(localAddress << " -> Received search message from " << senderAddress << " is invalid");
		return;
	}
	NS_LOG_DEBUG(localAddress << " -> Processing search message from " << senderAddress);
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
			NS_LOG_WARN(localAddress << " -> Serach message is unknown!");
			break;
	}
}

void SearchApplication::SendBroadcastMessage(Ptr<Packet> packet) {
	NS_LOG_FUNCTION(this << packet);
	InetSocketAddress remote = InetSocketAddress(Ipv4Address::GetBroadcast(), SEARCH_PORT);
	socket->SetAllowBroadcast(true);
	socket->Connect(remote);
	socket->Send(packet);
}

bool SearchApplication::IsValidRequest(SearchRequestHeader request) {
	NS_LOG_FUNCTION(this << request);
	POSITION requesterPosition = request.GetRequestPosition();
	POSITION myPosition = positionManager->GetCurrentPosition();
	double distance = PositionApplication::CalculateDistanceFromTo(requesterPosition, myPosition);
	if(seenRequests.find(GetRequestKey(request)) != seenRequests.end()) {
		NS_LOG_DEBUG(localAddress << " -> Request has been seen before");
		return false;
	}
	if(request.GetCurrentHops() > request.GetMaxHopsAllowed()) {
		NS_LOG_DEBUG(localAddress << " -> Request exceeds max hops allowed");
		return false;
	}
	if(distance > request.GetMaxDistanceAllowed()) {
		NS_LOG_DEBUG(localAddress << " -> Request exceeds max distance allowed");
		return false;
	}
	return true;
}

void SearchApplication::SendUnicastMessage(Ptr<Packet> packet, uint destinationAddress) {
	NS_LOG_FUNCTION(this << packet << destinationAddress);
	InetSocketAddress remote = InetSocketAddress(Ipv4Address(destinationAddress), SEARCH_PORT);
	socket->SetAllowBroadcast(false);
	socket->Connect(remote);
	socket->Send(packet);
}

SearchRequestHeader SearchApplication::CreateRequest() {
	NS_LOG_FUNCTION(this);
	SearchRequestHeader request;
	request.SetCurrentHops(0);
	request.SetMaxHopsAllowed(MAX_HOPS);
	request.SetRequestAddress(localAddress);
	request.SetRequestTimestamp(Utilities::GetCurrentRawDateTime());
	//request.SetMaxHopsAllowed(Utilities::Random(MIN_HOPS, MAX_HOPS));
	request.SetRequestPosition(positionManager->GetCurrentPosition());
	request.SetRequestedService(OntologyApplication::GetRandomService());
	request.SetMaxDistanceAllowed(Utilities::Random(MIN_REQUEST_DISTANCE, MAX_REQUEST_DISTANCE));
	NS_LOG_DEBUG(localAddress << " -> Request created: " << request);
	return request;
}

void SearchApplication::SendRequest(SearchRequestHeader requestHeader) {
	NS_LOG_FUNCTION(this << requestHeader);
	Ptr<Packet> packet = Create<Packet>();
	packet->AddHeader(requestHeader);
	TypeHeader typeHeader(STRATOS_SEARCH_REQUEST);
	packet->AddHeader(typeHeader);
	NS_LOG_DEBUG(localAddress << " -> Schedule request to send");
	Simulator::Schedule(Seconds(Utilities::GetJitter()), &SearchApplication::SendBroadcastMessage, this, packet);
	NS_LOG_DEBUG(localAddress << " -> Schedule request to verify");
	Simulator::Schedule(Seconds(VERIFY_TIME), &SearchApplication::VerifyResponses, this, GetRequestKey(requestHeader));
}

void SearchApplication::ForwardRequest(SearchRequestHeader requestHeader) {
	NS_LOG_FUNCTION(this << requestHeader);
	Ptr<Packet> packet = Create<Packet>();
	packet->AddHeader(requestHeader);
	TypeHeader typeHeader(STRATOS_SEARCH_REQUEST);
	packet->AddHeader(typeHeader);
	NS_LOG_DEBUG(localAddress << " -> Schedule request to forward");
	Simulator::Schedule(Seconds(Utilities::GetJitter()), &SearchApplication::SendBroadcastMessage, this, packet);
	if(requestHeader.GetCurrentHops() == requestHeader.GetMaxHopsAllowed()) {
		NS_LOG_DEBUG(localAddress << " -> I'm leaf for this request, verify responses (only mine) now");
		VerifyResponses(GetRequestKey(requestHeader));
	} else {
		NS_LOG_DEBUG(localAddress << " -> Schedule request to verify");
		Simulator::Schedule(Seconds(VERIFY_TIME), &SearchApplication::VerifyResponses, this, GetRequestKey(requestHeader));
	}
}

void SearchApplication::ReceiveRequest(Ptr<Packet> packet, uint senderAddress) {
	NS_LOG_FUNCTION(this << packet << senderAddress);
	SearchRequestHeader requestHeader;
	packet->RemoveHeader(requestHeader);
	requestHeader.SetCurrentHops(requestHeader.GetCurrentHops() + 1);
	NS_LOG_DEBUG(localAddress << " -> Received: " << requestHeader);
	pthread_mutex_lock(&mutex);
	std::pair<uint, double> requestKey = GetRequestKey(requestHeader);
	if(!IsValidRequest(requestHeader)) {
		NS_LOG_DEBUG(localAddress << " -> Request from " << Ipv4Address(senderAddress) << " is invalid");
		if(requestHeader.GetCurrentHops() < seenRequests[requestKey]) {
			NS_LOG_DEBUG(localAddress << " -> " << Ipv4Address(senderAddress) << " is possibly my ancestor, send error");
			CreateAndSendError(requestHeader, senderAddress);
		} else if(requestHeader.GetCurrentHops() == (seenRequests[requestKey] + 2)) {
			NS_LOG_DEBUG(localAddress << " -> " << Ipv4Address(senderAddress) << " is possibly my son, wai for his response");
			std::list<uint> pending = pendings[requestKey];
			pending.push_back(senderAddress);
			pendings[requestKey] = pending;
		}
		pthread_mutex_unlock(&mutex);
		return;
	}
	parents[requestKey] = senderAddress;
	NS_LOG_DEBUG(localAddress << " -> " << Ipv4Address(senderAddress) << " is my parent for this request");
	seenRequests[requestKey] = requestHeader.GetCurrentHops();
	routeManager->SetAsRouteTo(senderAddress, requestHeader.GetRequestAddress().Get());
	pthread_mutex_unlock(&mutex);
	CreateAndSaveResponse(requestHeader);
	ForwardRequest(requestHeader);
}

std::pair<uint, double> SearchApplication::GetRequestKey(SearchRequestHeader request) {
	NS_LOG_FUNCTION(this << request);
	uint address = request.GetRequestAddress().Get();
	double timestamp = request.GetRequestTimestamp();
	NS_LOG_DEBUG(localAddress << " -> Key from request is [" << address << ", " << timestamp << "] " << request);
	return std::make_pair(address, timestamp);
}

void SearchApplication::ReceiveError(Ptr<Packet> packet, uint senderAddress) {
	NS_LOG_FUNCTION(this << packet << senderAddress);
	SearchErrorHeader errorHeader;
	packet->RemoveHeader(errorHeader);
	NS_LOG_DEBUG(localAddress << " -> Received: " << errorHeader);
	std::pair<uint, double> requestKey = GetRequestKey(errorHeader);
	pthread_mutex_lock(&mutex);
	std::list<uint> pending = pendings[requestKey];
	pending.remove(senderAddress);
	pendings[requestKey] = pending;
	NS_LOG_DEBUG(localAddress << " -> " << Ipv4Address(senderAddress) << " has been remoed from possible sons");
	pthread_mutex_unlock(&mutex);
}

SearchErrorHeader SearchApplication::CreateError(SearchRequestHeader request) {
	NS_LOG_FUNCTION(this << request);
	SearchErrorHeader error;
	error.SetRequestAddress(request.GetRequestAddress());
	error.SetRequestTimestamp(request.GetRequestTimestamp());
	NS_LOG_DEBUG(localAddress << " -> Error created: " << error);
	return error;
}

std::pair<uint, double> SearchApplication::GetRequestKey(SearchErrorHeader error) {
	NS_LOG_FUNCTION(this << error);
	uint address = error.GetRequestAddress().Get();
	double timestamp = error.GetRequestTimestamp();
	NS_LOG_DEBUG(localAddress << " -> Key from error is [" << address << ", " << timestamp << "] " << error);
	return std::make_pair(address, timestamp);
}

void SearchApplication::SendError(SearchErrorHeader errorHeader, uint receiverAddress) {
	NS_LOG_FUNCTION(this << errorHeader << receiverAddress);
	Ptr<Packet> packet = Create<Packet>();
	packet->AddHeader(errorHeader);
	TypeHeader typeHeader(STRATOS_SEARCH_ERROR);
	packet->AddHeader(typeHeader);
	NS_LOG_DEBUG(localAddress << " -> Schedule error to send");
	Simulator::Schedule(Seconds(Utilities::GetJitter()), &SearchApplication::SendUnicastMessage, this, packet, receiverAddress);
}

void SearchApplication::CreateAndSendError(SearchRequestHeader request, uint senderAddress) {
	NS_LOG_FUNCTION(this << request << senderAddress);
	SendError(CreateError(request), senderAddress);
}

void SearchApplication::SaveResponse(SearchResponseHeader response) {
	NS_LOG_FUNCTION(this << response);
	std::pair<uint, double> requestKey = GetRequestKey(response);
	pthread_mutex_lock(&mutex);
	std::list<SearchResponseHeader> responses = this->responses[requestKey];
	responses.push_back(response);
	this->responses[requestKey] = responses;
	pthread_mutex_unlock(&mutex);
}

void SearchApplication::VerifyResponses(std::pair<uint, double> request) {
	NS_LOG_FUNCTION(this << &request);
	bool found;
	std::list<uint>::iterator i;
	std::list<uint>::iterator j;
	pthread_mutex_lock(&mutex);
	std::list<uint> pending = pendings[request];
	std::list<uint> neighborhood = neighborhoodManager->GetNeighborhood();
	for(i = pending.begin(); i != pending.end(); i++) {
		found = false;
		NS_LOG_DEBUG(localAddress << " -> Searching for " << Ipv4Address(*i) << " in neighborhood for [" << request.first << ", " << request.second << "]");
		for(j = neighborhood.begin(); j != neighborhood.end(); j++) {
			if((*i) == (*j)) {
				found = true;
				NS_LOG_DEBUG(localAddress << " -> " << Ipv4Address(*i) << " is in neighborhood for [" << request.first << ", " << request.second << "] wait for it's response");
				break;
			}
		}
		if(!found) {
			pending.erase(i--);
			NS_LOG_DEBUG(localAddress << " -> " << Ipv4Address(*i) << " is not in neighborhood for [" << request.first << ", " << request.second << "] and has been deleated from pending responses");
		}
	}
	pendings[request] = pending;
	pthread_mutex_unlock(&mutex);
	int hops = seenRequests[request];
	double maxSecondsWait = (double) ((MAX_HOPS - hops) * VERIFY_TIME);
	double secondsElapsed = (Now().GetMilliSeconds() - request.second) / 1000;
	NS_LOG_DEBUG(localAddress << " -> [" << request.first << ", " << request.second << "] hops = " << hops << ", maxWaitSeconds = " << maxSecondsWait << ", secondsElapsed = " << secondsElapsed << " [" << request.first << ", " << request.second << "]");
	if(pending.empty() || secondsElapsed >= maxSecondsWait) {
		NS_LOG_DEBUG(localAddress << " -> Either I'm not waiting for a response from one of my sons or max expantion time has been reached for [" << request.first << ", " << request.second << "]");
		SelectAndSendBestResponse(request);
	} else {
		NS_LOG_DEBUG(localAddress << " -> Schedule request [" << request.first << ", " << request.second << "] to verify");
		Simulator::Schedule(Seconds(VERIFY_TIME), &SearchApplication::VerifyResponses, this, request);
	}
}

void SearchApplication::CreateAndSaveResponse(SearchRequestHeader request) {
	NS_LOG_FUNCTION(this << request);
	SaveResponse(CreateResponse(request));
}

void SearchApplication::ReceiveResponse(Ptr<Packet> packet, uint senderAddress) {
	NS_LOG_FUNCTION(this << packet << senderAddress);
	SearchResponseHeader responseHeader;
	packet->RemoveHeader(responseHeader);
	NS_LOG_DEBUG(localAddress << " -> Received response: " << responseHeader);
	std::pair<uint, double> requestKey = GetRequestKey(responseHeader);
	pthread_mutex_lock(&mutex);
	std::list<SearchResponseHeader> responses = this->responses[requestKey];
	responses.push_back(responseHeader);
	this->responses[requestKey] = responses;
	std::list<uint> pending = pendings[requestKey];
	pending.remove(senderAddress);
	NS_LOG_DEBUG(localAddress << " -> Remove " << Ipv4Address(senderAddress) << " from pendings");
	pendings[requestKey] = pending;
	pthread_mutex_unlock(&mutex);
	routeManager->SetAsRouteTo(senderAddress, responseHeader.GetResponseAddress().Get());
}

void SearchApplication::SelectAndSendBestResponse(std::pair<uint, double> request) {
	NS_LOG_FUNCTION(this << &request);
	pthread_mutex_lock(&mutex);
	std::list<SearchResponseHeader> responses = this->responses[request];
	pthread_mutex_unlock(&mutex);
	if(responses.empty() && request.first == localAddress.Get()) {
		NS_LOG_DEBUG(localAddress << " -> There are no responses for request [" << request.first << ", " << request.second << "] and I'm the initiator");
		return;
	}
	SearchResponseHeader response = SelectBestResponse(responses);
	NS_LOG_DEBUG(localAddress << " -> Best reponse is: " << response);
	if(response.GetRequestAddress() == localAddress) {
		NS_LOG_DEBUG(localAddress << " -> Start schedule for response");
		scheduleManager->CreateAndExecuteSchedule(responses);
	} else {
		NS_LOG_DEBUG(localAddress << " -> Send response to parent");
		SendResponse(response, parents[request]);
	}
}

SearchResponseHeader SearchApplication::CreateResponse(SearchRequestHeader request) {
	NS_LOG_FUNCTION(this << request);
	POSITION requester = request.GetRequestPosition();
	POSITION me = positionManager->GetCurrentPosition();
	double distance = PositionApplication::CalculateDistanceFromTo(requester, me);
	SearchResponseHeader response;
	response.SetDistance(distance);
	response.SetResponseAddress(localAddress);
	response.SetHopDistance(request.GetCurrentHops());
	response.SetRequestAddress(request.GetRequestAddress());
	response.SetRequestTimestamp(request.GetRequestTimestamp());
	response.SetOfferedService(ontologyManager->GetBestOfferedService(request.GetRequestedService()));
	NS_LOG_DEBUG(localAddress << " -> Response created: " << response);
	return response;
}

std::pair<uint, double> SearchApplication::GetRequestKey(SearchResponseHeader response) {
	NS_LOG_FUNCTION(this << response);
	uint address = response.GetRequestAddress().Get();
	double timestamp = response.GetRequestTimestamp();
	NS_LOG_DEBUG(localAddress << " -> Key from response is [" << address << ", " << timestamp << "] " << response);
	return std::make_pair(address, timestamp);
}

void SearchApplication::SendResponse(SearchResponseHeader responseHeader, uint parent) {
	NS_LOG_FUNCTION(this << responseHeader << parent);
	Ptr<Packet> packet = Create<Packet>();
	packet->AddHeader(responseHeader);
	TypeHeader typeHeader(STRATOS_SEARCH_RESPONSE);
	packet->AddHeader(typeHeader);
	NS_LOG_DEBUG(localAddress << " -> Schedule response to send");
	Simulator::Schedule(Seconds(Utilities::GetJitter()), &SearchApplication::SendUnicastMessage, this, packet, parent);
}

SearchHelper::SearchHelper() {
	NS_LOG_FUNCTION(this);
	objectFactory.SetTypeId("SearchApplication");
}