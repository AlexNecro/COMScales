// Scales.cpp : Implementation of CScales

#include "stdafx.h"
#include "Scales.h"
#include "LicenseInfo.h"
#include "TestScalesDecoder.h"

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
#define BUFFER_SIZE 128
#define COMDEBUG
// CScales
/*TODO:
	move all sacales interconnection (ThreadFunction(), GetWeight(), decoders, COM ports, etc) to separate class, so CScales can connect several scales
	add functionality to connect several scales and measure by several scales
	do that with indicators too
*/

/*===========================
decoder states:
1. signal valid
2. signal unrecognized
3. no signal (or too short)

next, CScales must skip invalid signals, if timeout not reached, or set weight to zero and error to true
============================*/
DWORD WINAPI CScales::ThreadFunction(LPVOID param) {
	CScales *self = (CScales*)param;
	self->log->Log(TEXT("ThreadFunction(): started"));
	while (self->threadStarted) {
		if (!self->isInit()) {//wait for connect
			Sleep(self->readInterval*10);
			continue;
		}
		Measurement weight = self->ReadWeight();//can take long time!
		if (weight.isOK()) { // signal is not empty and is decoded by decoder
			weight = self->TestWeight(weight); //dummy function
			weight.scale(self->weightMult); //SCALE ONCE!!!!
			//================= ENTER CRITICAL SECTION ==========================
			EnterCriticalSection(&(self->criticalSection));
			if (self->IsWeightChanged(weight)) { //weight changed, need to handle it somehow:				
				self->HandleWeight(weight);				
			}
			else {
				self->lastTimeSuccess = GetTickCount(); //must renew time
			}
			LeaveCriticalSection(&(self->criticalSection));
			//================= LEAVE CRITICAL SECTION ==========================
			if (self->scalesPort)
				self->scalesPort->ClearBuffers();//if read something - clear all unread infos	
			//and echo it to indicator (this must be rewritten):
			if (self->indicator && self->echoMult > 0) {
				//here must be code to reformat number according to indicator size (number of digits)
				self->indicator->PrintWeight(weight.weight*self->echoMult);
			}
		}
		else { //so, weight is bad!
			//...
		}
		Sleep(self->readInterval);
		//maybe it's good idea to increase read interval if read fails too often?
	}
	self->log->Log(TEXT("ThreadFunction(): exited"));
	return 0;
}

CScales::CScales()
{
	licenseState = LicenseState::Trial;
	licensed0 = 0xEFFE;
	deviceType = DeviceType::None;
	scalesPort = 0;	
	readInterval = 200;//ms

	SYSTEMTIME st;
	GetSystemTime(&st);
/*#ifdef DEBUG
	if (st.wYear > BUILD_YEAR && st.wMonth > BUILD_MONTH + 1) {
		licenseState = LicenseState::Expired;
	}
	else {
		licenseState = LicenseState::Trial;
		licensed0 = 0xDEAD;
	}
#else
	licenseState = LicenseState::Hardware;
	licensed0 = 0xDEAD;
	TCHAR szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL,
		CSIDL_PERSONAL | CSIDL_FLAG_CREATE,
		NULL,
		0,
		szPath)))
	{
		PathAppend(szPath, licenseNoticeName);
	}
	try {
		CAtlFile file;
		HRESULT res = file.Create(
			szPath,
			GENERIC_WRITE,
			FILE_SHARE_READ,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL);
		if (SUCCEEDED(res)) {
			file.Write(licenseNotice, strlen(licenseNotice));
			file.Close();
		}
	} catch (...) {
	}
#endif // DEBUG */
	scalesPort = 0;
	scalesBuffer = 0;
	indicator = 0;
	weightMult = 1.0;
	echoMult = 1.0;
	workerThread = 0;
	log = new NLog(TEXT("NScales.log"), false, true);
	log->Start();
	log->LogF(TEXT("Build on: %02d.%04d"), BUILD_MONTH, BUILD_YEAR);
	logWeight = 0;
	lastGoodWeight = 0.0;
}

CScales::~CScales() {
	DoneModule();
	Log(TEXT("Destroyed"));
	log->Close();
	delete log;
	if (logWeight) {
		logWeight->Close();
		delete logWeight;
	}
}

Measurement CScales::TestWeight(Measurement& weight)
{	
	/*double mult = 0.0;
	if (Licensed(16) == ((COMPort::Dead() << 16) + COMPort::Beef())) mult += 1.0;
	weight.scale(mult);*/
	return weight;
}

STDMETHODIMP CScales::InitModule(BSTR KeyString, VARIANT_BOOL* OK)
{
	CString str;
	COLE2T tempstr(KeyString);
	if (licenseState != LicenseState::Trial && _tcscmp(KeyString, licenseKey) != 0) licenseState = LicenseState::NotLicensed;	
	//if (_tcscmp(KeyString, licenseKey) != 0) licenseState = LicenseState::NotLicensed;
	log->LogF(TEXT("InitModule(%s): License: %d"), (TCHAR*)tempstr, licenseState);	
	if (!(licenseState == LicenseState::Hardware || licenseState == LicenseState::Trial)) {
		*OK = VARIANT_FALSE;
		return E_FAIL;
	}
	else {		
		*OK = VARIANT_TRUE;
	}	
	//create thread:
	if (MODEL_SEPARATE == 1) {
		if (!InitializeCriticalSectionAndSpinCount(&criticalSection, 0)) {
			Log(TEXT("InitModule(): can't init critical section!!!"));			
		}
		threadStarted = true;
		workerThread = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			CScales::ThreadFunction,// thread function name
			this,					// argument to thread function 
			0,		// creation flags (CREATE_SUSPENDED?)
			NULL);
	}
	try {
#ifdef DEBUG
		for (DWORD i = 0; i < ScalesSettings::all->size(); i++)
			TestScalesDecoder::Test(log, *ScalesSettings::all->at(i));
#endif
	}
	catch (...) {

	}
	return S_OK;
}


STDMETHODIMP CScales::DoneModule()
{
	// TODO: Add your implementation code here
	DisconnectDevice();	
	if (MODEL_SEPARATE == 1 && workerThread) {
		//how to stop thread?		
		Log(TEXT("DoneModule(): stopping thread"));
		threadStarted = false;
		WaitForSingleObject(workerThread, 1000);
		CloseHandle(workerThread);
		workerThread = 0;
		DeleteCriticalSection(&criticalSection);
	}
	Log(TEXT("DoneModule(): done"));
	return S_OK;
}

void CScales::CreateScales() {
	if (deviceType == DeviceType::Test) return;
	void *buffer;
	scalesBufferSize = ScalesSettings::getByType(deviceType).messageSize * 2;
	if (scalesBufferSize)
		scalesBuffer = (byte*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BUFFER_SIZE);
	else
		scalesBuffer = 0;
	buffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(COMPort));
	scalesPort = new(buffer)COMPort(log);	
}

STDMETHODIMP CScales::ConnectDevice(BSTR strDeviceType, BSTR PortName, /*LONG Address,*/ LONG BaudRate, FLOAT multiplier)
{
	if (!(licenseState == LicenseState::Hardware || licenseState == LicenseState::Trial)) {
		return E_FAIL;
	}
	DisconnectDevice();
	CString str;
	COLE2T tempstr(strDeviceType);
	COLE2T tempport(PortName);
	deviceType = GetDeviceTypeByName(tempstr);
	str.Format(TEXT("ConnectDevice(%s, %s, %d, %d, %f): %d"), (TCHAR*)tempstr, (TCHAR*)tempport, 0, BaudRate, multiplier, deviceType);
	Log(str);
	CreateScales();
	if (deviceType!=DeviceType::Test) {
		if (!SUCCEEDED(scalesPort->OpenPort((TCHAR*)tempport, BaudRate))) {
			Log(TEXT("OpenPort() failed"));
			DeleteScales();
			return E_FAIL;
		}
	}
	if (multiplier > 0) weightMult = multiplier;
	else weightMult = 1.0;	
	lastTimeSuccess = GetTickCount();
	maxTimeSuccess = 3000; //allow lag
	lastGoodWeight = 0.0;
	return S_OK;
}

void CScales::DeleteScales() {			
	if (scalesBuffer) {
		HeapFree(GetProcessHeap(), 0, scalesBuffer);
		scalesBuffer = 0;
	}
	if (scalesPort) {
		HeapFree(GetProcessHeap(), 0, scalesPort);
		scalesPort = 0;
	}	
}

STDMETHODIMP CScales::DisconnectDevice()
{
	// TODO: Add your implementation code here	
	if (scalesPort) {
		Log(TEXT("DisconnectDevice()"));
		scalesPort->ClosePort();
		DeleteScales();			
	};
	return S_OK;
}

Measurement CScales::ReadWeight() { //reads weight from com-port, this function must be called in thread, asynchronous - 
	Measurement weight;
	try {
		//read data from port:
		if (deviceType == DeviceType::Test) {
			weight = NScalesDecoder::DecodeWeight(deviceType, 0, 0);
		}
		else {
			if (!isInit()) return weight;
			int bytesRead = scalesPort->ReadTailData(scalesBuffer, scalesBufferSize);
#ifdef COMDEBUG
			log->Log(TEXT("COMPort is empty!"));
#endif
			if (bytesRead == 0) {
				weight.state = ScalesStates::NoData;
				return weight;
		}
#ifdef COMDEBUG
			log->Write(scalesBuffer, bytesRead);
#endif
			weight = NScalesDecoder::DecodeWeight(deviceType, scalesBuffer, bytesRead);
		}
	}
	catch (...) {
		Log(TEXT("catched exception on ReadWeight()"));
	}
	return weight;
}//ReadWeight() 

bool CScales::IsWeightChanged(const Measurement& weight) {
	if (history.empty()) return true;	
	log->LogF(TEXT("IsWeightChanged: %f != %f = %d"), weight.weight, history.back().weight, !weight.equals(history.back()));
	//lastTimeSuccess = GetTickCount(); //this must guarded by CS too!
	return !weight.equals(history.back());
}

Measurement CScales::HandleWeight(Measurement& weight) {	//this is blocking function, must be guarded by critical section	
	//weight is changed and state is OK
	log->LogF(TEXT("HandleWeight(): %f, %d"), weight.weight, weight.state);
	if (history.size() >= MAX_HISTORY) {
		history.clear();
	}
	if (weight.state == ScalesStates::OK) {
		lastGoodWeight = weight.weight;
		lastTimeSuccess = GetTickCount();
	}
	history.push_back(weight); //save to history
	WriteWeightToFile(weight); //and write to file
	return weight;
}//HandleWeight()

void CScales::WriteWeightToFile(const Measurement& weight)
{
	if (logWeight) {
		logWeight->LogF(TEXT("%f"), weight.weight);
	}
}//WriteWeightToFile()

STDMETHODIMP CScales::GetWeight(DOUBLE* AWeight)
{		
	if (!isInit()) return E_FAIL;
	if (MODEL_SEPARATE) {
		//just return weight
		if (history.empty()) {
			*AWeight = 0.0;
		}
		else {
			EnterCriticalSection(&criticalSection);
			if ((GetTickCount() - lastTimeSuccess) < maxTimeSuccess) {
				*AWeight = lastGoodWeight;
				log->LogF(TEXT("GetWeight(): %f"), *AWeight);
			}
			else {
				*AWeight = 0.0;
				log->LogF(TEXT("GetWeight(): %f (timeout (%d), return 0.0)"), lastGoodWeight, (GetTickCount() - lastTimeSuccess));
			}
			LeaveCriticalSection(&criticalSection);
		}		
		return S_OK;
	}
	else {
		//read weight
		*AWeight = 0.0;//HandleWeight(ReadWeight()).weight;
	}	
	return S_OK;
}

STDMETHODIMP CScales::IsStable(VARIANT_BOOL * AMotion)
{
	/*if (!scalesPort) return E_FAIL;
	CString a;
	a.Format(TEXT("IsStable(): %s"), stable?TEXT("YES"):TEXT("NO"));
	Log(a);	
	if (stable)
		*AMotion = VARIANT_TRUE;
	else
		*AMotion = VARIANT_FALSE;*/
	return S_OK;
}

STDMETHODIMP CScales::GetStateInfo(BSTR* strState)
{	
	if (licenseState == LicenseState::Expired) {
		*strState = SysAllocString(TEXT("Срок действия лицензии истёк"));
		log->Log(TEXT("GetStateInfo(): license expired"));
	}
	else if (licenseState == LicenseState::NotLicensed) {
		*strState = SysAllocString(TEXT("Лицензия не обнаружена"));
		log->Log(TEXT("GetStateInfo(): no license"));
	}
	else if (!isInit()) {
		*strState = SysAllocString(TEXT("Компонента не инициализирована"));
		log->Log(TEXT("GetStateInfo(): not inited"));
	}
	else if (history.empty()) {
		*strState = SysAllocString(TEXT("Данные ещё не получены"));
		log->Log(TEXT("GetStateInfo(): no data"));
	}
	else {
		try {
			if ((GetTickCount() - lastTimeSuccess) >= maxTimeSuccess) {
				*strState = SysAllocString(TEXT("Истекло время ожидания"));
			} else {
			Measurement weight = history.back();
			log->LogF(TEXT("GetStateInfo(): w=%f s=%d"), weight.weight, weight.state);
			switch (weight.state) {
			case ScalesStates::OK:
				*strState = SysAllocString(TEXT("OK"));
				break;
			case ScalesStates::Unstable:
				*strState = SysAllocString(TEXT("Вес не стабилизирован"));
				break;			
			case ScalesStates::NoSignal:
				*strState = SysAllocString(TEXT("Нет сигнала от весов"));
				break;
			case ScalesStates::NotConnected:
				*strState = SysAllocString(TEXT("Нет связи с весами"));
				break;
			case ScalesStates::PortError:
				*strState = SysAllocString(TEXT("Ошибка открытия порта"));
				break;
			case ScalesStates::WrongSignal:
				*strState = SysAllocString(TEXT("Вероятно, требуется перезагрузка компьютера и весов"));
				break;
			default:
				*strState = SysAllocString(TEXT("Неизвестная ошибка"));
			}
			}
		} catch (...) {

		}
	}	
	return S_OK;
}

STDMETHODIMP CScales::SetZero()
{
	Log(TEXT("SetZero()"));
	return E_NOTIMPL;
}

STDMETHODIMP CScales::GetVersionInfo(BSTR* strInfo)
{
	CString a;
	a.Format(TEXT("GetVersionInfo()"));
	Log(a);
	*strInfo = SysAllocString(TEXT("Компонента NScales.Scales для получения веса с весового оборудования."));
	return S_OK;
}

STDMETHODIMP CScales::GetHelp(BSTR* strInfo)
{
	CString a;	
	a.Format(TEXT("GetHelp()"));
	Log(a);	
	*strInfo = SysAllocString(TEXT("Чтобы протестировать интерфейс, вызовите ConnectDevice(\"Тест\",\"\", 0, 1)"));
	return S_OK;
}

STDMETHODIMP CScales::ListSupportedDevices(BSTR* strInfo)
{
	//TCHAR* str = TEXT("Тест,БУ 4263 М1,Mettler Toledo IND310,CAS-6000,HBT-9,FT11,VT-220");
	CString str = ScalesSettings::getAllNames();
	CString a;
	a.Format(TEXT("ListSupported() = %s"), str.GetBuffer());
	Log(a);
	*strInfo = SysAllocString(str.GetBuffer());
	return S_OK;
}

STDMETHODIMP CScales::ListPorts(BSTR* strInfo)
{
	std::vector<CString> ports = GetPorts();
	CString sports;
	for (unsigned i = 0; i < ports.size(); i++) {
		sports.Append(ports[i]);
		if (i < ports.size() - 1) {
			sports.Append(TEXT(","));
		}
	}
	CString a;
	log->LogF(TEXT("ListPorts() = %s"), sports);	
	*strInfo = SysAllocString(sports);
	return S_OK;
}

std::vector<CString> CScales::GetPorts() {
	std::vector<CString> ports;
	//from MSDN
	HKEY hKey;
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys = 0;               // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 

	DWORD i, retCode;

	TCHAR	achValue[MAX_VALUE_NAME+1];
	DWORD	cchValue = MAX_VALUE_NAME;
	TCHAR	achData[MAX_VALUE_NAME+1];
	DWORD	cchData = MAX_VALUE_NAME;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,		
		TEXT("Hardware\\DeviceMap\\SerialComm"),
		0,
		KEY_READ,
		&hKey) == ERROR_SUCCESS
		)
	{
		retCode = RegQueryInfoKey(
			hKey,                    // key handle 
			achClass,                // buffer for class name 
			&cchClassName,           // size of class string 
			NULL,                    // reserved 
			&cSubKeys,               // number of subkeys 
			&cbMaxSubKey,            // longest subkey size 
			&cchMaxClass,            // longest class string 
			&cValues,                // number of values for this key 
			&cchMaxValue,            // longest value name 
			&cbMaxValueData,         // longest value data 
			&cbSecurityDescriptor,   // security descriptor 
			&ftLastWriteTime);       // last write time 
	}

	if (cValues > 255) cValues = 255;

	// Enumerate the key values. 
	log->LogF(TEXT("GetPorts() = %d"), cValues);
	if (cValues)
	{		
		for (i = 0, retCode = ERROR_SUCCESS; i<cValues; i++)
		{
			cchValue = MAX_VALUE_NAME;
			cchData = MAX_VALUE_NAME;
			achValue[0] = '\0';
			retCode = RegEnumValue(hKey, i,
				achValue,
				&cchValue,
				NULL,
				NULL,				
				(BYTE*)achData,
				&cchData);
			if (retCode == ERROR_SUCCESS) {
				ports.push_back(achData);
				//log->LogF(TEXT("GetPorts(%d): %s:%s"), i, achValue, achData);
								
			}
			else {
				//log->LogF(TEXT("GetPorts(%d): %s:%s"), i, achValue, achData);
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, retCode, 0, achData, cchData, NULL);
				log->LogF(TEXT("GetPorts(%d): error %d: %s"), i, retCode, achData);
			}
		}
	}

	RegCloseKey(hKey);

	return ports;
}

STDMETHODIMP CScales::IsOK(VARIANT_BOOL* OK) {
	if (!isInit()) {
		*OK = VARIANT_FALSE;
	}
	else if (history.empty()) {
		*OK = VARIANT_FALSE;
	}
	else {
		try {
			if (!(licenseState == LicenseState::Hardware || licenseState == LicenseState::Trial)) {
				*OK = VARIANT_FALSE;
			}
			else if ((GetTickCount() - lastTimeSuccess) < maxTimeSuccess) {
				*OK = VARIANT_TRUE;
			}
			else {
				*OK = VARIANT_FALSE;
			}

		}
		catch (...) {
			*OK = VARIANT_FALSE;
		}
	}		
	log->LogF(TEXT("IsOK() = %d"), (*OK == VARIANT_FALSE)?false:true);	
	return S_OK;
}

STDMETHODIMP CScales::ConnectIndicator(BSTR PortName, FLOAT multiplier)
{
	DisconnectIndicator();
	indicator = new Indicator(log);
	log->LogF(TEXT("ConnectIndicator(%s)"), PortName);
	if (!indicator->Connect(PortName)) return E_FAIL;
	echoMult = multiplier;
	return S_OK;
}

STDMETHODIMP CScales::DisconnectIndicator()
{
	if (indicator) {
		indicator->Disconnect();
		delete indicator;
		indicator = 0;
	}
	return S_OK;
}

STDMETHODIMP CScales::ConnectLogFile(BSTR Path)
{
	COLE2T tempstr(Path);
	log->LogF(TEXT("Starting log: %s"), (TCHAR*)tempstr);
	if (logWeight) {
		logWeight->Close();
		delete logWeight;
	}	
	logWeight = new NLog((TCHAR*)tempstr, true, false);
	logWeight->Start();	
	return S_OK;
}

DeviceType CScales::GetDeviceTypeByName(TCHAR* name) {
	return ScalesSettings::getByName(name).deviceType;
}

STDMETHODIMP CScales::GetHistoryLength(LONG* Length) {//get number of stored measurements	
	*Length = history.size();	
	return S_OK;
}

STDMETHODIMP CScales::GetHistoryTimeN(DWORD Index, DATE* DateTime) {//get measure time by index
	if (history.size() <= Index) {
		*DateTime = 0.0;
		return S_OK;
	}	
	*DateTime = history[Index].DateTime;	
	return S_OK;
}

STDMETHODIMP CScales::GetHistoryWeightN(DWORD Index, DOUBLE* Weight) {//get measurement by index
	if (history.size() <= Index) {
		*Weight = 0.0;
		return S_OK;
	}
	EnterCriticalSection(&criticalSection);
	*Weight = history[Index].weight;
	LeaveCriticalSection(&criticalSection);
	return S_OK;
}

STDMETHODIMP CScales::IsPortConnected(BSTR PortName, LONG BaudRate, VARIANT_BOOL * HasData)
{
	*HasData = VARIANT_FALSE;
	COMPort *port = new COMPort(log);	
	if (port->BytesAvailable(PortName, BaudRate)>0) *HasData = VARIANT_TRUE;
	delete port;
	return S_OK;
}

bool CScales::isInit() {
	return scalesPort || deviceType == DeviceType::Test;
}