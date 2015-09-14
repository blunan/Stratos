#ifndef UTILITIES_H
#define UTILITIES_H

#include "ns3/core-module.h"

class Utilities {

	public:
		static double GetJitter();
		static double GetCurrentRawDateTime();
		static double Random(double min, double max);
		static double GetSecondsElapsedSinceUntil(double since, double until);
};

#endif