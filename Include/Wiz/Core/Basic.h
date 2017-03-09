// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_CORE_BASIC_H__
#define __WIZ_CORE_BASIC_H__

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
namespace Wiz{ namespace Core{

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
typedef signed __int8 SInt8;
typedef unsigned __int8 UInt8;
typedef signed __int16 SInt16;
typedef unsigned __int16 UInt16;
typedef signed __int32 SInt32;
typedef unsigned __int32 UInt32;
typedef signed __int64 SInt64;
typedef unsigned __int64 UInt64;

#ifndef _WIN64
	typedef SInt32 SIntStd;
	typedef UInt32 UIntStd;
#else
	typedef SInt64 SIntStd;
	typedef UInt64 UIntStd;
#endif

// ============================================================
template< typename T >
struct IsSigned{
	enum{ _value = T(-1) < 0 };
};

// ============================================================
template< typename T >
struct GetTypeMax{
	static const T _value = (T)( ~T(0) ^ ( (T)( IsSigned< T >::_value ? 1 : 0 ) << ( sizeof(T) * 8 - 1 ) ) );
};

// ============================================================
template< typename T >
struct GetDoubledType;

template<>
struct GetDoubledType< unsigned char >{
	typedef unsigned short Type;
};

template<>
struct GetDoubledType< unsigned short >{
	typedef unsigned int Type;
};

template<>
struct GetDoubledType< unsigned int >{
	typedef unsigned __int64 Type;
};

// ============================================================
template< typename T >
struct GetHalvedType;

template<>
struct GetHalvedType< unsigned short >{
	typedef unsigned char Type;
};

template<>
struct GetHalvedType< unsigned int >{
	typedef unsigned short Type;
};

template<>
struct GetHalvedType< unsigned __int64 >{
	typedef unsigned int Type;
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
template< typename T, typename U >
inline int MemoryCompare( T& t, U& u, unsigned char byte = sizeof(T) ){
	return ::memcmp( &t, &u, byte );
}

// ============================================================
template< typename T >
inline void MemoryReset( T& entry, unsigned char byte = 0 ){
	::FillMemory( &entry, sizeof(entry), byte );
}

// ============================================================
template< typename Var, typename Min, typename Max >
inline bool IsOutOf( Var& var, Min min, Max max ){
	return( var < (Var)min || var > (Var)max );
}

// ============================================================
template< typename Var, typename Min, typename Max >
inline Var& Arrange( Var& var, Min min, Max max ){
	if( var < (Var)min )
		var = (Var)min;
	if( var > (Var)max )
		var = (Var)max;
	return var;
}

// ============================================================
template< typename Var, typename Max >
inline Var& Arrange( Var& var, Max max ){
	if( var >= 0 )
		return Arrange( var, 0, max );
	else
		return Arrange( var, max, 0 );
}

// ============================================================
template< typename T >
static inline typename GetDoubledType< T >::Type LeftShiftOr( T a, T b ){
	return ( (GetDoubledType< T >::Type)a << ( sizeof(T) * 8 ) ) | b;
}

// ============================================================
template< typename T >
static inline typename GetHalvedType< T >::Type GetHighPart( T v ){
	return ( (GetHalvedType< T >::Type)( v >> ( sizeof(T) * 4 ) ) );
}

// ============================================================
template< typename T >
static inline typename GetHalvedType< T >::Type GetLowPart( T v ){
	return (GetHalvedType< T >::Type)v;
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
typedef StaticContainer< int, 0 > StructureEmpty;

// ============================================================
template< typename T >
struct StructureNative : public StaticContainer< T, 1 >{

	Entry _entry;

	operator Entry&(){
		return _entry;
	}

	Entry* operator ->(){
		return &_entry;
	}
};

// ============================================================
template< typename T >
struct Structure : public StructureNative< T >{
	inline Structure(){
		_entry = Entry();
	}

	inline Entry* operator &(){
		return (Entry*)this;
	}

	inline Structure& operator =( Entry& entry ){
		_entry = entry;
		return *this;
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

}}
// ===================================Namespace Tail==========================================

#endif
