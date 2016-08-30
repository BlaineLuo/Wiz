// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_STRING_FORWARD_H__
#define __WIZ_STRING_FORWARD_H__

#include "Wiz/Core/Forward.h"
#include <TChar.h>
#include <StrSafe.h>
#include <StdArg.h>

// ===================================Namespace Head==========================================
namespace Wiz{ namespace String{

using namespace Wiz::Core;

// ============================================================
template< typename T >
inline int Compare( T dst, typename AddConst< T >::Type src, unsigned int len = -1 );

template<>
inline int Compare( char* dst, typename AddConst< char* >::Type src, unsigned int len ){
	return ::strncmp( dst, src, len );
}

template<>
inline int Compare( wchar_t* dst, typename AddConst< wchar_t* >::Type src, unsigned int len ){
	return ::wcsncmp( dst, src, len );
}

// ============================================================
template< typename T >
inline int Search( T str, unsigned int offset, T token ){
	unsigned int strLen = GetLength( str );
	unsigned int tokenLen = GetLength( token );
	for( unsigned int i = offset; i < strLen; i++ )
		if( 0 == Compare( str + i, token, tokenLen ) )
			return i;
	return -1;
}

// ============================================================
template< typename T >
inline T Copy( T dst, typename AddConst< T >::Type src );

template<>
inline char* Copy( char* dst, typename AddConst< char* >::Type src ){
	::StringCchCopyA( dst, STRSAFE_MAX_CCH, src );
	return dst;
}

template<>
inline wchar_t* Copy( wchar_t* dst, typename AddConst< wchar_t* >::Type src ){
	::StringCchCopyW( dst, STRSAFE_MAX_CCH, src );
	return dst;
}

// ============================================================
template< typename T >
inline T Copy( T dst, T src, unsigned int len );

template<>
inline char* Copy( char* dst, char* src, unsigned int len ){
	::StringCchCopyNA( dst, STRSAFE_MAX_CCH, src, len );
	return dst;
}

template<>
inline wchar_t* Copy( wchar_t* dst, wchar_t* src, unsigned int len ){
	::StringCchCopyNW( dst, STRSAFE_MAX_CCH, src, len );
	return dst;
}

// ============================================================
template< typename T >
inline unsigned int GetLength( T str );

template<>
inline unsigned int GetLength( char* str ){
	return ::strlen( str );
}

template<>
inline unsigned int GetLength( const char* str ){
	return GetLength( (char*)str );
}

template<>
inline unsigned int GetLength( wchar_t* str ){
	return ::wcslen( str );
}

template<>
inline unsigned int GetLength( const wchar_t* str ){
	return GetLength( (wchar_t*)str );
}

// ============================================================
template< typename T >
inline T ToLowerCase( T str );

template<>
inline char* ToLowerCase( char* str ){
	return ::CharLowerA( str );
}

template<>
inline wchar_t* ToLowerCase( wchar_t* str ){
	return ::CharLowerW( str );
}

// ============================================================
template< typename T >
inline T ToUpperCase( T str );

template<>
inline char* ToUpperCase( char* str ){
	return ::CharUpperA( str );
}

template<>
inline wchar_t* ToUpperCase( wchar_t* str ){
	return ::CharUpperW( str );
}

// ============================================================
template< typename T >
inline T VPrintf( T dst, unsigned int maxLen, typename AddConst< T >::Type format, va_list argList );

template<>
inline char* VPrintf( char* dst, unsigned int maxLen, typename AddConst< char* >::Type format, va_list argList ){
	::StringCchVPrintfA( dst, maxLen, format, argList );
	return dst;
}

template<>
inline wchar_t* VPrintf( wchar_t* dst, unsigned int maxLen, typename AddConst< wchar_t* >::Type format, va_list argList ){
	::StringCchVPrintfW( dst, maxLen, format, argList );
	return dst;
}

// ============================================================
#ifndef VPRINTF
#define VPRINTF( dst, maxLen, format ) \
	va_list	args;\
	va_start( args, format );\
	VPrintf( dst, maxLen, format, args );\
	va_end( args );
#endif

// ============================================================
template< typename T >
inline T Printf( T dst, unsigned int maxLen, typename AddConst< T >::Type format, ... );

template<>
inline char* Printf( char* dst, unsigned int maxLen, typename AddConst< char* >::Type format, ... ){
	VPRINTF( dst, maxLen, format );
	return dst;
}

template<>
inline wchar_t* Printf( wchar_t* dst, unsigned int maxLen, typename AddConst< wchar_t* >::Type format, ... ){
	VPRINTF( dst, maxLen, format );
	return dst;
}

// ============================================================
template< typename T >
inline T PrintfEx( T dst, unsigned int maxLen, typename AddConst< T >::Type format, ... );

template<>
inline char* PrintfEx( char* dst, unsigned int maxLen, typename AddConst< char* >::Type format, ... ){
	unsigned int offset = GetLength( dst );
	Arrange( offset, 0, maxLen );
	VPRINTF( dst + offset, maxLen - offset, format );
	return dst;
}

template<>
inline wchar_t* PrintfEx( wchar_t* dst, unsigned int maxLen, typename AddConst< wchar_t* >::Type format, ... ){
	unsigned int offset = GetLength( dst );
	Arrange( offset, 0, maxLen );
	VPRINTF( dst + offset, maxLen - offset, format );
	return dst;
}

// ============================================================
template< typename T >
inline int Convert( T dst, unsigned int maxLen, typename GetInverseType< T >::Type src, unsigned int codePage = CP_ACP );

template<>
inline int Convert( char* dst, unsigned int maxLen, typename GetInverseType< char* >::Type src, unsigned int codePage ){
	return ::WideCharToMultiByte( codePage, 0, src, -1, dst, maxLen, NULL, NULL );
}

template<>
inline int Convert( wchar_t* dst, unsigned int maxLen, typename GetInverseType< wchar_t* >::Type src, unsigned int codePage ){
	return ::MultiByteToWideChar( codePage, 0, src, -1, dst, maxLen );
}

// ============================================================
template< typename T >
T* ToFormalText( T* text, QWORD cent ){
	enum{ buf_size = 64 };
	T buffer[ buf_size ] = {0};
	unsigned int head = buf_size - 2;

	for( unsigned int i = 0; i < 2; i++ ){
		buffer[ head-- ] = ( cent % 10 ) + 0x30;
		cent /= 10;
	}
	buffer[ head-- ] = 0x2E;

	for( unsigned int i = 0; ; i++ ){
		buffer[ head-- ] = ( cent % 10 ) + 0x30;
		cent /= 10;
		if( cent <= 0 )
			break;

		if( i % 3 == 2 )
			buffer[ head-- ] = 0x2C;
	}
	return Copy( text, &buffer[ head + 1 ] );
}

// ============================================================
template< typename T, unsigned int MaxCount >
class StringArray : public Array< T, MaxCount >{

public:
	template< typename T >
	StringArray& operator <<( T* str ){
		this->clear();

		if( NULL == str || _maxCount <= GetLength( str ) )
			return *this;

		if( IsSameType< Entry*, RemoveConst< T* >::Type >::_value )
			Copy< Entry* >( *this, (Entry*)str );
		else
			Convert< Entry* >( *this, _maxCount, (GetInverseType< Entry* >::Type)str );
		return *this;
	}
};

// ============================================================
template< typename T = TCHAR >
class MaxPath : public StringArray< T, MAX_PATH >{};

// ============================================================
template< typename T = TCHAR >
class Text32 : public StringArray< T, 32 >{};

// ============================================================
template< typename T = TCHAR >
class Text64 : public StringArray< T, 64 >{};

// ============================================================
template< typename T = TCHAR >
class Text1024 : public StringArray< T, 1024 >{};

}}
// ===================================Namespace Tail==========================================

#endif
