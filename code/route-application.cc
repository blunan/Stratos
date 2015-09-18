#include "route-application.h"

NS_OBJECT_ENSURE_REGISTERED(RouteApplication);

TypeId RouteApplication::GetTypeId() {
	static TypeId typeId = TypeId("RouteApplication")
		.SetParent<Application>()
		.AddConstructor<RouteApplication>();
	return typeId;
}

RouteApplication::RouteApplication() {
}

void RouteApplication::DoInitialize() {
	pthread_mutex_init(&mutex, NULL);
	Application::DoInitialize();
}

RouteApplication::~RouteApplication() {
}

void RouteApplication::DoDispose() {
	pthread_mutex_destroy(&mutex);
	Application::DoDispose();
	
}

void RouteApplication::StartApplication() {
	routes.clear();
}

void RouteApplication::StopApplication() {
	routes.clear();
}

uint RouteApplication::GetRouteTo(uint destination) {
	pthread_mutex_lock(&mutex);
	uint nextStep = routes[destination];
	pthread_mutex_unlock(&mutex);
	return nextStep;
}

void RouteApplication::SetAsRouteTo(uint nextStep, uint destination) {
	pthread_mutex_lock(&mutex);
	routes[destination] = nextStep;
	pthread_mutex_unlock(&mutex);	
}

RouteHelper::RouteHelper() {
	objectFactory.SetTypeId("RouteApplication");
}