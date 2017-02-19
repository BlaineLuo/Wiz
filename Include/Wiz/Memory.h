// ============================================================
// @Brief: Memory Classes
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_MEMORY_H__
#define __WIZ_MEMORY_H__

#include "Wiz/Core/Handle.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Memory{

using namespace Wiz::Core;

// ============================================================
class Seeker{

public:
	typedef unsigned char* Pointer;

protected:
	Pointer _head;
	Pointer _seek;
	unsigned int _size;
	unsigned int _offset;

public:
	inline Seeker( void* head = NULL, unsigned int size = 0 ){
		this->create( head, size );
	}

	inline Seeker& create( void* head, unsigned int size ){
		_head = (Pointer)head;
		_size = size;
		return this->clear().resetSeek();
	}

	inline Pointer getHead( unsigned int offset = 0 ){
		return _head + offset;
	}

	inline Pointer getSeek( unsigned int offset = 0 ){
		return _seek + offset;
	}

	inline unsigned int getSize(){
		return _size;
	}

	inline unsigned int getOffset(){
		return _offset;
	}

	inline unsigned int getRemain(){
		int remain = this->getSize() - this->getOffset();
		return Arrange( remain, 0, this->getSize() );
	}

	inline Seeker& moveSeek( int offset ){
		return this->resetSeek( this->getOffset() + offset );
	}

	inline Seeker& resetSeek( int offset = 0 ){
		_seek = this->getHead( _offset = Arrange( offset, 0, this->getSize() ) );
		return *this;
	}

	inline Seeker& clear(){
		if( NULL != this->getHead() )
			::ZeroMemory( this->getHead(), this->getSize() );
		return *this;
	}
};

// ============================================================
class Buffer : public HandleT< HGLOBAL, NULL >{

public:
	inline ~Buffer(){
		if( this->isCreated() )
			::GlobalFree( *this );
	}

	bool createBuffer( UINT size ){
		Reconstruct( this );
		this->setHandle( ::GlobalAlloc( GMEM_FIXED, size ) );
		if( !this->isCreated() )
			return false;

		this->clear();
		return true;
	}

	inline bool copyFrom( void* data, UINT size, UINT offset = 0 ){

		if( !this->isCreated() || data == NULL || this->getSize() < ( size + offset ) )
			return false;

		::CopyMemory( (PUCHAR)this->getHandle() + offset, data, size );
		return true;
	}

	inline bool copyTo( void* data, UINT size, UINT offset = 0 ){

		if( !this->isCreated() || data == NULL || this->getSize() < ( size + offset ) )
			return false;

		::CopyMemory( data, (PUCHAR)this->getHandle() + offset, size );
		return true;
	}

	inline bool clear(){

		if( !this->isCreated() )
			return false;

		::ZeroMemory( *this, this->getSize() );
		return true;
	}

	inline UINT getSize(){
		return ::GlobalSize( *this );
	}
};

// ============================================================
// @Brief: Buffer Template
// ============================================================
template< typename T >
class BufferT : protected Buffer{

public:
	typedef T Entry;
	using Buffer::createBuffer;

	inline bool create(){
		return this->createBuffer( sizeof(Entry) );
	}

	inline Entry* getHandle(){
		return (Entry*)this->Buffer::getHandle();
	}
};

// ============================================================
class ParameterSet : public Array< BYTE, 128 >{

	// Because the max Parameter number is 32, and most Parameter size is 4 byte.
public:
	inline void set( void* args ){
		::CopyMemory( this, args, _totalSize );
	}

	inline void set( int dummy, ... ){
		va_list	args;
		va_start( args, dummy );
		::CopyMemory( this, args, _totalSize );
	}
	//Note: dummy is only a start for the variable argument list, so it can be any value.
};

}}
// ===================================Namespace Tail==========================================

#endif
