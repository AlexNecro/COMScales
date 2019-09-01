#pragma once
//microsoft sdks\windows\v7.1a\include\shlobj.h(1151): warning C4091: 'typedef ': ignored on left of 'tagGPFIDL_FLAGS' when no variable is declared:
#pragma warning(disable: 4091)
#include <atlstr.h>
#include <atlfile.h>
#include <Shlobj.h>
#include <stdarg.h>
using namespace ATL;
enum NLogLevel {};
class NLog
{
protected:
	CAtlFile *log;
	TCHAR szPath[MAX_PATH];
	bool	logging;

public:
	NLog(CString fname, bool isFullPath, bool forDebug);
	~NLog();
	void Log(CString msg);
	void LogF(CString msg, ...);
	void Write(byte* buffer, DWORD size);
	void Start();
	void Stop();
	void Close();
	//short ToHex(char ch);
};

