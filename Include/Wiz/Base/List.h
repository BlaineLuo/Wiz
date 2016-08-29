// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_BASE_LIST_H__
#define __WIZ_BASE_LIST_H__

#include "Wiz/Base/Forward.h"

// ============================================================
// Forward Declaration
// ============================================================
namespace Wiz{
	namespace Base{
		template< typename T > class ListNode;
		template< typename T > class ListIterator;
		template< typename T > class List;
	}
}

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Base{

// ============================================================
template< typename T >
class ListNode{

public:
	typedef	T Entry;

	Entry _entry;
	ListNode* _prev;
	ListNode* _next;

	ListNode( Entry* entry, ListNode* prev, ListNode* next ){
		_entry = *entry;
		_prev = prev;
		_next = next;

		if( _prev )
			_prev->_next = this;
		if( _next )
			_next->_prev = this;
	}

	~ListNode(){
		if( _prev )
			_prev->_next = _next;
		if( _next )
			_next->_prev = _prev;
	}
};

// ============================================================
template< typename T >
class ListIterator{

public:
	typedef	T Entry;
	typedef	ListNode< Entry > Node;

private:
	Node* _node;
	int _index;

public:
	ListIterator( List< Entry >& list, int index = 0 ){
		_node = list[ index ];
		_index = index;
	}

	Entry* getCurEntry(){
		return( _node ? &_node->_entry : NULL );
	}

	Entry* getNextEntry(){
		if( _node ){
			_node = _node->_next;
			_index++;
		}
		return this->getCurEntry();
	}

	Entry* getPrevEntry(){
		if( _node ){
			_node = _node->_prev;
			_index--;
		}
		return this->getCurEntry();
	}

	int getIndex(){
		return _index;
	}

	Entry* operator ->(){
		return this->getCurEntry();
	}

	bool operator ==( Entry* entry ){
		return( this->getCurEntry() == entry );
	}

	bool operator !=( Entry* entry ){
		return( this->getCurEntry() != entry );
	}

	Entry* operator ++(){
		return this->getNextEntry();
	}

	Entry* operator --(){
		return this->getPrevEntry();
	}
};

// ============================================================
template< typename T >
class List : public DynamicContainer{

public:
	typedef	T Entry;
	typedef	ListNode< Entry > Node;
	typedef	ListIterator< Entry > Iterator;

private:
	Node* _head;
	Node* _tail;

public:
	List(){
		_head = NULL;
		_tail = NULL;
	}

	~List(){
		this->removeNodeAll();
	}

	Node* operator []( int index ){
		return this->findNodeAt( index );
	}

	Node* findNodeAt( int index ){

		if( this->isEmpty() )
			return NULL;

		Node* node = NULL;

		if( index >= 0 ){

			node = _head;

			for( int i = 0; i < index; i++ ){

				node = node->_next;
				if( node == NULL )
					return NULL;
			}

		}else{

			node = _tail;

			for( int i = -1; i > index; i-- ){

				node = node->_prev;
				if( node == NULL )
					return NULL;
			}

		}
		return node;
	}

	Entry* findEntryAt( int index ){

		Node* node = this->findNodeAt( index );
		if( node == NULL )
			return NULL;

		return &node->_entry;
	}

	bool insertNodeAfter( int index, Entry* entry ){

		if( this->isEmpty() && ( index == 0 || index == -1 ) ){

			_head = _tail = new Node( entry, NULL, NULL );

		}else{

			Node* node = this->findNodeAt( index );
			if( node == NULL )
				return false;

			new Node( entry, node, node->_next );

			if( _tail == node )
				_tail = _tail->_next;
		}

		this->incCurCount();
		return true;
	}

	bool insertNodeAfter( int index, Entry entry ){
		return this->insertNodeAfter( index, &entry );
	}

	bool insertNodeBefore( int index, Entry* entry ){

		if( this->isEmpty() && ( index == 0 || index == -1 ) ){

			_head = _tail = new Node( entry, NULL, NULL );

		}else{

			Node* node = this->findNodeAt( index );
			if( node == NULL )
				return false;

			new Node( entry, node->_prev, node );

			if( _head == node )
				_head = _head->_prev;
		}

		this->incCurCount();
		return true;
	}

	bool insertNodeBefore( int index, Entry entry ){
		return this->insertNodeBefore( index, &entry );
	}

	bool insertNodeAtHead( Entry* entry ){
		return this->insertNodeBefore( 0, entry );
	}

	bool insertNodeAtHead( Entry entry ){
		return this->insertNodeAtHead( &entry );
	}

	bool insertNodeAtTail( Entry* entry ){
		return this->insertNodeAfter( -1, entry );
	}

	bool insertNodeAtTail( Entry entry ){
		return this->insertNodeAtTail( &entry );
	}

	bool insertNodeAt( int index, Entry* entry ){

		Node* node = this->findNodeAt( index );
		if( node != NULL ){

			if( index >= 0 )
				return this->insertNodeBefore( index, entry );
			else
				return this->insertNodeAfter( index, entry );

		}else{

			if( index >= 0 )
				return this->insertNodeAtTail( entry );
			else
				return this->insertNodeAtHead( entry );
		}
		return false;
	}

	bool removeNodeAt( int index ){

		if( this->isEmpty() )
			return false;

		Node* node = this->findNodeAt( index );
		if( node == NULL )
			return false;

		if( _head == node )
			_head = _head->_next;

		if( _tail == node )
			_tail = _tail->_prev;

		delete node;

		this->decCurCount();
		return true;
	}

	bool removeNodeAtHead(){
		return this->removeNodeAt( 0 );
	}

	bool removeNodeAtTail(){
		return this->removeNodeAt( -1 );
	}

	bool removeNodeAll(){
		while( this->removeNodeAt( 0 ) );
		return true;
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
