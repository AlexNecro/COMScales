#include "stdafx.h"
#include "COMPort.h"

HRESULT COMPort::OpenPort(CString PortName, DWORD BaudRate, DWORD _timeout)
{
	//if port name is COM1-COM9, can open as-is, if COM10-COM255, must be open as \\.\COM12
	ClosePort();	
	this->timeout = _timeout;
	portName = PortName;
	if (portName.GetLength() > 4) {
		portName = CString(TEXT("\\\\.\\")) + portName;
	}
	DCB dcb;
	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);
	handle = CreateFile(portName.GetBuffer(), GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, 0, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		log->LogF(TEXT("COMPort::OpenPort(%s): error %d"), PortName.GetBuffer(), GetLastError());
		return E_FAIL;
	}
	
	HRESULT fSuccess = GetCommState(handle, &dcb);
	if (!fSuccess) {
		CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
		log->LogF(TEXT("COMPort::OpenPort(%s): can't get comm state"), PortName.GetBuffer());
		return E_FAIL;
	}
	log->LogF(TEXT("COMPort::OpenPort(%s): COMMSTATE: br=%d, bs=%d, p=%d, sb=%d"), PortName.GetBuffer(),dcb.BaudRate, dcb.ByteSize, dcb.Parity, dcb.StopBits);
	dcb.BaudRate = BaudRate;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
		

	log->LogF(TEXT("COMPort::OpenPort(%s): NEW COMMSTATE: br=%d, bs=%d, p=%d, sb=%d"), PortName.GetBuffer(), dcb.BaudRate, dcb.ByteSize, dcb.Parity, dcb.StopBits);
	fSuccess = SetCommState(handle, &dcb);
	if (!fSuccess) {
		CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
		log->LogF(TEXT("COMPort::OpenPort(%s): can't set comm state"), PortName.GetBuffer());
		return E_FAIL;
	}
	GetCommState(handle, &dcb);
	//try to read/write in sync mode
	COMMTIMEOUTS timeout = { 0 };
	timeout.ReadIntervalTimeout = 0xFFFFFFFF;
	timeout.ReadTotalTimeoutConstant = this->timeout;
	timeout.ReadTotalTimeoutMultiplier = 100;
	timeout.WriteTotalTimeoutConstant = this->timeout;
	timeout.WriteTotalTimeoutMultiplier = 0xFFFFFFFF;


	SetCommTimeouts(handle, &timeout);
	PurgeComm(handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
	return S_OK;
}

HRESULT COMPort::ClosePort()
{
	if (handle != INVALID_HANDLE_VALUE)
		try {
			CloseHandle(handle);		
		}
		catch (...) {
			;
		}
	handle = INVALID_HANDLE_VALUE;
	return S_OK;
}

COMPort::COMPort(NLog* log)
{
	this->log = log;
	handle = INVALID_HANDLE_VALUE;
}


COMPort::~COMPort()
{
	ClosePort();
}

int COMPort::ReadTailData(void* data, DWORD size)
{
	if (handle == INVALID_HANDLE_VALUE) {
		log->Log(TEXT("COMPort::ReadData(): port is not open"));
		return 0;
	}

	DWORD begin = GetTickCount();
	DWORD feedback = 0;

	char* buf = (char*)data;
	DWORD len = size;
	DWORD temp;
	COMSTAT ComState;
	ClearCommError(handle, &temp, &ComState);		
	if (ComState.cbInQue > size) { //we need only the tail of the data		
		DWORD needRead = ComState.cbInQue - size;
		char trash;
		while (needRead) {
			ReadFile(handle, &trash, 1, &feedback, NULL);
			needRead--;
		}
	}
	else if (ComState.cbInQue < size)  {
		//log->LogF(TEXT("COMPort::ReadData(): data is not ready (%d of %d)"), ComState.cbInQue, size);
		return 0;
	}
	//log->LogF(TEXT("COMPort::ReadData(): bytes available: %d, err: %d, trash: %d"), ComState.cbInQue, temp, (ComState.cbInQue - size));
	int attempts = 3;
	while (len && (attempts || (GetTickCount() - begin) < (DWORD)timeout / 3)) {

		if (attempts) attempts--;

		if (!ReadFile(handle, buf, len, &feedback, NULL)) {
			//log->Log(TEXT("COMPort::ReadData(): ReadFile failed"));
			return 0;
		}
		
		len -= feedback;
		buf += feedback;

	}
	DWORD end = GetTickCount();
	if ((end - begin) >= timeout) {
		//log->LogF(TEXT("COMPort::ReadData(): too long response (%d)"), (end - begin));
	}		
	return size - len;
}

int COMPort::WriteData(const void* data, int size)
{
	//this is just for sending commands "get weight"
	DWORD feedback;
	if (!WriteFile(handle, data, size, &feedback, 0) || feedback != size) {
		return feedback;
	}
	return feedback;
}

int COMPort::BytesAvailable(CString PortName, DWORD BaudRate)
{	
	DWORD temp;
	COMSTAT ComState;
	if (OpenPort(PortName, BaudRate)) {
		ClearCommError(handle, &temp, &ComState);
		ClosePort();
		return ComState.cbInQue;
	}
	return 0;
}

void COMPort::ClearBuffers()
{
	if (handle == INVALID_HANDLE_VALUE) {
		PurgeComm(handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
	}
}

DWORD COMPort::Dead()
{
	return 0xdead;
}

DWORD COMPort::Beef()
{
	return 0xbeef;
}

