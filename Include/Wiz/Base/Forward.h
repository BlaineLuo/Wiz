// ============================================================
// @Author	Blaine
// @Date	2015/03/01
// ============================================================
#ifndef __WIZ_BASE_FORWARD_H__
#define __WIZ_BASE_FORWARD_H__

#include <New>
#include <List>
#include <Windows.h>

#define COMMOA , //For pass template as macro arguments.

typedef unsigned __int64 QWORD;

// ============================================================
#ifndef DEFINE_SINGLE_EX
#define DEFINE_SINGLE_EX( className, stmtCtor, stmtDtor ) \
	protected:\
		static inline className** GetInstance(){\
			static className* instance = NULL;\
			return &instance;}\
		inline className(){ stmtCtor }\
		inline ~className(){ stmtDtor }\
	public:\
		static className* CreateInstance(){\
			className** instance = className::GetInstance();\
			if( NULL == *instance )\
				*instance = new className();\
			return *instance;}\
		static bool CloseInstance(){\
			className** instance = className::GetInstance();\
			if( NULL == *instance )\
				return false;\
			delete *instance;\
			*instance = NULL;\
			return true;}
#endif

// ============================================================
#ifndef DEFINE_SINGLE
#define DEFINE_SINGLE( className ) \
	private:\
	static className* _instance;\
	protected:\
	className();\
	~className();\
	public:\
	static className* CreateInstance();\
	static bool CloseInstance();
#endif

// ============================================================
#ifndef IMPLE_SINGLE
#define IMPLE_SINGLE( className ) \
	className* className::_instance = NULL;\
	className* className::CreateInstance(){\
		if( NULL == _instance )\
			_instance = new className();\
		return _instance;}\
	bool className::CloseInstance(){\
		if( NULL == _instance )\
			return false;\
		delete _instance;\
		_instance = NULL;\
		return true;}
#endif

// ============================================================
#ifndef IMPLE_SINGLE_EX
#define IMPLE_SINGLE_EX( templateForm, className ) \
	templateForm\
	className* className::_instance = NULL;\
	templateForm\
	className* className::CreateInstance(){\
		if( NULL == _instance )\
			_instance = new className();\
		return _instance;}\
	templateForm\
	bool className::CloseInstance(){\
		if( NULL == _instance )\
			return false;\
		delete _instance;\
		_instance = NULL;\
		return true;}
#endif

// ============================================================
#ifndef RELEACE_BLOCK
#ifndef _DEBUG
	#define RELEACE_BLOCK( statements ) statements
#else
	#define RELEACE_BLOCK( statements )
#endif
#endif

// ============================================================
#ifndef DEBUG_BLOCK
#ifdef _DEBUG
	#define DEBUG_BLOCK( statements ) statements
#else
	#define DEBUG_BLOCK( statements )
#endif
#endif

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Base{

// ============================================================
template< typename T >
struct AddConst{
	typedef const T Type;
};

template< typename T >
struct AddConst< T* >{
	typedef const T* Type;
};

// ============================================================
template< typename T >
struct RemoveConst{
	typedef T Type;
};

template< typename T >
struct RemoveConst< const T >{
	typedef T Type;
};

template< typename T >
struct RemoveConst< const T* >{
	typedef T* Type;
};

// ============================================================
template< typename T >
struct IsChar{
	enum{ _value = false };
};

template<>
struct IsChar< char >{
	enum{ _value = true };
};

template<>
struct IsChar< char* >{
	enum{ _value = true };
};

// ============================================================
template< typename T >
struct IsWideChar{
	enum{ _value = false };
};

template<>
struct IsWideChar< wchar_t >{
	enum{ _value = true };
};

template<>
struct IsWideChar< wchar_t* >{
	enum{ _value = true };
};

// ============================================================
#ifndef _CONST_TEXT
#define _CONST_TEXT( charType, text ) \
	(charType)( IsWideChar< charType >::_value ? (void*)L##text : (void*)text )
#endif

#ifndef CONST_TEXT
#define CONST_TEXT( charType, text ) _CONST_TEXT( charType, text )
#endif

// ============================================================
template< typename T, typename U >
struct IsSameType{
	enum{ _value = false };
};

template< typename T >
struct IsSameType< T, T >{
	enum{ _value = true };
};

// ============================================================
template< typename T >
struct GetInverseType;

template<>
struct GetInverseType< char >{
	typedef wchar_t Type;
};

template<>
struct GetInverseType< char* >{
	typedef wchar_t* Type;
};

template<>
struct GetInverseType< wchar_t >{
	typedef char Type;
};

template<>
struct GetInverseType< wchar_t* >{
	typedef char* Type;
};

// ============================================================
template< typename T >
inline void Construct( T* p ){
	::new( p ) T();
}

// ============================================================
template< typename T >
inline void Destroy( T* p ){
	p->~T();
}

// ============================================================
template< typename T >
inline void Reconstruct( T* p ){
	Destroy( p );
	Construct( p );
}

// ============================================================
template< typename T >
inline void MemoryReset( T& entry, unsigned char byte = 0 ){
	::FillMemory( &entry, sizeof(entry), byte );
}

// ============================================================
template< typename Var, typename Min, typename Max >
inline void Arrange( Var& var, Min min, Max max ){
	if( var < (Var)min )
		var = (Var)min;
	if( var > (Var)max )
		var = (Var)max;
}

// ============================================================
template< typename T >
class Allocator{

public:
	typedef T Entry;

	Entry* allocate( unsigned int count = 1 ){
		::operator new( sizeof(Entry) * count );
	}

	void deallocate( Entry* entry ){
		::operator delete( entry );
	}
};

// ============================================================
template< unsigned int MaxCount >
class StaticIndexer{

	unsigned int _index;

public:
	enum{ _maxCount = MaxCount };

	StaticIndexer( unsigned int index = 0 ){
		*this = index;
	}

	inline operator unsigned int&(){
		return _index;
	}

	inline StaticIndexer& operator =( unsigned int index ){
		_index = index % _maxCount;
		return *this;
	}

	inline StaticIndexer& operator ++(){
		*this = ( *this + 1 );
		return *this;
	}

	inline StaticIndexer& operator --(){
		*this = ( *this + _maxCount - 1 );
		return *this;
	}
};

// ============================================================
template< typename T, unsigned int MaxCount >
struct StaticContainer{
	typedef T Entry;
	enum{ _maxCount = MaxCount };
	enum{ _entrySize = sizeof(Entry) };
	enum{ _totalSize = _entrySize * _maxCount };
	typedef StaticIndexer< _maxCount > Indexer;
};

// ============================================================
template< typename T >
class StructureNative : public StaticContainer< T, 1 >{

protected:
	Entry _entry;

public:
	operator Entry&(){
		return _entry;
	}

	Entry* operator ->(){
		return &_entry;
	}
};

// ============================================================
template< typename T >
class Structure : public StructureNative< T >{

public:
	Structure(){
		_entry = Entry();
	}

	Structure& operator =( Entry& entry ){
		_entry = entry;
		return *this;
	}
};

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
class DynamicContainer{

protected:
	unsigned int _curCount;

public:
	DynamicContainer(){
		this->setCurCount( 0 );
	}

	unsigned int getCurCount(){
		return _curCount;
	}

	void setCurCount( unsigned int curCount ){
		_curCount = curCount;
	}

	bool isEmpty(){
		return( this->getCurCount() == 0 );
	}

	bool incCurCount(){
		_curCount++;
		return true;
	}

	bool decCurCount(){

		if( this->isEmpty() )
			return false;

		_curCount--;
		return true;
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
template< typename T >
struct PoolNode{
	int _isAcquired;
	T _entry;
};

template< typename T, unsigned int MaxCount >
class Pool : public LimitedContainer< PoolNode< T >, MaxCount >{

public:
	typedef T Entry;
	typedef PoolNode< T > Node;

	Entry* acquire(){

		if( this->isFull() )
			return NULL;

		for( unsigned int i = 0; i < _maxCount; i++ ){
			Node& node = (*this)[i];
			if( node._isAcquired )
				continue;

			node._isAcquired = true;
			this->incCurCount();
			return &node._entry;
		}
		return NULL;
	}

	bool release( Entry* entry ){

		if( this->isEmpty() || entry == NULL )
			return false;

		for( unsigned int i = 0; i < _maxCount; i++ ){
			Node& node = (*this)[i];
			if( &node._entry != entry )
				continue;

			node._isAcquired = false;
			this->decCurCount();
			return true;
		}
		return false;
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
