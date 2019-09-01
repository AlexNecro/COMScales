#pragma once
#include "IScalesHW.h"
//CAS-6000
//must be set continuous mode, of course
//PC must send device ID to acquire data (is it true?), device ID must be set on scales
//by option F22
//port parameters must be set by options F31-F33(COM1) or F34-F36(COM2)
//message size: 22 bytes
//--------------------------------------------------
//|SB|','|GS|','|\0DB|','|data(8bytes)|\0|KG|CR|LF|
//|2 |1  |2 |1  |2   |1  |    8       | 1|2 |1 |1 |
//--------------------------------------------------
//SB: 'US'==unstable, 'ST'=='stable', 'OL'==overload
//GS: 'GS'==gross weight, 'NT'==net weight
//DB: bits, left to right: 7:1 6:Stable 5:High 4:Low 3:HOLD 2:GROSS 1:TARE 0:ZERO
//data: text string with digits, ',' & '-': "-   13.5" (spaces are spaces or nulls?) 10000000
//KG: text, units 'KG'==kilogramms

//this is zero:
//53 54 2C 47 53 2C 44 C5 2C 20 20 20 20 30 2E 30 30 00 20 74 0D 0A
//53 54 2c 47 53 2c 44 c5 2c 20 20 20 20 30 2e 30 30 00 20 74 0d 0a
//this is 1.33:
//53 54 2C 47 53 2C 44 C4 2C 20 20 20 20 31 2E 33 33 00 20 74 0D 0A

class NScalesCAS6000 :
	public IScalesHW
{
protected:
	byte scaleAddress;
public:
	NScalesCAS6000(NLog* log);
	virtual ~NScalesCAS6000();
	virtual long Connect(CString PortName, LONG address, LONG baudRate);
	virtual double GetWeight();
};

