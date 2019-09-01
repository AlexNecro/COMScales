// dllmain.h : Declaration of module class.

class CCOMScalesModule : public ATL::CAtlDllModuleT< CCOMScalesModule >
{
public :
	DECLARE_LIBID(LIBID_COMScalesLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_COMSCALES, "{E261DB02-CC6B-491C-9934-482546FAE399}")
};

extern class CCOMScalesModule _AtlModule;
