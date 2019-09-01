#pragma once
#include "IScalesHW.h"
//this is 1360kg:
//0E 02 69 70 30 20 20 31 33 36 30 20 20 20 20 20 30 0D
//this is 0:
//48 02 69 70 30 20 20 20 20 20 30 20 20 20 20 20 30 0D
//so, weight is in bytes 6-11, as text
//byte 1 must be 02, byte 17 must be 0D
class NScalesFT11 :
	public IScalesHW
{
public:
	NScalesFT11(NLog* log);
	virtual ~NScalesFT11();
	virtual long Connect(CString PortName, LONG address, LONG baudRate);
	virtual double GetWeight();
};

