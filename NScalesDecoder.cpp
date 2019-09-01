#include "stdafx.h"
#include "NScalesDecoder.h"
ScalesSettings ScalesSettings::Test = ScalesSettings(DeviceType::Test, TEXT("Тест"), 0);
ScalesSettings ScalesSettings::BU4263M1 = ScalesSettings(DeviceType::BU4263M1, TEXT("БУ 4263 М1"), 6);
ScalesSettings ScalesSettings::CAS6000 = ScalesSettings(DeviceType::CAS6000, TEXT("CAS-6000"), 22);
ScalesSettings ScalesSettings::FT11 = ScalesSettings(DeviceType::FT11, TEXT("FT11"), 18);
ScalesSettings ScalesSettings::HBT9 = ScalesSettings(DeviceType::HBT9, TEXT("HBT-9"), 14);
ScalesSettings ScalesSettings::IND310 = ScalesSettings(DeviceType::IND310, TEXT("Mettler Toledo IND310"), 18);
ScalesSettings ScalesSettings::VT220 = ScalesSettings(DeviceType::VT220, TEXT("VT-220"), 10);

std::vector<ScalesSettings*>* ScalesSettings::all = 0;
int ScalesSettings::count = 0;

ScalesSettings::ScalesSettings(DeviceType deviceType, ATL::CString name, int messagesize) {
	this->messageSize = messagesize;
	this->deviceType = deviceType;
	this->name = name;
	if (all == 0) all = new std::vector<ScalesSettings*>();
	all->push_back(this);
	count++;
}

ScalesSettings& ScalesSettings::getByType(DeviceType deviceType) {
	for (unsigned i = 0; i < all->size(); i++) {
		if (deviceType == all->at(i)->deviceType) return *(all->at(i));
	};
	return ScalesSettings::Test;
}

ScalesSettings& ScalesSettings::getByName(ATL::CString name) {
	for (unsigned i = 0; i < all->size(); i++) {
		if (name.CompareNoCase(all->at(i)->name)==0) return *(all->at(i));
	};
	return ScalesSettings::Test;
}

ATL::CString ScalesSettings::getAllNames() {
	ATL::CString names;
	for (unsigned i = 0; i < all->size(); i++) {
		names.Append(all->at(i)->name);
		if (i < all->size() - 1) names.Append(TEXT(","));
	};
	return names;
}

NScalesDecoder::NScalesDecoder()
{
}


NScalesDecoder::~NScalesDecoder()
{
}

Measurement NScalesDecoder::DecodeWeight(DeviceType deviceType, byte* buffer, int bytesRead)
{
	switch (deviceType) {
		case DeviceType::BU4263M1:
			return DecodeWeightBU4263M1(buffer, bytesRead, ScalesSettings::BU4263M1);
			break;
		case DeviceType::IND310:
			return DecodeWeightIND310(buffer, bytesRead, ScalesSettings::IND310);
			break;
		case DeviceType::HBT9:
			return DecodeWeightHBT9(buffer, bytesRead, ScalesSettings::HBT9);
			break;
		case DeviceType::CAS6000:
			return DecodeWeightCAS6000(buffer, bytesRead, ScalesSettings::CAS6000);
			break;
		case DeviceType::FT11:
			return DecodeWeightFT11(buffer, bytesRead, ScalesSettings::FT11);
			break;
		case DeviceType::VT220:
			return DecodeWeightVT220(buffer, bytesRead, ScalesSettings::VT220);
			break;
		default:
			return DecodeWeightTest(buffer, bytesRead);
	}	
}

Measurement NScalesDecoder::DecodeWeightTest(byte * buffer, int bytesRead)
{
	static double weight = 1234.5678;
	static DWORD lastTick = GetTickCount();
	if ((GetTickCount() - lastTick) >= 2000) {		
		weight = 1.0*rand() / 1000;
		lastTick = GetTickCount();
	}
	return Measurement(weight, ScalesStates::OK);
}

Measurement NScalesDecoder::DecodeWeightBU4263M1(byte * buffer, int bytesRead, const ScalesSettings& settings)
{	
	Measurement w;
	if (bytesRead < settings.messageSize) {
		w.state = ScalesStates::NoData; //port is empty or data format is not recognized
		return w;
	}

	//need to find such message: "23 4E xx xx xx 24"	
	for (int i = 0; i <= (bytesRead - settings.messageSize); i++) {
		if (buffer[i] == 0x23 && buffer[i + 1] == 0x4e && buffer[i + 5] == 0x24) {
			if (buffer[i + 2] == 0xfe && buffer[i + 3] == 0xff && buffer[i + 4] == 0xff) {//23 4E FE FF FF 24, 167772.147969
				w.state = ScalesStates::WrongSignal;
			} else {
				w.weight = 10.0*(buffer[i + 2] + (buffer[i + 3] << 8) + (buffer[i + 4] << 16));			
				w.state = ScalesStates::OK;
			}			
			return w;
		}
	}	
	return w;
}

Measurement NScalesDecoder::DecodeWeightCAS6000(byte * buffer, int bytesRead, const ScalesSettings& settings)
{
#define WSTART 9
	Measurement w;
	if (bytesRead < settings.messageSize) {
		w.state = ScalesStates::NoData;
		return w;
	}	

	for (int i = 0; i <= (bytesRead - settings.messageSize); i++) {
		if (buffer[i] == 'U' && buffer[i + 1] == 'S') { //unstable
			w.state = ScalesStates::Unstable;
			return w;
		}
		if (buffer[i] == 'O' && buffer[i + 1] == 'L') { //overload
			w.state = ScalesStates::Overload;
			return w;
		}
		if (buffer[i] == 'S' && buffer[i + 1] == 'T' && buffer[i + 20] == '\r' && buffer[i + 21] == '\n') {
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
					w.weight = w.weight + pos*(buffer[i + WSTART + d] - '0');
				}
			}			
			w.state = ScalesStates::OK;
			return w;
		}
	}
	
	return w;
}

Measurement NScalesDecoder::DecodeWeightFT11(byte * buffer, int bytesRead, const ScalesSettings& settings)
{
	Measurement w;
	if (bytesRead < settings.messageSize) {
		w.state = ScalesStates::NoData;
		return w;
	}
	
	for (int i = 0; i <= (bytesRead - settings.messageSize); i++) {		
		if (buffer[i] == 0x02) {
			if (buffer[i + 16] == 0x0D) {
				if (buffer[i + 4] > '0' && buffer[i + 4] <= '9') w.weight = w.weight + 100000.0*(buffer[i + 4] - '0');
				if (buffer[i + 5] > '0' && buffer[i + 5] <= '9') w.weight = w.weight + 10000.0*(buffer[i + 5] - '0');
				if (buffer[i + 6] > '0' && buffer[i + 6] <= '9') w.weight = w.weight + 1000.0*(buffer[i + 6] - '0');
				if (buffer[i + 7] > '0' && buffer[i + 7] <= '9') w.weight = w.weight + 100.0*(buffer[i + 7] - '0');
				if (buffer[i + 8] > '0' && buffer[i + 8] <= '9') w.weight = w.weight + 10.0*(buffer[i + 8] - '0');
				if (buffer[i + 9] > '0' && buffer[i + 9] <= '9') w.weight = w.weight + 1.0*(buffer[i + 9] - '0');
				w.state = ScalesStates::OK;
				return w;
				break;				
			}
		}
	}		
	return w;
}

Measurement NScalesDecoder::DecodeWeightHBT9(byte * buffer, int bytesRead, const ScalesSettings& settings)
{
	Measurement w;
	if (bytesRead < settings.messageSize) {
		w.state = ScalesStates::NoData;
		return w;
	}

	for (int i = 0; i <= (bytesRead - settings.messageSize); i++) {
		if (buffer[i] == 'w'
			&& buffer[i + 10] == 'k' && buffer[i + 11] == 'g') {

			double mult = 1000000.0;
			double div = 1.0;
			int pointPos = 0;
			for (int q = 2; q < 10; q++) {
				if (buffer[i + q] >= '0' && buffer[i + q] <= '9') {
					w.weight += mult*(buffer[i + q] - '0');
					mult /= 10.0;
				}
				else if (buffer[i + q] == '.') {
					pointPos = q;
					div = mult * 10;
				}

			}			
			w.weight /= div;
			w.state = ScalesStates::OK;
			return w;
			break;
		}
	}
	
	return w;
}

Measurement NScalesDecoder::DecodeWeightIND310(byte * buffer, int bytesRead, const ScalesSettings& settings)
{
	Measurement w;
	if (bytesRead < settings.messageSize) {
		w.state = ScalesStates::NoData;
		return w;
	}

	byte scaleAddress = 0x01;

	for (int i = 0; i <= (bytesRead - settings.messageSize); i++) {
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
				if (sign) multiplier = -multiplier;
				if (multiplier == 0) continue;// we're wrong
											  //weight is in text format, something like this: "    00" (six digits)
				if (buffer[i + 4] > '0' && buffer[i + 4] <= '9') w.weight = w.weight + 100000.0*(buffer[i + 4] - '0');
				if (buffer[i + 5] > '0' && buffer[i + 5] <= '9') w.weight = w.weight + 10000.0*(buffer[i + 5] - '0');
				if (buffer[i + 6] > '0' && buffer[i + 6] <= '9') w.weight = w.weight + 1000.0*(buffer[i + 6] - '0');
				if (buffer[i + 7] > '0' && buffer[i + 7] <= '9') w.weight = w.weight + 100.0*(buffer[i + 7] - '0');
				if (buffer[i + 8] > '0' && buffer[i + 8] <= '9') w.weight = w.weight + 10.0*(buffer[i + 8] - '0');
				if (buffer[i + 9] > '0' && buffer[i + 9] <= '9') w.weight = w.weight + 1.0*(buffer[i + 9] - '0');
				w.weight = w.weight*multiplier / 10; //something I don't understand

				w.state = ScalesStates::OK;
				return w;
			}
		}
	}
	
	return w;
}

Measurement NScalesDecoder::DecodeWeightVT220(byte * buffer, int bytesRead, const ScalesSettings& settings)
{
	Measurement w;
	if (bytesRead < settings.messageSize) {
		w.state = ScalesStates::NoData;
		return w;
	}
	
	for (int i = 0; i <= (bytesRead - settings.messageSize); i++) {
		if ((buffer[i] == 'T' || buffer[i] == 'P') && buffer[i + 5] == '.') {
			w.weight
				= 100.0*(buffer[i + 2] - '0')
				+ 10.0*(buffer[i + 3] - '0')
				+ 1.0*(buffer[i + 4] - '0') //tonnes!
				+ 0.1*(buffer[i + 6] - '0')
				+ 0.01*(buffer[i + 7] - '0');
			w.weight = w.weight * 1000;//kgs
			w.state = ScalesStates::OK;
			return w;
			break;
		}
	}
	
	return w;
}
