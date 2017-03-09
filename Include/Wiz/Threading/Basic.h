// ============================================================
// @Brief: Thread Classes
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_THREADING_BASIC_H__
#define __WIZ_THREADING_BASIC_H__

#include "Wiz/Core/All.h"
#include "Wiz/Memory.h"

// ============================================================
// Forward Declaration
// ============================================================
namespace Wiz{ namespace Threading{
	template< typename T > class SafeListIterator;
	template< typename T > class SafeList;
	template< typename K, typename E > class SafeListMap;
}}

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Threading{

using namespace Wiz::Core;
using namespace Wiz::Memory;

// ============================================================
// @Brief: Base Class for Synchronization Objects
// ============================================================
class SyncObject : public HandleT< HANDLE, NULL >{
public:
	inline ~SyncObject(){
		if( this->isCreated() )
			this->closeHandle();
	}

	inline bool isSuccess( DWORD lastError = ::GetLastError() ){
		return( ERROR_ACCESS_DENIED != lastError && ERROR_ALREADY_EXISTS != lastError );
	}

	virtual bool signal() = 0;
};

// ============================================================
// @Brief: Event Object ( Inter-Process )
// ============================================================
class Event : public SyncObject{
public:
	bool create( PTCHAR objName = NULL, BOOL isManualReset = false, BOOL isInitialState = false ){
		Reconstruct( this );

		this->setHandle( ::CreateEvent( NULL, isManualReset, isInitialState, objName ) );
		if( !this->isSuccess() )
			Reconstruct( this );

		return this->isCreated();
	}

	virtual bool signal(){
		return( 0 != ::SetEvent( *this ) );
	}
};

// ============================================================
// @Brief: Mutex Object ( Inter-Process )
// ============================================================
class Mutex : public SyncObject{
public:
	bool create( PTCHAR objName = NULL, BOOL isInitialOwner = false ){
		Reconstruct( this );

		this->setHandle( ::CreateMutex( NULL, isInitialOwner, objName ) );
		if( !this->isSuccess() )
			Reconstruct( this );

		return this->isCreated();
	}

	virtual bool signal(){
		return( 0 != ::ReleaseMutex( *this ) );
	}
};

// ============================================================
// @Brief: Semaphore Object ( Inter-Process )
// ============================================================
class Semaphore : public SyncObject{
public:
	bool create( PTCHAR objName = NULL, DWORD maxCount = 1, DWORD iniCount = 0 ){
		Reconstruct( this );

		this->setHandle( ::CreateSemaphore( NULL, iniCount, maxCount, objName ) );
		if( !this->isSuccess() )
			Reconstruct( this );

		return this->isCreated();
	}

	// Note: If ( Count + Current Count ) > MaxCount of Semaphore, Signal will fial.
	virtual bool signal(){
		return( 0 != ::ReleaseSemaphore( *this, 1, NULL ) );
	}
};

// ============================================================
// @Brief: CriticalSection Object ( non Inter-Process )
// ============================================================
class CriticalSection : public Structure< CRITICAL_SECTION >{
public:
	inline CriticalSection(){
		::InitializeCriticalSection( &(*this) );
	}

	inline ~CriticalSection(){
		::DeleteCriticalSection( &(*this) );
	}

	inline bool tryEnter(){
		return( 0 != ::TryEnterCriticalSection( &(*this) ) );
	}

	inline void enter(){
		::EnterCriticalSection( &(*this) );
	}

	inline void leave(){
		::LeaveCriticalSection( &(*this) );
	}
};

// ============================================================
class Thread : public SyncObject{

	template< typename Object = void, typename Method = void* >
	struct Context{
		void* _function;
		Object* _object;
		Method _method;
		ParameterSet _paramSet;
	};

	Buffer _buffer;

public:
	static DWORD WINAPI Procedure( Context<>* context ){
		typedef DWORD(__cdecl *PtrFunction)( ... );
		if( NULL == context )
			return 0;

		PtrFunction function = (PtrFunction)context->_function;
		return function( (ParameterSet)context->_paramSet );
	}

	template< typename Object, typename Method >
	static DWORD WINAPI ProcedureEx( Context< Object, Method >* context ){
		typedef DWORD(__cdecl Object::*PtrMethod)( ... );
		if( NULL == context )
			return 0;

		Object* object = context->_object;
		PtrMethod method = (PtrMethod)context->_method;
		return ( object->*method )( (ParameterSet)context->_paramSet );
	}

	bool create( void* function, ... ){
		typedef Context<> Ctx;
		va_list	args;
		va_start( args, function );

		_buffer.createBuffer( sizeof(Ctx) );
		Ctx* context = (Ctx*)_buffer.getHandle();
		context->_function = function;
		context->_paramSet.set( args );

		this->setHandle( ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)Procedure, context, 0, NULL ) );
		return this->isCreated();
	}

	template< typename Object, typename Method >
	bool createEx( Object* object, Method method, ... ){
		typedef Context< Object, Method > Ctx;
		va_list	args;
		va_start( args, method );

		_buffer.createBuffer( sizeof(Ctx) );
		Ctx* context = (Ctx*)_buffer.getHandle();
		context->_object = object;
		context->_method = method;
		context->_paramSet.set( args );

		this->setHandle( ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ProcedureEx< Object, Method >, context, 0, NULL ) );
		return this->isCreated();
	}

	virtual bool signal(){
		return false;
	}
};

// ============================================================
template< typename Object, typename SyncObj >
class Trigger{

	typedef DWORD(__cdecl Object::*PtrMethod)( ... );
	Object* _object;
	PtrMethod _method;

	SyncObj _syncObj;
	Thread _thread;
	UINT _intervalMilliseconds;

	void __cdecl process(){
		while( 1 ){
			DWORD ret = ::WaitForMultipleObjectsEx( 1, _syncObj.getHandleAddress(), true, _intervalMilliseconds, false );
			( _object->*_method )();
		}// ========End of Thread Loop.========
	}

public:
	template< typename Method >
	void create( Object* object, Method method, UINT intervalMilliseconds = INFINITE ){
		_object = object;
		_method = (PtrMethod)method;
		_intervalMilliseconds = intervalMilliseconds;
		_syncObj.create();
		_thread.createEx( this, &Trigger::process );
	}

	bool trigger(){
		return _syncObj.signal();
	}
};

// ============================================================
template< typename T, UINT MaxCount >
class SafeCircularQueue : public CircularQueue< T, MaxCount >{

	CriticalSection _criticalSection;

public:
	typedef CircularQueue Parent;

	bool push( Entry& entry ){
		_criticalSection.enter();
		bool ret = this->Parent::push( entry );
		_criticalSection.leave();
		return ret;
	}

	bool pop( Entry* entry = NULL ){
		_criticalSection.enter();
		bool ret = this->Parent::pop( entry );
		_criticalSection.leave();
		return ret;
	}

	bool clear(){
		_criticalSection.enter();
		bool ret = this->Parent::clear();
		_criticalSection.leave();
		return ret;
	}
};

// ============================================================
template< typename T, UINT MaxCount >
class SafeOriginalQueue : public SafeCircularQueue< T, MaxCount >{
public:
	typedef SafeCircularQueue Parent;

	bool push( Entry& entry ){
		if( this->isFull() )
			return false;
		return this->Parent::push( entry );
	}
};

// ============================================================
template< typename T, UINT MaxCount >
class AsyncQueue : public SafeOriginalQueue< T, MaxCount >{

	Semaphore _semaphore;

public:
	typedef SafeOriginalQueue Parent;

	AsyncQueue(){
		_semaphore.create( NULL, MaxCount );
	}

	bool push( Entry& entry ){
		if( !this->Parent::push( entry ) )
			return false;
		return _semaphore.signal();
	}

	bool pop( Entry* entry, DWORD milliseconds = INFINITE ){
		if( ::WaitForMultipleObjects( 1, _semaphore.getHandleAddress(), false, milliseconds ) != WAIT_OBJECT_0 )
			return false;
		return this->Parent::pop( entry );
	}

	bool clear(){
		Entry entry;
		while( this->getCurCount() > 0 )
			this->pop( &entry, 0 );
		return true;
	}
};

// ============================================================
template< typename T >
class SafeListIterator : public ListIterator< T >{

	CriticalSection* _criticalSection;

public:
	SafeListIterator( SafeList< T >& list, int index = 0 ) :
		ListIterator( list, index )
	{
		_criticalSection = &list._criticalSection;
		_criticalSection->enter();
	}

	~SafeListIterator(){
		_criticalSection->leave();
	}
};

// ============================================================
template< typename T >
class SafeList : public List< T >{

	friend class SafeListIterator< T >;
	CriticalSection _criticalSection;

public:
	typedef SafeListIterator< T > Iterator;
};

// ============================================================
template< typename K, typename E >
class SafeListMap : public SafeList< MapNode< K, E > >{

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

}}
// ===================================Namespace Tail==========================================

#endif
