#include "route-application.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

NS_LOG_COMPONENT_DEFINE("RouteApplication");

NS_OBJECT_ENSURE_REGISTERED(RouteApplication);

TypeId RouteApplication::GetTypeId() {
	NS_LOG_FUNCTION_NOARGS();
	static TypeId typeId = TypeId("RouteApplication")
		.SetParent<Application>()
		.AddConstructor<RouteApplication>();
	return typeId;
}

RouteApplication::RouteApplication() {
	NS_LOG_FUNCTION(this);
}

void RouteApplication::DoInitialize() {
	NS_LOG_FUNCTION(this);
	pthread_mutex_init(&mutex, NULL);
	Application::DoInitialize();
}

RouteApplication::~RouteApplication() {
	NS_LOG_FUNCTION(this);
}

void RouteApplication::DoDispose() {
	NS_LOG_FUNCTION(this);
	pthread_mutex_destroy(&mutex);
	Application::DoDispose();
}

void RouteApplication::StartApplication() {
	NS_LOG_FUNCTION(this);
	routes.clear();
}

void RouteApplication::StopApplication() {
	NS_LOG_FUNCTION(this);
	routes.clear();
}

uint RouteApplication::GetRouteTo(uint destination) {
	NS_LOG_FUNCTION(this << destination);
	pthread_mutex_lock(&mutex);
	uint nextHop = routes[destination];
	pthread_mutex_unlock(&mutex);
	NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> next hop to reach " << Ipv4Address(destination) << " is " << Ipv4Address(nextHop));
	return nextHop;
}

void RouteApplication::SetAsRouteTo(uint nextHop, uint destination) {
	NS_LOG_FUNCTION(this << nextHop << destination);
	pthread_mutex_lock(&mutex);
	routes[destination] = nextHop;
	pthread_mutex_unlock(&mutex);
	NS_LOG_DEBUG(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> setting " << Ipv4Address(nextHop) << " as next hop to" << Ipv4Address(destination));
}

RouteHelper::RouteHelper() {
	NS_LOG_FUNCTION(this);
	objectFactory.SetTypeId("RouteApplication");
}