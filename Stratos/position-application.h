#ifndef POSITION_APPLICATION_H
#define POSITION_APPLICATION_H

#include "ns3/mobility-module.h"

#include "definitions.h"
#include "application-helper.h"

using namespace ns3;

class PositionApplication : public Application {

	public:
		static TypeId GetTypeId();
		
		PositionApplication();
		~PositionApplication();

	protected:
		virtual void DoInitialize();
		virtual void DoDispose();

	private:
		virtual void StartApplication();
		virtual void StopApplication();

	private:
		Ptr<MobilityModel> mobility;

	public:
		POSITION GetCurrentPosition();
		double CalculateDistanceFromTo(POSITION from, POSITION to);
};

class PositionHelper : public ApplicationHelper {

	public:
		PositionHelper();
};

#endif