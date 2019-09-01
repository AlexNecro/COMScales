// Scales.h : Declaration of the CScales

#pragma once
//microsoft sdks\windows\v7.1a\include\shlobj.h(1151): warning C4091: 'typedef ': ignored on left of 'tagGPFIDL_FLAGS' when no variable is declared:
#pragma warning(disable: 4091)
#include "resource.h"       // main symbols



#include "COMScales_i.h"
#include <atlfile.h>
#include <Shlobj.h>
#include "Indicator.h"
#include "NScalesDecoder.h"
//#include "NScalesBU4263M1.h"
//#include "NScalesIND310.h"
//#include "NScalesTest.h"
//#include "NScalesCAS6000.h"
//#include "NScalesHBT9.h"
//#include "NScalesFT11.h"
//#include "NScalesVT220.h"

//use separate thread for measurement:
#define MODEL_SEPARATE 1
#define MAX_HISTORY 999999
#define BUILD_MONTH (\
  __DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 1 : 6) \
: __DATE__ [2] == 'b' ? 2 \
: __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
: __DATE__ [2] == 'y' ? 5 \
: __DATE__ [2] == 'l' ? 7 \
: __DATE__ [2] == 'g' ? 8 \
: __DATE__ [2] == 'p' ? 9 \
: __DATE__ [2] == 't' ? 10 \
: __DATE__ [2] == 'v' ? 11 \
: 12)

#define BUILD_YEAR \
    ( \
        (__DATE__[ 7] - '0') * 1000 + \
        (__DATE__[ 8] - '0') *  100 + \
        (__DATE__[ 9] - '0') *   10 + \
        (__DATE__[10] - '0') \
    )

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

//WARNING!!!! http://stackoverflow.com/questions/34074204/visual-studio-2015-cannot-produce-an-atl-dll-that-successfully-registers-on-wind
//to register dll with WindowsXP, this must be done:
//Link the CRT statically - in Config Properties / C / C++ / Code Generation / Runtime Library select the non - DLL multi - threaded option, for example / MT in the release build.
//Add / Zc:threadSafeInit - (n.b.the trailing - is part of the switch) to the compiler options under Config Properties / C / C++ / Command Line / Additional Options.
//Necro, 29.11.2016

using namespace ATL;

class ATL_NO_VTABLE CScales :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CScales, &CLSID_Scales>,
	public IDispatchImpl<IScales, &IID_IScales, &LIBID_COMScalesLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
protected:
	NLog* log;
	NLog* logWeight;
	DOUBLE lastGoodWeight;	
	std::vector<Measurement> history;
	
	bool isConnected;
	COMPort *scalesPort;
	byte *scalesBuffer;
	DWORD scalesBufferSize;
	DWORD licensed0;
	DeviceType	deviceType;
	LicenseState licenseState;
	Indicator *indicator;
	float weightMult;
	float echoMult;
	DWORD lastTimeSuccess;
	DWORD maxTimeSuccess;
	//thread data:
	CRITICAL_SECTION criticalSection;
	bool threadStarted;
	int readInterval; //200ms seems to be good
	HANDLE workerThread;

	void CreateScales();
	void DeleteScales();
	static DWORD WINAPI ThreadFunction(LPVOID param);
	Measurement ReadWeight();
	Measurement HandleWeight(Measurement& weight);//use like  HandleWeight(ReadWeight())
	Measurement TestWeight(Measurement& weight);
	bool IsWeightChanged(const Measurement& weight);
	void WriteWeightToFile(const Measurement& weight);
	bool isInit();
	void Log(CString msg) { log->Log(msg); }
	DWORD Licensed(DWORD token) { return (licensed0 << token) + 0xBEEF; }
	DeviceType GetDeviceTypeByName(TCHAR* name);
	std::vector<CString> GetPorts();
public:
	CScales();
	~CScales();	

DECLARE_REGISTRY_RESOURCEID(IDR_SCALES)


BEGIN_COM_MAP(CScales)
	COM_INTERFACE_ENTRY(IScales)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:



	STDMETHOD(InitModule)(BSTR KeyString, VARIANT_BOOL* OK);
	STDMETHOD(DoneModule)();
	STDMETHOD(ConnectDevice)(BSTR DeviceType, BSTR PortName, /*LONG Address,*/ LONG BaudRate, FLOAT multiplier);
	STDMETHOD(DisconnectDevice)();	
	STDMETHOD(GetWeight)(DOUBLE* AWeight);	
	STDMETHOD(IsStable)(VARIANT_BOOL* AMotion);	/*DELETE*/	
	STDMETHOD(GetStateInfo)(BSTR* strState);
	STDMETHOD(SetZero)();/*DELETE*/	
	STDMETHOD(GetVersionInfo)(BSTR* strInfo);
	STDMETHOD(GetHelp)(BSTR* strInfo);
	STDMETHOD(ListSupportedDevices)(BSTR* strInfo);
	STDMETHOD(ListPorts)(BSTR* strInfo);
	STDMETHOD(IsOK)(VARIANT_BOOL* OK);
	STDMETHOD(ConnectIndicator)(BSTR PortName, FLOAT Multiplier);
	STDMETHOD(DisconnectIndicator)();	
	STDMETHOD(ConnectLogFile)(BSTR Path);
	STDMETHOD(GetHistoryLength)(LONG* Length);//get number of stored measurements
	STDMETHOD(GetHistoryTimeN)(DWORD Index, DATE* DateTime);//get measure time by index
	STDMETHOD(GetHistoryWeightN)(DWORD Index, DOUBLE* Weight);//get measurement by index
	STDMETHOD(IsPortConnected)(BSTR PortName, LONG BaudRate, VARIANT_BOOL* HasData);
};

OBJECT_ENTRY_AUTO(__uuidof(Scales), CScales)
