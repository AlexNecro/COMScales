#include "stdafx.h"
#include "NScalesHBT9.h"


NScalesHBT9::NScalesHBT9(NLog* log) : IScalesHW(log)
{
	messageSize = 14;
}


NScalesHBT9::~NScalesHBT9()
{
}

long NScalesHBT9::Connect(CString PortName, LONG address, LONG baudRate) {
	long ret = IScalesHW::Connect(PortName, address, baudRate);
	return ret;
}

double NScalesHBT9::GetWeight()
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
	//need to find such message: "wn0001.000kg" "wn000226.0kg" point can be фе different positions!
	double weight = 0.0;
	for (int i = 0; i <= (bytesRead - messageSize); i++) {
		if (buffer[i] == 'w'
			&& buffer[i + 10] == 'k' && buffer[i + 11] == 'g') {			

			double mult = 1000000.0;
			double div = 1.0;
			int pointPos = 0;
			for (int q = 2; q < 10; q++) {
				if (buffer[i + q] >= '0' && buffer[i + q] <= '9') {
					weight += mult*(buffer[i + q] - '0');
					mult /= 10.0;
				}
				else if (buffer[i + q] == '.') {
					pointPos = q;
					div = mult*10;
				}

			}
			log->LogF(TEXT("w:%f / %f = %f (%d)"), weight, div, weight/div, pointPos);
			weight /= div;
			state = ScalesStates::OK;
			return weight;
			break;
		}
	}
	log->LogF(TEXT("bytes read %d, no data"), bytesRead);
	state = ScalesStates::Timeout;
	DWORD end = GetTickCount();		
	return WEIGHTERROR;
}