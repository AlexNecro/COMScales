#include "stdafx.h"
#include "NLog.h"


NLog::NLog(CString fname, bool isFullPath, bool forDebug)
{
	log = 0;
	logging = false;
#ifndef DEBUG
	if (forDebug) return;
#endif // DEBUG
	if (!isFullPath) {
		if (SUCCEEDED(SHGetFolderPath(NULL,
			CSIDL_PERSONAL | CSIDL_FLAG_CREATE,
			NULL,
			0,
			szPath)))
		{
			PathAppend(szPath, fname.GetBuffer());
		}
	}
	else {
		_tcscpy_s(szPath, MAX_PATH, fname.GetBuffer());
	}
	try {
		log = new CAtlFile();
		HRESULT res = log->Create(
			szPath,
			GENERIC_WRITE,
			FILE_SHARE_READ,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL);
		if (!SUCCEEDED(res)) {
			delete log;
			log = 0;
			logging = false;
		}
	}
	catch (...) {
		logging = false;
		log = 0;
	}	
}


NLog::~NLog()
{
	Close();
}

void NLog::Start() {
	if (log) {
		logging = true;
		SYSTEMTIME st;
		GetLocalTime(&st);
		CString str;
		str.Format(TEXT("%04d.%02d.%02d %02d:%02d:%02d.%04d "), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		Log(str);
	}
}

void NLog::Stop() {
	logging = false;
}

void NLog:: Log(CString msg)
{
	if (logging)
	{
		CString str;
		SYSTEMTIME st;
		GetLocalTime(&st);
		str.Format(TEXT("%02d:%02d:%02d.%04d "), st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		log->Write(str.GetBuffer(), str.GetLength()*sizeof(TCHAR));
		log->Write(msg.GetBuffer(), msg.GetLength()*sizeof(TCHAR));
		log->Write(TEXT("\r\n"), 4);
		log->Flush();
	}
	else {
	}
}

void NLog::LogF(CString msg, ...)
{
	if (logging) {
		va_list vl;
		va_start(vl, msg);
		CString str;
		str.FormatV(msg, vl);
		va_end(vl);
		Log(str);
	}
}

void NLog::Write(byte * buffer, DWORD size)
{
	if (logging) {
		CString str, temp;		
		for (DWORD i = 0; i < size; i++) {			
			temp.Format(TEXT("%02x "), buffer[i]);
			str = str + temp;
		}
		CString msg;		
		LogF(TEXT("%d: [%s]"), size, str);		
	}
}

void NLog :: Close() {
	if (log) {
		log->Close();
		log = 0;
		logging = false;
	}
}