// ============================================================
// @Brief: Memory Classes
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_MEMORY_H__
#define __WIZ_MEMORY_H__

#include "Wiz/Base/Backward.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Memory{

using namespace Wiz::Base;

// ============================================================
class Buffer : public HandleT< HGLOBAL, NULL >{

	UINT _size;
	UINT _seek;

public:
	inline Buffer(){
		this->setSize( 0 );
		this->setSeek( 0 );
	}

	inline ~Buffer(){
		if( this->isCreated() )
			::GlobalFree( *this );
	}

	bool createBuffer( UINT size ){

		Reconstruct( this );

		this->setHandle( ::GlobalAlloc( GMEM_FIXED, size ) );

		if( !this->isCreated() )
			return false;

		this->setSize( size );
		this->setSeek( 0 );
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
		return _size;
	}

	inline UINT getSeek(){
		return _seek;
	}

	inline void* getSeekPos(){
		return (void*)( (PCHAR)this->getHandle() + this->getSeek() );
	}

	inline UINT getRemain(){

		if( this->getSize() < this->getSeek() )
			return 0;

		return( this->getSize() - this->getSeek() );
	}

	inline void setSize( UINT size ){
		_size = size;
	}

	inline void setSeek( UINT seek ){
		_seek = seek;
		Arrange( _seek, 0, this->getSize() );
	}

	inline bool shiftSeek( UINT offset ){

		if( this->getSeek() + offset > this->getSize() )
			return false;

		this->setSeek( this->getSeek() + offset );
		return true;
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
		return (Entry*)Buffer::getHandle();
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
