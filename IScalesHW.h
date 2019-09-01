#pragma once
#include "windows.h"
#include <atlfile.h>
#include "COMPort.h"
#include "base.h"
#include "NLog.h"
using namespace ATL;
//common interface for all readers:

class IScalesHW
{
protected:
	NLog* log;
	byte *buffer;
	COMPort *port;
	ScalesStates state;
	int messageSize;
	double weight;
public:

	IScalesHW(NLog* log) {
		port = 0;
		this->log = log;
		state = ScalesStates::NotConnected;
		messageSize = 18;// just by default
		buffer = 0;
	}

	virtual ~IScalesHW() { Disconnect(); }
	virtual void ClearBuffers() { if (port) port->ClearBuffers(); }
	

	virtual HRESULT Connect(CString PortName, LONG address, LONG baudRate) {
		//common init:
		Disconnect();
		buffer = new byte[messageSize * 6];
		port = new COMPort(log);
		HRESULT res = port->OpenPort(PortName, baudRate);
		if (!SUCCEEDED(res)) {
			state = ScalesStates::PortError;
			return E_FAIL;
		}
		else {
			state = ScalesStates::NoSignal;
		};		
		return S_OK;
	}

	virtual long Disconnect() {
		//common deinit:
		if (port) {
			try {
				port->ClosePort();
				delete port;
			}
			catch (...) {

			}
			port = 0;
		}
		if (buffer) {
			try
			{
				delete[] buffer;				
			}
			catch (...)
			{

			}
			buffer = 0;
		}
		state = ScalesStates::NotConnected;
		return 0;
	}

	virtual ScalesStates GetState() { return state; }

	virtual double GetWeight() = 0;

	virtual bool GetMotion() { return state == ScalesStates::Unstable; }
}; //IScalesHW