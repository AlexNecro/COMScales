#include "stdafx.h"
#include "NScalesIND310.h"

NScalesIND310::NScalesIND310(NLog* log): IScalesHW(log)
{
	messageSize = 18;
	scaleAddress = 0x01;
}


NScalesIND310::~NScalesIND310()
{
	Disconnect();
}

long NScalesIND310::Connect(CString PortName, LONG address, LONG baudRate) {
	long ret = IScalesHW::Connect(PortName, address, baudRate);	
	scaleAddress = 0xff & address; //one byte
	if (scaleAddress == 0) scaleAddress = 0x01;
	return ret;
}

double NScalesIND310::GetWeight()
{	
#define WSTART 4
	if (!port) return 0.0;
	int bytesRead = port->ReadData(buffer, messageSize * 2);
	if (bytesRead == 0) {
		state = ScalesStates::NoSignal;
		return WEIGHTERROR;
	}
	if (bytesRead < messageSize) {
		state = ScalesStates::Timeout;
		return WEIGHTERROR;
	}
	//need to find such message: "<scaleAddress>..(15)..<CR>"
	double weight = 0.0;
	for (int i = 0; i <= (bytesRead - messageSize); i++) {
		if (buffer[i] == scaleAddress && buffer[i + 16] == 0x0D) {			
			//seems to be a message, need more tests (SWA[5]==1, SWA[6]==0. SWB[5]==1, SWC[5]==1, SWC[6]==0):
			if ((buffer[i + 1] & 0x60) == 0x20 && (buffer[i + 2] & 0x20) == 0x20 && (buffer[i + 3] & 0x60) == 0x20) {				
				byte sign = buffer[i + 2] & 0x02;
				double multiplier = 0;
				switch (buffer[i + 1] & 0x07) {
					case 1: multiplier = 10; break;
					case 2: multiplier = 1; break;
					case 3: multiplier = 0.1; break;
					case 4: multiplier = 0.01; break;
					case 5: multiplier = 0.001; break;
				}
				log->LogF(TEXT("2. NScalesIND310::GetWeight(): multiplier: %02x (%f)"), (buffer[i + 1] & 0x07), multiplier);
				if (sign) multiplier = -multiplier;
				if (multiplier == 0) continue;// we're wrong
				//weight is in text format, something like this: "    00" (six digits)
				if (buffer[i + 4] > '0' && buffer[i + 4] <= '9') weight = weight + 100000.0*(buffer[i + 4] - '0');
				if (buffer[i + 5] > '0' && buffer[i + 5] <= '9') weight = weight + 10000.0*(buffer[i + 5] - '0');
				if (buffer[i + 6] > '0' && buffer[i + 6] <= '9') weight = weight + 1000.0*(buffer[i + 6] - '0');
				if (buffer[i + 7] > '0' && buffer[i + 7] <= '9') weight = weight + 100.0*(buffer[i + 7] - '0');
				if (buffer[i + 8] > '0' && buffer[i + 8] <= '9') weight = weight + 10.0*(buffer[i + 8] - '0');
				if (buffer[i + 9] > '0' && buffer[i + 9] <= '9') weight = weight + 1.0*(buffer[i + 9] - '0');				
				weight = weight*multiplier/10; //something I don't understand
				
				state = ScalesStates::OK;
				return weight;
			}
		}
	}
	state = ScalesStates::Timeout;
	return WEIGHTERROR;
}
