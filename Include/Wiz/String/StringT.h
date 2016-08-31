// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_STRING_STRINGT_H__
#define __WIZ_STRING_STRINGT_H__

#include "Wiz/String/Forward.h"
#include "Wiz/Memory.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace String{

using namespace Wiz::Memory;

// ============================================================
// @Brief: String Template for ASCII and Unicode.
// ============================================================
template< typename T = TCHAR >
class StringT : protected Buffer{

public:
	using Buffer::clear;
	using Buffer::getSize;
	typedef T Character;

	inline StringT(){
	}

	inline StringT( char* str ){
		*this = str;
	}

	inline StringT( wchar_t* str ){
		*this = str;
	}

	inline operator Character*(){
		return this->getString();
	}

	inline bool operator ==( Character* str ){
		return( 0 == Compare( *this, str ) );
	}

	inline StringT& operator =( StringT& str ){
		return *this = (Character*)str;
	}

	StringT& operator =( char* str ){
		if( NULL == str )
			return *this;
		if( this->isMultiByte() ){
			this->createString( GetLength( str ) );
			this->copyFrom( str, this->getSize() );
			return *this;
		}
		if( !this->copyFromMultiByte( str ) )
			this->createString( 1 );
		return *this;
	}

	StringT& operator =( wchar_t* str ){
		if( NULL == str )
			return *this;
		if( this->isUnicode() ){
			this->createString( GetLength( str ) );
			this->copyFrom( str, this->getSize() );
			return *this;
		}
		if( !this->copyFromUnicode( str ) )
			this->createString( 1 );
		return *this;
	}

	inline bool createString( unsigned int strLen ){
		Reconstruct( this );
		return this->createBuffer( sizeof(Character) * ( strLen + 1 ) );
	}

	StringT& format( Character* format, ... ){
		Text1024< Character > text;
		VPRINTF( text, text._maxCount, format );
		*this = text;
		return *this;
	}

	inline unsigned int getLength(){
		return GetLength( *this );
	}

	inline Character* getString(){
		return (Character*)( this->getHandle() );
	}

	inline bool isMultiByte(){
		return IsChar< Character >::_value;
	}

	inline bool isUnicode(){
		return IsWideChar< Character >::_value;
	}

	inline Character* toLowerCase(){
		return ToLowerCase( *this );
	}

	inline Character* toUpperCase(){
		return ToUpperCase( *this );
	}

	bool copyFromMultiByte( char* str, unsigned int codePage = CP_ACP ){

		if( NULL == str || this->isMultiByte() )
			return false;

		// Get the Unicode String Length with null-terminated.
		int len = Convert( NULL, 0, str, codePage );
		if( len <= 0 )
			return false;

		this->createString( len );
		return( 0 < Convert( (wchar_t*)this->getString(), len, str, codePage ) );
	}

	bool copyFromUnicode( wchar_t* str, unsigned int codePage = CP_ACP ){

		if( NULL == str || this->isUnicode() )
			return false;

		// Get the MultiByte String Length with null-terminated.
		int len = Convert( NULL, 0, str, codePage );
		if( len <= 0 )
			return false;

		this->createString( len );
		return( 0 < Convert( (char*)this->getString(), len, str, codePage ) );
	}
};

// ============================================================
// @Brief: Format String of Last Error
// ============================================================
class ErrorString : public StringT<>{
public:
	using StringT::operator =;
	inline ErrorString& getErrorString( DWORD errorCode = GetLastError() ){
		PTCHAR formatMsg = NULL;
		::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			errorCode,
			LANG_NEUTRAL,
			(PTCHAR)&formatMsg,
			0,
			NULL
		);
		*this = formatMsg;
		::LocalFree( formatMsg );
		return *this;
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
