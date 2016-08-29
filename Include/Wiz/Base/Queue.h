// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_BASE_QUEUE_H__
#define __WIZ_BASE_QUEUE_H__

#include "Wiz/Base/Forward.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Base{

// ============================================================
template< typename T, unsigned int MaxCount >
class CircularQueue : public LimitedContainer< T, MaxCount >{

protected:
	Indexer _indexPush;
	Indexer _indexPop;

public:
	Entry* seek( unsigned int offset ){
		return &(*this)[ _indexPush + offset ];
	}

	bool seek( unsigned int offset, Entry* entry ){

		Entry* e = this->seek( offset );
		if( NULL == e )
			return false;

		*entry = *e;
		return true;
	}

	Entry* peek( unsigned int offset ){

		if( this->isEmpty() || offset >= this->getCurCount() )
			return NULL;

		return &(*this)[ _indexPop + offset ];
	}

	bool peek( unsigned int offset, Entry* entry ){

		Entry* e = this->peek( offset );
		if( NULL == e )
			return false;

		*entry = *e;
		return true;
	}

	bool pop( Entry* entry = NULL ){

		if( this->isEmpty() )
			return false;

		if( NULL != entry )
			*entry = (*this)[ _indexPop ];

		++_indexPop;
		this->decCurCount();
		return true;
	}

	bool push( Entry& entry ){

		(*this)[ _indexPush ] = entry;

		++_indexPush;

		if( this->isFull() )
			++_indexPop;
		else
			this->incCurCount();

		return true;
	}

	bool clear(){
		while( this->getCurCount() > 0 )
			this->pop();
		return true;
	}
};

// ============================================================
template< typename T, unsigned int MaxCount >
class OriginalQueue : public CircularQueue< T, MaxCount >{
public:
	typedef CircularQueue Parent;

	bool push( Entry& entry ){
		if( this->isFull() )
			return false;
		return this->Parent::push( entry );
	}
};

// ============================================================
template< typename T, unsigned int MaxCount >
class PipelineQueue : public OriginalQueue< T, MaxCount >{

public:
	typedef OriginalQueue Parent;
	typedef Array< typename Indexer, 8 > PipeSet;

protected:
	PipeSet _pipeSet;

	bool checkPipe( unsigned int pipeId ){
		Indexer& current = _pipeSet[ pipeId ];
		Indexer* previous = NULL;
		if( 0 == pipeId )
			previous = &_indexPush;
		else
			previous = &_pipeSet[ PipeSet::Indexer( pipeId )-- ];
		return( current != *previous );
	}

public:
	Entry* peek( unsigned int pipeId ){

		if( !this->checkPipe( pipeId ) )
			return false;

		return &(*this)[ _pipeSet[ pipeId ] ];
	}

	bool done( unsigned int pipeId ){

		if( !this->checkPipe( pipeId ) )
			return false;

		_pipeSet[ pipeId ]++;
		return true;
	}

	bool pop(){
		if( this->isEmpty() )
			return false;

		for( int i = 0; i < _pipeSet._maxCount; i++ ){
			Indexer& x = _pipeSet[i];
			if( x == _indexPop )
				x++;
		}
		return this->Parent::pop();
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
