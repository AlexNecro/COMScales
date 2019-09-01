#include "stdafx.h"
#include "NScalesBU4263M1.h"

NScalesBU4263M1::NScalesBU4263M1(NLog* log) : IScalesHW(log)
{
	messageSize = 6;
}


NScalesBU4263M1::~NScalesBU4263M1()
{
	Disconnect();
}

double NScalesBU4263M1::GetWeight()
{
	if (!port) return 0.0;	
	int bytesRead = port->ReadData(buffer, messageSize * 2);
	if (bytesRead == 0) {
		state = ScalesStates::NoSignal;
		log->Log(TEXT("NScalesBU4263M1::GetWeight(): nothing is read"));
		return WEIGHTERROR;
	}
	if (bytesRead < messageSize) {
		state = ScalesStates::Timeout;
		return WEIGHTERROR;
	}
	weight = 0.0;
	//need to find such message: "23 4E xx xx xx 24"
	//DWORD weight = WEIGHTERROR;
	for (int i = 0; i <= (bytesRead - messageSize); i++) {
		if (buffer[i] == 0x23 && buffer[i + 1] == 0x4e && buffer[i + 5] == 0x24) {
			weight = (buffer[i + 2] + (buffer[i + 3] << 8) + (buffer[i + 4] << 16));
			state = ScalesStates::OK;
			CString a;
			a.Format(TEXT("Weight is parsed: %f; bytes=%d"), weight, bytesRead);
			log->Log(a);
			return (double)weight * 10; //kilogramms
		}
	}	
	state = ScalesStates::Timeout;
	return WEIGHTERROR;
}