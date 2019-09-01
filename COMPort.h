#pragma once
#include "stdafx.h"
#include <vector>
#include <atlfile.h>
#include "NLog.h"
using namespace ATL;
using namespace std;
class COMPort
{
protected:
	NLog* log;
	ATL::CString portName;
	HANDLE handle;
	DWORD timeout;
	DWORD baudRate;
		
public:
	COMPort(NLog* log);
	~COMPort();
	// sets comport settings		
	HRESULT OpenPort(CString PortName, DWORD BaudRate = CBR_9600, DWORD timeout = 200);
	HRESULT ClosePort();
	int ReadTailData(void* data, DWORD size); //reads *last* _size available bytes, writes data to _data pointer, returns length
	int WriteData(const void* data, int size);
	int BytesAvailable(CString PortName, DWORD BaudRate = CBR_9600); //opens port and returns number of bytes available, closes port
	void ClearBuffers();
	static DWORD Dead();
	static DWORD Beef();
};

