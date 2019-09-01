#pragma once
#include "IScalesHW.h"
class NScalesTest :
	public IScalesHW
{
protected:
	LONG lastTick;
public:
	NScalesTest(NLog* log);
	virtual ~NScalesTest();
	virtual double GetWeight();
	virtual long Connect(CString PortName, LONG address, LONG baudRate);
	virtual long Disconnect();
};

