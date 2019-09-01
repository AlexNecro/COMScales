#include "stdafx.h"
#include "NScalesFT11.h"


NScalesFT11::NScalesFT11(NLog* log) : IScalesHW(log)
{
	messageSize = 18;
}


NScalesFT11::~NScalesFT11()
{
}

long NScalesFT11::Connect(CString PortName, LONG address, LONG baudRate) {
	long ret = IScalesHW::Connect(PortName, address, baudRate);	
	return ret;
}

double NScalesFT11::GetWeight()
{
	DWORD begin = GetTickCount();
	CString a;
	if (!port) {		
		return 0.0;
	};
	int bytesRead = port->ReadData(buffer, messageSize * 2);
	if (bytesRead == 0) {
		state = ScalesStates::NoSignal;		
		return WEIGHTERROR;
	}
	if (bytesRead < messageSize) {
		state = ScalesStates::Timeout;		
		return WEIGHTERROR;
	}		
	weight = 0.0;
	//double weight = 0.0;
	for (int i = 0; i <= (bytesRead - messageSize); i++) {
		a.Format(TEXT("[0]==%02X, [16]==%02X"), buffer[i], buffer[i + 16]);
		log->Log(a);
		if (buffer[i] == 0x02) {			
			if (buffer[i + 16] == 0x0D) {
				if (buffer[i + 4] > '0' && buffer[i + 4] <= '9') weight = weight + 100000.0*(buffer[i + 4] - '0');
				if (buffer[i + 5] > '0' && buffer[i + 5] <= '9') weight = weight + 10000.0*(buffer[i + 5] - '0');
				if (buffer[i + 6] > '0' && buffer[i + 6] <= '9') weight = weight + 1000.0*(buffer[i + 6] - '0');
				if (buffer[i + 7] > '0' && buffer[i + 7] <= '9') weight = weight + 100.0*(buffer[i + 7] - '0');
				if (buffer[i + 8] > '0' && buffer[i + 8] <= '9') weight = weight + 10.0*(buffer[i + 8] - '0');
				if (buffer[i + 9] > '0' && buffer[i + 9] <= '9') weight = weight + 1.0*(buffer[i + 9] - '0');
				break;
			}
		}
	}	
	a.Format(TEXT("Weight is parsed: %f; bytes=%d"), weight, bytesRead);
	log->Log(a);
	state = ScalesStates::OK;
	DWORD end = GetTickCount();		
	return weight;
}