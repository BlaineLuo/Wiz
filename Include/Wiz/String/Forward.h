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
inline int Compare( char* dst, AddConst< char* >::Type src, unsigned int len = -1 ){
	return ::strncmp( dst, src, len );
}

inline int Compare( wchar_t* dst, AddConst< wchar_t* >::Type src, unsigned int len = -1 ){
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
inline char* Copy( char* dst, AddConst< char* >::Type src ){
	::StringCchCopyA( dst, STRSAFE_MAX_CCH, src );
	return dst;
}

inline wchar_t* Copy( wchar_t* dst, AddConst< wchar_t* >::Type src ){
	::StringCchCopyW( dst, STRSAFE_MAX_CCH, src );
	return dst;
}

// ============================================================
inline char* Copy( char* dst, char* src, unsigned int len ){
	::StringCchCopyNA( dst, STRSAFE_MAX_CCH, src, len );
	return dst;
}

inline wchar_t* Copy( wchar_t* dst, wchar_t* src, unsigned int len ){
	::StringCchCopyNW( dst, STRSAFE_MAX_CCH, src, len );
	return dst;
}

// ============================================================
inline unsigned int GetLength( const char* str ){
	return ::strlen( str );
}

inline unsigned int GetLength( const wchar_t* str ){
	return ::wcslen( str );
}

// ============================================================
inline char* ToLowerCase( char* str ){
	return ::CharLowerA( str );
}

inline wchar_t* ToLowerCase( wchar_t* str ){
	return ::CharLowerW( str );
}

// ============================================================
inline char* ToUpperCase( char* str ){
	return ::CharUpperA( str );
}

inline wchar_t* ToUpperCase( wchar_t* str ){
	return ::CharUpperW( str );
}

// ============================================================
inline char* VPrintf( char* dst, unsigned int maxLen, AddConst< char* >::Type format, va_list argList ){
	::StringCchVPrintfA( dst, maxLen, format, argList );
	return dst;
}

inline wchar_t* VPrintf( wchar_t* dst, unsigned int maxLen, AddConst< wchar_t* >::Type format, va_list argList ){
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
inline char* Printf( char* dst, unsigned int maxLen, AddConst< char* >::Type format, ... ){
	VPRINTF( dst, maxLen, format );
	return dst;
}

inline wchar_t* Printf( wchar_t* dst, unsigned int maxLen, AddConst< wchar_t* >::Type format, ... ){
	VPRINTF( dst, maxLen, format );
	return dst;
}

// ============================================================
inline char* PrintfEx( char* dst, unsigned int maxLen, AddConst< char* >::Type format, ... ){
	unsigned int offset = GetLength( dst );
	Arrange( offset, 0, maxLen );
	VPRINTF( dst + offset, maxLen - offset, format );
	return dst;
}

inline wchar_t* PrintfEx( wchar_t* dst, unsigned int maxLen, AddConst< wchar_t* >::Type format, ... ){
	unsigned int offset = GetLength( dst );
	Arrange( offset, 0, maxLen );
	VPRINTF( dst + offset, maxLen - offset, format );
	return dst;
}

// ============================================================
inline int Convert( char* dst, unsigned int maxLen, GetInverseType< char* >::Type src, unsigned int codePage = CP_ACP ){
	return ::WideCharToMultiByte( codePage, 0, src, -1, dst, maxLen, NULL, NULL );
}

inline int Convert( wchar_t* dst, unsigned int maxLen, GetInverseType< wchar_t* >::Type src, unsigned int codePage = CP_ACP ){
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
			Copy( *this, (Entry*)str );
		else
			Convert( *this, _maxCount, (GetInverseType< Entry* >::Type)str );
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
