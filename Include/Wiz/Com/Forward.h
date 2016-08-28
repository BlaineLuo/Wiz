// ============================================================
// @Brief	Windows COM Classes 
// @Author	Blaine
// @Date	2014/03/24
// ============================================================
#ifndef __WIZ_COM_FORWARD_H__
#define __WIZ_COM_FORWARD_H__

#include "Wiz/Base/Backward.h"
#include <ObjBase.h>
#include <ShlGuid.h>
#include <ShObjIdl.h>

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Com{

using namespace Wiz::Base;

// ============================================================
class Startup{
	DEFINE_SINGLE_EX( Startup,
		::CoInitializeEx( NULL, COINIT_MULTITHREADED );
	,
		::CoUninitialize();
	);
};

// ============================================================
// @Brief	COM Object Base
// ============================================================
template< typename T >
class ComObject : public HandleT< T*, NULL >{

public:
	inline ~ComObject(){
		if( this->isCreated() )
			(*this)->Release();
	}

	inline ComObject& operator =( T* obj ){
		return this->setHandle( obj );
	}

	inline T* operator ->(){
		return this->getHandle();
	}

	inline HRESULT createComObject(
		REFCLSID refClassId,
		REFIID refInterfaceId,
		LPUNKNOWN unknownOuter = NULL,
		DWORD classContext = CLSCTX_INPROC_SERVER
	){
		Reconstruct( this );
		return ::CoCreateInstance(
			refClassId,
			unknownOuter,
			classContext,
			refInterfaceId,
			(LPVOID*)this->getHandleAddress()
		);
	}

	inline HRESULT setProxyBlanket(
		DWORD authnSvc,
		DWORD authzSvc,
		DWORD authnLevel,
		DWORD impLevel,
		DWORD capabilities = EOAC_NONE,
		OLECHAR* serverPrincName = NULL,
		RPC_AUTH_IDENTITY_HANDLE authInfo = NULL
	){
		return ::CoSetProxyBlanket(
			*this,
			authnSvc,
			authzSvc,
			serverPrincName,
			authnLevel,
			impLevel,
			authInfo,
			capabilities
		);
	}

	//inline HRESULT queryInterface( REFIID refInterfaceId, ComObject& obj ){
	//	return (*this)->QueryInterface( refInterfaceId, obj.getHandleAddress() );
	//}
};

// ============================================================
class Stream : public ComObject< IStream >{
public:
	inline HRESULT createStreamOnHGlobal( HGLOBAL handleGlobal ){
		return ::CreateStreamOnHGlobal( handleGlobal, false, this->getHandleAddress() );
	}

	inline QWORD getSeekOffset(){
		LARGE_INTEGER largeIntOffset = {0};
		ULARGE_INTEGER largeIntCurrent = {0};

		if( S_OK != (*this)->Seek( largeIntOffset, STREAM_SEEK_CUR, &largeIntCurrent ) )
			return 0;
		return largeIntCurrent.QuadPart;
	}

	inline HRESULT setSeekOffset( QWORD offset ){
		LARGE_INTEGER largeInt = {0};
		largeInt.QuadPart = offset;
		return (*this)->Seek( largeInt, STREAM_SEEK_SET, NULL );
	}
};

// ============================================================
#ifndef _WIN32_WCE
class ShellLink : public ComObject< IShellLink >{
public:
	inline HRESULT createComObject(){
		return ComObject::createComObject( CLSID_ShellLink, IID_IShellLink );
	}
};
#endif

// ============================================================
class PersistFile : public ComObject< IPersistFile >{};

}}
// ===================================Namespace Tail==========================================

#endif
