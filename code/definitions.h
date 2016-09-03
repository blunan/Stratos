#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <string>
#include <sys/types.h>

#define MAX_HOPS 4

#define HELLO_TIME 2 //seconds

#define MIN_JITTER 0.001 //1ms

#define MAX_JITTER 0.01 //10ms

#define HELLO_PORT 60000

#define VERIFY_TIME 1 //seconds

#define SEARCH_PORT 60001

#define SERVICE_PORT 60002

#define MAX_DISTANCE 1000 //meters

#define PACKET_LENGTH 256 //bytes

#define MAX_REQUEST_TIME 50 //seconds

#define MAX_TIMES_NOT_SEEN 3

#define MIN_REQUEST_DISTANCE 400

#define MAX_REQUEST_DISTANCE 600

#define TOTAL_SIMULATION_TIME 100 //seconds

#define TOTAL_NUMBER_OF_NODES 100

struct POSITION {
	double x;
	double y;
};

struct NEIGHBOR {
	uint address;
	double lastSeen;
};

struct OFFERED_SERVICE {
	std::string service;
	int semanticDistance;
};

enum MessageType {
	STRATOS = 0,
	STRATOS_HELLO = 1,
	STRATOS_SEARCH_REQUEST = 2,
	STRATOS_SEARCH_RESPONSE = 3,
	STRATOS_SEARCH_ERROR = 4,
	STRATOS_SERVICE_REQUEST = 5,
	STRATOS_SERVICE_RESPONSE = 6,
	STRATOS_SERVICE_ERROR = 7
};

enum Flag {
	STRATOS_NULL = 0,
	STRATOS_START_SERVICE = 1,
	STRATOS_SERVICE_STARTED = 2,
	STRATOS_DO_SERVICE = 3,
	STRATOS_STOP_SERVICE = 4,
	STRATOS_SERVICE_STOPPED = 5
};

#endif