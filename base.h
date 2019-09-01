#pragma once

#include <atlstr.h>
#include <ATLComTime.h>
enum LicenseState { NotLicensed, Trial, Expired, Hardware };
enum DeviceType { None = 0, Test, BU4263M1, IND310, CAS6000, HBT9, FT11, VT220 };
enum ScalesStates { NotConnected, PortError, NoSignal, Unstable, Overload, OK, GenericError, Timeout, NoData, NoLicense, TrialLicense, WrongSignal};

struct Measurement {
	DATE DateTime;
	double weight;
	ScalesStates state;
	Measurement(double _weight, ScalesStates _state) {
		weight = _weight;
		state = _state;
		weight = _weight;
		SYSTEMTIME st;
		GetLocalTime(&st);
		DateTime = *(new ATL::COleDateTime(st));
	}

	Measurement() {
		weight = 0.0;
		state = ScalesStates::GenericError;
		SYSTEMTIME st;
		GetLocalTime(&st);
		DateTime = *(new ATL::COleDateTime(st));
	}

	bool isOK() {
		if (state == ScalesStates::OK) {
			//|| state == ScalesStates::Timeout) {
			return true;
		}
		return false;
	}

	void scale(double multiplier) {
		weight *= multiplier;
	}

	bool equals(const Measurement& _weight) const {
		return _weight.weight == weight;
	}

	//bool isWaiting(DWORD lastTimeSuccess, DWORD maxWaitTime = 3000) { //no signal, but timeout is not expired
//		if ((state == ScalesStates::NoSignal || state == ScalesStates::Timeout) && ((GetTickCount() - lastTimeSuccess) < maxWaitTime)) return true;
//		return false;
//	}
};//class Measurement