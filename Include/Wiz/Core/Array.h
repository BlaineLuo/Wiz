// ============================================================
// @Author: Blaine Luo
// @Date: 2017/03
// ============================================================
#ifndef __WIZ_CORE_ARRAY_H__
#define __WIZ_CORE_ARRAY_H__

#include "Wiz/Core/Basic.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Core{

#pragma pack( push, 1 )

// ============================================================
template< typename T, unsigned int MaxCount >
class ArrayNative : public StaticContainer< T, MaxCount >{

protected:
	Entry _entries[ _maxCount ];

public:
	void insert( unsigned int index, Entry& entry ){
		(*this)[index] = entry;
	}

	void remove( unsigned int index ){
		MemoryReset( (*this)[index] );
	}

	void clear(){
		MemoryReset( _entries );
	}

	//TODO: Fix conflict with operator[]
	operator Entry*(){
		return &(*this)[0];
	}

	template< typename Index >
	Entry& operator []( Index index ){
		return _entries[ (unsigned int)index % _maxCount ];
	}
};

// ============================================================
template< typename T, unsigned int MaxCount >
class ArrayIterator{

public:
	typedef T Entry;
	enum{ _maxCount = MaxCount };
	typedef ArrayNative< Entry, _maxCount > Array;

protected:
	Array& _array;
	typename Array::Indexer _indexer;

public:
	ArrayIterator( Array& arrayN, unsigned int index = 0 ) :
		_array( arrayN )
	{
		_indexer = index;
	}

	inline operator Entry&(){
		return _array[ _indexer ];
	}

	inline ArrayIterator& operator ++(){
		++_indexer;
		return *this;
	}

	inline ArrayIterator& operator --(){
		--_indexer;
		return *this;
	}
};

// ============================================================
template< typename T, unsigned int MaxCount >
class Array : public ArrayNative< T, MaxCount >{

public:
	typedef ArrayIterator< Entry, _maxCount > Iterator;

	Array(){
		this->clear();
	}
};

// ============================================================
template< typename T, unsigned int MaxCount >
class LimitedContainer : public DynamicContainer, public Array< T, MaxCount >{

public:
	bool isFull(){
		return( this->getCurCount() >= _maxCount );
	}

	bool incCurCount(){
		if( this->isFull() )
			return false;
		return this->DynamicContainer::incCurCount();
	}
};

// ============================================================
template< typename T, unsigned int MaxCount >
class Pool : public LimitedContainer< T, MaxCount >{

protected:
	Array< bool, _maxCount > _isAcquiredSet;

public:
	Entry* acquire( unsigned int* idx = NULL ){

		if( this->isFull() )
			return NULL;

		for( unsigned int i = 0; i < _maxCount; i++ ){
			bool& isAcquired = _isAcquiredSet[ i ];
			if( isAcquired )
				continue;

			if( NULL != idx )
				*idx = i;

			isAcquired = true;
			this->incCurCount();
			return &(*this)[ i ];
		}
		return NULL;
	}

	bool release( unsigned int idx ){

		if( this->isEmpty() )
			return false;

		bool& isAcquired = _isAcquiredSet[ idx ];
		if( !isAcquired )
			return false;

		Reconstruct( &(*this)[ idx ] );
		isAcquired = false;
		this->decCurCount();
		return true;
	}

	inline Entry* fetch( unsigned int idx ){
		if( !_isAcquiredSet[ idx ] )
			return NULL;
		return &(*this)[ idx ];
	}
};

#pragma pack( pop )

}}
// ===================================Namespace Tail==========================================

#endif
