// ============================================================
// @Author	Blaine
// @Date	2015/03/01
// ============================================================
#ifndef __WIZ_CRYPTOGRAPHY_SIMPLE_H__
#define __WIZ_CRYPTOGRAPHY_SIMPLE_H__

#include "Wiz/String/StringT.h"
#include <WinCrypt.h>
#pragma comment( lib, "Crypt32.lib" )

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Cryptography{

using namespace Wiz::String;

// ============================================================
class HexCryptor{
public:
	template< typename T >
	static T binToHex( BYTE bin ){
		bin &= 0x0F;
		if( bin < 0x0A )
			return bin + 0x30;
		else
			return ( bin - 0x0A ) + 0x41;
	}

	template< typename T >
	static BYTE hexToBin( T hex ){
		hex -= 0x30;
		if( hex < 0x0A )
			return hex;
		hex -= 0x11;
		if( hex < 0x07 )
			return hex + 0x0A;
		return 0;
	}

	template< typename T >
	static UINT binaryToHex( void* bin, UINT binSize, T* hex ){
		char* binary = (char*)bin;
		for( UINT i = 0; i < binSize; i++ ){
			hex[ i * 2 ] = binToHex< T >( ( binary[i] & 0xF0 ) >> 4 );
			hex[ i * 2 + 1 ] = binToHex< T >( binary[i] & 0x0F );
		}
		return binSize * 2;
	}

	template< typename T >
	static UINT hexToBinary( T* hex, void* bin ){
		char* binary = (char*)bin;
		UINT binLen = GetLength( hex ) / 2;
		for( UINT i = 0; i < binLen; i++ ){
			binary[i] = ( hexToBin( hex[ i * 2 ] ) ) << 4;
			binary[i] |= hexToBin( hex[ i * 2 + 1 ] );
		}
		return binLen;
	}
};

// ============================================================
class Base64Cryptor{
public:
	static UINT binaryToString( void* binary, UINT binSize, PTCHAR string, UINT strSize ){
		DWORD len = strSize;
		if( !::CryptBinaryToString( (BYTE*)binary, binSize, CRYPT_STRING_BASE64, string, &len ) )
			return 0;
		return len;
	}

	static UINT stringToBinary( PTCHAR string, void* binary, UINT binSize ){
		DWORD len = binSize;
		if( !::CryptStringToBinary( string, ::lstrlen(string), CRYPT_STRING_BASE64, (BYTE*)binary, &len, NULL, NULL ) )
			return 0;
		return len;
	}
};

// ============================================================
class BcdCryptor{
public:
	static void intToBcd( unsigned __int64 num, char* bcd, unsigned int bcdLen ){
		for( int i = bcdLen - 1; i >= 0; i-- ){
			bcd[i] = num % 10;
			num /= 10;
			bcd[i] |= ( num % 10 ) << 4;
			num /= 10;
		}
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
