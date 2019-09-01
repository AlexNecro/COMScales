#include "stdafx.h"
#include "NScalesCAS6000.h"


NScalesCAS6000::NScalesCAS6000(NLog* log) : IScalesHW(log)
{
	messageSize = 22;
	scaleAddress = 0x01;
}


NScalesCAS6000::~NScalesCAS6000()
{
	Disconnect();
}

long NScalesCAS6000::Connect(CString PortName, LONG address, LONG baudRate) {
	long ret = IScalesHW::Connect(PortName, address, baudRate);
	scaleAddress = 0xff & address; //one byte
	return ret;
}

double NScalesCAS6000::GetWeight()
{
#define WSTART 9
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
	weight = 0.0;
	//need to find such message: "ST..(15)..<CR>" - 'ST' is for "stable"
	//double weight = 0.0;
	for (int i = 0; i <= (bytesRead - messageSize); i++) {
		if (buffer[i] == 'U' && buffer[i+1] == 'S') { //unstable
			state = ScalesStates::Unstable;
			return WEIGHTERROR;
		}
		if (buffer[i] == 'O' && buffer[i+1] == 'L') { //overload
			state = ScalesStates::Overload;
			return WEIGHTERROR;
		}
		if (buffer[i] == 'S' && buffer[i+1] == 'T' && buffer[i+20]=='\r' && buffer[i+21]=='\n')	{							
			double multiplier = 1.0;
			double pos = 1.0;
			for (int d = 7; d >= 0; d--) { //walk through digits
				if (buffer[i + WSTART + d] == ' ') continue;//space
				if (buffer[i + WSTART + d] == '-') {
					multiplier = -1.0 * multiplier;
					continue;//sign
				};
				if (buffer[i + WSTART + d] == '.') {
					switch (d) {
						case 6: multiplier *= 0.1;
								break;
						case 5: multiplier *= 0.01;
							break;
						case 4: multiplier *= 0.001;
							break;
						case 3: multiplier *= 0.0001;
							break;
						case 2: multiplier *= 0.00001;
							break;
						case 1: multiplier *= 0.000001;
							break;
						case 0: multiplier *= 0.0000001;
							break;

					}
					continue;//decimal point
				};
				if (buffer[i + WSTART + d] >= '0' && buffer[i + WSTART + d] <= '9') {
					pos *= 10;
					weight = weight + pos*(buffer[i + WSTART + d] - '0');
				}
			}
			weight = weight;//convert to kgs

			state = ScalesStates::OK;
			return weight;			
		}
	}
	state = ScalesStates::Timeout;
	return WEIGHTERROR;
}