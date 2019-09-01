#pragma once
#include "IScalesHW.h"
//"T+000.00"
class NScalesVT220 :
	public IScalesHW
{
public:
	NScalesVT220(NLog* log);
	virtual ~NScalesVT220();
	virtual long Connect(CString PortName, LONG address, LONG baudRate);
	virtual double GetWeight();
};

