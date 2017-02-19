// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_COM_WMI_H__
#define __WIZ_COM_WMI_H__

#ifndef _WIN32_WCE

#include "Wiz/Com/Forward.h"
#include <WbemIdl.h>
#pragma comment( lib, "WbemUuid.lib" )

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Com{ namespace Wmi{

// ============================================================
class ClassObject : public ComObject< IWbemClassObject >{};

// ============================================================
class Enumerator : public ComObject< IEnumWbemClassObject >{
public:
	HRESULT next(
		ClassObject& classObj,
		PULONG returned,
		ULONG count = 1,
		LONG timeout = WBEM_INFINITE
	){
		return (*this)->Next( timeout, count, classObj.getHandleAddress(), returned );
	}
};

// ============================================================
class Service : public ComObject< IWbemServices >{
public:
	inline HRESULT execQuery(
		Enumerator& enumerator,
		OLECHAR* queryLanguage,
		OLECHAR* queryString,
		LONG flags = WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		IWbemContext* context = NULL
	){
		return (*this)->ExecQuery( queryLanguage, queryString, flags, context, enumerator.getHandleAddress() );
	}
};

// ============================================================
class Locator : public ComObject< IWbemLocator >{
public:
	inline HRESULT createComObject(){
		return ComObject::createComObject( CLSID_WbemLocator, IID_IWbemLocator );
	}

	inline HRESULT connectServer( Service& service ){
		return (*this)->ConnectServer(
			L"ROOT\\CIMV2",
			NULL,
			NULL,
			NULL,
			0,
			NULL,
			NULL,
			service.getHandleAddress()
		);
	}
};

}}}
// ===================================Namespace Tail==========================================

#endif

#endif
