#pragma once
#include "base.h"
#include "COMPort.h"
#include "NLog.h"
class Indicator
{
public:
	Indicator(NLog* log);
	~Indicator();
	bool Connect(CString PortName, LONG baudRate = 9600);
	void Disconnect();
	void Print(CString text);
	void PrintWeight(double weight);
	void SetDisplayTime(int millis);
	int GetDigits();
	bool CanText();

protected:
	NLog* log;	
	COMPort *port;
	int digits;
	bool canText;
	char key[4];
	void Encode(char* buffer, int len);
	void Decode(char* buffer, int len);
	void SetSecret(DWORD secret);
};

