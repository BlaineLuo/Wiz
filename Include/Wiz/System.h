// ============================================================
// @Brief	System Classes
// @Author	Blaine
// @Date	2014/12/23
// ============================================================
#ifndef __WIZ_SYSTEM_H__
#define __WIZ_SYSTEM_H__

#include "Wiz/String/StringT.h"
//#include <ShlWApi.h>

// ===================================Namespace Head==========================================
namespace Wiz{ namespace System{

using namespace Wiz::String;

// ============================================================
// @Brief	Function Pointer Loader
// ============================================================
#ifndef LOAD_API
	#ifdef UNICODE
		#define LOAD_API( dllName, returnType, callType, functionName, ... ) \
			typedef returnType (callType *P##functionName) ( __VA_ARGS__ ); \
			static P##functionName Api_##functionName = (P##functionName)Wiz::System::GetProcAddressEx( dllName, _T( #functionName ), true );
	#else
		#define LOAD_API( dllName, returnType, callType, functionName, ... ) \
			typedef returnType (callType *P##functionName) ( __VA_ARGS__ ); \
			static P##functionName Api_##functionName = (P##functionName)Wiz::System::GetProcAddressEx( dllName, _T( #functionName ), false );
	#endif
#endif

// ============================================================
static void* GetProcAddressEx( PTCHAR moduleName, PTCHAR procName, bool isUnicode ){

	StringT<> procNameFirst;
#ifndef _WIN32_WCE
	StringT< char > procNameSecond;
#else
	StringT< wchar_t > procNameSecond;
#endif

	if( isUnicode )
		procNameFirst.format( _T("%sW"), procName );
	else
		procNameFirst.format( _T("%sA"), procName );

	procNameSecond = procNameFirst;

	void* procAddress = ::GetProcAddress( ::LoadLibrary( moduleName ), procNameSecond );
	if( procAddress != NULL )
		return procAddress;

	procNameSecond = procName;
	return ::GetProcAddress( ::LoadLibrary( moduleName ), procNameSecond );
}

static DWORD GetRandom(){
	LARGE_INTEGER counter = {0};
	DWORD randomNum = 0;

	::QueryPerformanceCounter( &counter );
	::srand( counter.LowPart );
	// Due to rand() return value is 15 bit( 0 ~ 32767 ),
	// so do it three times to generate 32 bit random number.
	randomNum = ::rand() % 4;
	randomNum = randomNum << 15;
	randomNum += ::rand();
	randomNum = randomNum << 15;
	randomNum += ::rand();
	return randomNum;
}

// ============================================================
template< typename T >
inline void ShowMessage( T format, ... );

template<>
inline void ShowMessage( const char* format, ... ){
	Text1024< char > text;
	VPRINTF( &text[0], text._maxCount, format );
	::MessageBoxA( NULL, text, "Message", MB_OK | MB_TOPMOST );
}

template<>
inline void ShowMessage( const wchar_t* format, ... ){
	Text1024< wchar_t > text;
	VPRINTF( &text[0], text._maxCount, format );
	::MessageBoxW( NULL, text, L"Message", MB_OK | MB_TOPMOST );
}

}}
// ===================================Namespace Tail==========================================

#endif
