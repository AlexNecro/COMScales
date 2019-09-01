#pragma once
#include "IScalesHW.h"
//this is the message (1000kg, netto), terminated with \r\n:
//"wn0001.000kg"
//this is 0:
//"wn0000.000kg"
//so, msg size is 12 (will get 15)
class NScalesHBT9 :
	public IScalesHW
{
public:
	NScalesHBT9(NLog* log);
	virtual ~NScalesHBT9();
	virtual long Connect(CString PortName, LONG address, LONG baudRate);
	virtual double GetWeight();
};

