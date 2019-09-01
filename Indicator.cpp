#include "stdafx.h"
#include "Indicator.h"


Indicator::Indicator(NLog* log)
{
	port = 0;
	this->log = log;
	SetSecret(0);
}


Indicator::~Indicator()
{
	Disconnect();
}

bool Indicator::Connect(CString PortName, LONG baudRate) {
	digits = 6;
	canText = false;
	Disconnect();
	port = new COMPort(log);
	HRESULT res = port->OpenPort(PortName, baudRate);
	if (!SUCCEEDED(res)) {
		delete port;
		port = 0;
		return false;
	}		
	return true;
}

void Indicator::Disconnect() {
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
	return;
}

void Indicator::Print(CString text) {
	if (!port) return;
	CString a;
	a.Format(TEXT("p %s\n"), text);
	int len = WideCharToMultiByte(CP_UTF8,0,a.GetBuffer(),-1,0,0,0,0);
	char* buffer = new char[len];
	WideCharToMultiByte(CP_UTF8, 0, a.GetBuffer(), -1, buffer, len, 0, 0);	
	Encode(buffer, len);
	port->WriteData(buffer, len);
}

void Indicator::PrintWeight(double weight) {
	CString a;
	a.Format(TEXT("%.3f"), weight);
	Print(a);
}

void Indicator::SetDisplayTime(int millis) {
	CString str;
	str.Format(TEXT("to %d"), millis);	
	Print(str);
}

int Indicator::GetDigits()
{
	return digits;//must ask for number of digits
}

bool Indicator::CanText()
{
	return canText;//must ask capability to write alpha chars
}

void Indicator::Encode(char* buffer, int size) {
	for (int i = 0; i<size; i++) {
		buffer[i] = buffer[i] ^ key[i % 4];
	}
}

void Indicator::Decode(char* buffer, int size) {
	for (int i = 0; i<size; i++) {
		buffer[i] = buffer[i] ^ key[i % 4];
	}
}

void Indicator::SetSecret(DWORD secret) {
	this->key[0] = (secret >> 24) & 0xff;
	this->key[1] = (secret >> 16) & 0xff;
	this->key[2] = (secret >> 8) & 0xff;
	this->key[3] = (secret)& 0xff;
}