#include "stdafx.h"
#include "NScalesVT220.h"


NScalesVT220::NScalesVT220(NLog* log) : IScalesHW(log)
{
	messageSize = 10;
}


NScalesVT220::~NScalesVT220()
{
}

long NScalesVT220::Connect(CString PortName, LONG address, LONG baudRate) {
	long ret = IScalesHW::Connect(PortName, address, baudRate);	
	return ret;
}

double NScalesVT220::GetWeight()
{
	DWORD begin = GetTickCount();
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
	double weight = 0.0;
	for (int i = 0; i <= (bytesRead - messageSize); i++) {
		if (buffer[i] == 'T' && buffer[i+5]=='.') {			
			weight
				= 100.0*(buffer[i + 2] - '0')
				+ 10.0*(buffer[i + 3] - '0')
				+ 1.0*(buffer[i + 4] - '0') //tonnes!
				+ 0.1*(buffer[i + 6] - '0')
				+ 0.01*(buffer[i + 7] - '0');
			weight = weight * 1000;//kgs
			break;
		}
	}	
	state = ScalesStates::OK;
	DWORD end = GetTickCount();		
	return weight;
}