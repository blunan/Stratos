#include "utilities.h"

#include "definitions.h"

double Utilities::GetJitter() {
	return Random(MIN_JITTER, MAX_JITTER);
}

double Utilities::GetCurrentRawDateTime() {
	return ns3::Now().GetMilliSeconds();
}

double Utilities::Random(double min, double max) {
	ns3::Ptr<ns3::UniformRandomVariable> random = ns3::CreateObject<ns3::UniformRandomVariable>();
	return random->GetValue(min, max);
}

double Utilities::GetSecondsElapsedSinceUntil(double since, double until) {
	return (until - since) / 1000;
}