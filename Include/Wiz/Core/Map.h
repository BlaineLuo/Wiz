// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_CORE_MAP_H__
#define __WIZ_CORE_MAP_H__

#include "Wiz/Core/Forward.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Core{

// ============================================================
template< typename K, typename E >
struct MapNode{
	typedef K Key;
	typedef E Entry;
	Key _key;
	Entry _entry;
	MapNode(): _key(), _entry(){};
};

// ============================================================
template< typename K, K NullKey, typename E, UINT MaxCount >
class ArrayMap : public Array< MapNode< K, E >, MaxCount >{

public:
	typedef	K Key;
	typedef	E Entry;
	typedef	MapNode< Key, Entry > Node;
	enum{ _nullKey = (int)NullKey };

	bool insertR( Key key, Entry& entry ){
		Node* node = this->findForce( _nullKey );
		if( node == NULL )
			return false;
		node->_key = key;
		node->_entry = entry;
		return true;
	}

	bool insertV( Key key, Entry entry ){
		return this->insertR( key, entry );
	}

	bool remove( Key key ){
		Node* node = this->findNode( key );
		if( node == NULL )
			return false;
		MemoryReset( *node );
		return true;
	}

	Entry* findEntry( Key key ){
		Node* node = this->findNode( key );
		if( node == NULL )
			return NULL;
		return &node->_entry;
	}

	Node* findNode( Key key ){
		if( (int)key == _nullKey )
			return NULL;
		return this->findForce( key );
	}

protected:
	Node* findForce( Key key ){
		for( int i = 0; i < _maxCount; i++ ){
			Node& node = (*this)[i];
			if( node._key == key )
				return &node;
		}
		return NULL;
	}
};

// ============================================================
template< typename K, typename E >
class ListMap : public List< MapNode< K, E > >{

public:
	typedef	K Key;
	typedef	E Entry;
	typedef	MapNode< Key, Entry > Node;

	Node* findNodeAt( int index ){
		return this->List::findEntryAt( index );
	}

	Entry* findEntryAt( int index ){
		Node* node = this->List::findEntryAt( index );
		return node ? &node->_entry : NULL;
	}

	Entry* findEntryR( Key& key ){
		for( Iterator iter( *this ); iter != NULL; ++iter )
			if( iter->_key == key )
				return &iter->_entry;
		return NULL;
	}

	Entry* findEntryV( Key key ){
		return this->findEntryR( key );
	}

	bool insertMapNode( Key& key, Entry& entry ){

		if( this->findEntryR( key ) != NULL )
			return false;

		Node node;
		node._key = key;
		node._entry = entry;
		return this->insertNodeAtTail( &node );
	}
};

// ============================================================
template< typename K, K NullKey, typename E, UINT MaxCount >
class PoolMap : public Pool< MapNode< K, E >, MaxCount >{

public:
	typedef	K Key;
	typedef	E Entry;
	typedef	MapNode< Key, Entry > Node;
	enum{ _nullKey = (int)NullKey };

	bool insertR( Key key, Entry& entry ){
		Node* node = this->acquire();
		if( node == NULL )
			return false;
		node->_key = key;
		node->_entry = entry;
		return true;
	}

	bool insertV( Key key, Entry entry ){
		return this->insertR( key, entry );
	}

	bool remove( Key key ){
		Node* node = this->find( key );
		if( node == NULL )
			return false;
		return this->release( node );
	}

	Node* find( Key key ){

		if( this->isEmpty() || (int)key == _nullKey )
			return NULL;

		for( UINT i = 0, findCount = 0; ( i < _maxCount ) & ( findCount < this->getCurCount() ); i++ ){
			Pool::Node& poolNode = (*this)[i];
			if( !poolNode._isAcquired )
				continue;

			findCount++;
			Node& node = poolNode._entry;
			if( node._key != key )
				continue;

			return &node;
		}
		return NULL;
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
