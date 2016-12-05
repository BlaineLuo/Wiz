// ============================================================
// @Author: Blaine Luo
// @Date: 2016/12
// ============================================================
#ifndef __WIZ_SYSTEM_SNAPSHOT_H__
#define __WIZ_SYSTEM_SNAPSHOT_H__

#include "Wiz/System/Forward.h"
#include <TLHelp32.h>

// ===================================Namespace Head==========================================
namespace Wiz{ namespace System{

// ============================================================
template< typename Ftor, DWORD Flags >
class SnapObj : public HandleT< HANDLE, INVALID_HANDLE_VALUE >{

protected:
	BOOL _hasNext;

public:
	typedef typename Ftor::Entry Entry;

	inline SnapObj(){
		_hasNext = false;
	}

	inline ~SnapObj(){
		if( this->isCreated() )
			::CloseHandle( *this );
	}

	inline bool create( DWORD processId = 0 ){
		Reconstruct( this );
		this->setHandle( ::CreateToolhelp32Snapshot( Flags, processId ) );
		return this->isCreated();
	}

	bool next( Entry& entry ){
		entry.dwSize = sizeof(Entry);
		if( _hasNext )
			return Ftor::Next( *this, entry );

		_hasNext = true;
		return Ftor::First( *this, entry );
	}
};

// ============================================================
template< typename T >
struct SnapFtor{
	typedef T Entry;
};

struct SnapProcessFtor : SnapFtor< PROCESSENTRY32 >{
	inline static bool First( HANDLE snapObj, Entry& entry ){
		return( 0 != ::Process32First( snapObj, &entry ) );
	}
	inline static bool Next( HANDLE snapObj, Entry& entry ){
		return( 0 != ::Process32Next( snapObj, &entry ) );
	}
};

struct SnapThreadFtor : SnapFtor< THREADENTRY32 >{
	inline static bool First( HANDLE snapObj, Entry& entry ){
		return( 0 != ::Thread32First( snapObj, &entry ) );
	}
	inline static bool Next( HANDLE snapObj, Entry& entry ){
		return( 0 != ::Thread32Next( snapObj, &entry ) );
	}
};

struct SnapModuleFtor : SnapFtor< MODULEENTRY32 >{
	inline static bool First( HANDLE h, Entry& entry ){
		return( 0 != ::Module32First( h, &entry ) );
	}
	inline static bool Next( HANDLE h, Entry& entry ){
		return( 0 != ::Module32Next( h, &entry ) );
	}
};

typedef SnapObj< SnapProcessFtor, TH32CS_SNAPPROCESS > SnapshotProcess;
typedef SnapObj< SnapThreadFtor, TH32CS_SNAPTHREAD > SnapshotThread;
typedef SnapObj< SnapModuleFtor, TH32CS_SNAPMODULE > SnapshotModule;

}}
// ===================================Namespace Tail==========================================

#endif
