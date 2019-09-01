#include "stdafx.h"
#include "NScalesTest.h"


NScalesTest::NScalesTest(NLog* log) : IScalesHW(log)
{
	this->log = log;
	state = ScalesStates::NotConnected;
}


NScalesTest::~NScalesTest()
{
}

double NScalesTest::GetWeight()
{	
	if (GetTickCount() - lastTick > 2000) {
		lastTick = GetTickCount();
		weight = 1.0*rand() / 1000;
	}
	return weight;
}

long NScalesTest::Connect(CString PortName, LONG address, LONG baudRate)
{	
	weight = 1234.5678;
	state = ScalesStates::OK;
	lastTick = GetTickCount();
	return S_OK;
}

long NScalesTest::Disconnect()
{
	return 0;
}
