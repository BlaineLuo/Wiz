// ============================================================
// @Author: Blaine Luo
// @Date: 2017/03
// ============================================================
#ifndef __WIZ_CORE_HANDLE_H__
#define __WIZ_CORE_HANDLE_H__

#include "Wiz/Core/Basic.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Core{

// ============================================================
// @Brief: Handle Template
// ============================================================
template< typename T, T NullHandle >
class HandleT{

public:
	typedef T Handle;

private:
	Handle _handle;

public:
	static inline Handle GetNullHandle(){
		return NullHandle;
	}

	inline HandleT(){
		this->setHandle( this->GetNullHandle() );
	}

	inline ~HandleT(){
		this->setHandle( this->GetNullHandle() );
	}

	inline operator Handle&(){
		return this->getHandle();
	}

	inline Handle& getHandle(){
		return _handle;
	}

	inline Handle* getHandleAddress(){
		return &_handle;
	}

	inline HandleT& setHandle( Handle v ){
		_handle = v;
		return *this;
	}

	inline bool isCreated(){
		return( *this != this->GetNullHandle() );
	}

	inline bool closeHandle(){
		return( 0 != ::CloseHandle( *this ) );
	}
};

// ============================================================
// @Brief: Handle Pointer Template
// @Note: This Pointer Template only handle
//        Pointer once, it has no reference count.
// ============================================================
template< typename T >
class HandleP : public HandleT< T*, NULL >{

public:
	inline HandleP( Handle v = HandleP::GetNullHandle() ){
		this->setHandle( v );
	}

	inline ~HandleP(){
		if( this->isCreated() )
			delete this->getHandle();
	}

	inline HandleP& operator =( Handle v ){
		this->setHandle( v );
		return *this;
	}

	inline Handle operator ->(){
		return this->getHandle();
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
