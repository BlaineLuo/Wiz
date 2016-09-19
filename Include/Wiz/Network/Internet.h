// ============================================================
// @Brief: Windows Internet Classes
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_NETWORK_INTERNET_H__
#define __WIZ_NETWORK_INTERNET_H__

#include "Wiz/String/StringT.h"
#include <WinINet.h>
#pragma comment( lib, "WinINet" )

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Network{ namespace Internet{

using namespace Wiz::String;

// ============================================================
class Handle : public HandleT< HINTERNET, NULL >{
public:
	inline ~Handle(){
		if( this->isCreated() )
			::InternetCloseHandle( *this );
	}
};

// ============================================================
class Internet : public Handle{
public:
	inline bool open(
		PTCHAR agent,
		DWORD accessType = INTERNET_OPEN_TYPE_DIRECT,
		PTCHAR proxyName = NULL,
		PTCHAR proxyBypass = NULL,
		DWORD flags = 0
	){
		Reconstruct( this );
		this->setHandle( ::InternetOpen( agent, accessType, proxyName, proxyBypass, flags ) );
		return this->isCreated();
	}
};

// ============================================================
class Connection : public Handle{

protected:
	inline bool openConnection(
		Internet& internet,
		PTCHAR server,
		WORD port,
		PTCHAR username,
		PTCHAR password,
		DWORD service,
		DWORD flags,
		DWORD_PTR context = 0
	){
		Reconstruct( this );
		if( !internet.isCreated() )
			return false;

		this->setHandle( ::InternetConnect( internet, server, port, username, password, service, flags, context ) );
		return this->isCreated();
	}

public:
	inline bool openFtp(
		Internet& internet,
		PTCHAR server,
		WORD port,
		PTCHAR username,
		PTCHAR password
	){
		return this->openConnection(
			internet,
			server,
			port,
			username,
			password,
			INTERNET_SERVICE_FTP,
			FTP_TRANSFER_TYPE_BINARY
		);
	}

	inline bool openHttp( Internet& internet, PTCHAR server, WORD port ){
	   return this->openConnection( internet, server, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0 );
	}

	inline bool ftpSetCurrentDirectory( PTCHAR directory ){
		return( 0 != ::FtpSetCurrentDirectory( *this, directory ) );
	}
};

// ============================================================
class Ftp : public Handle{

public:
	bool findFile(
		Connection& connection,
		PTCHAR searchExpression,
		WIN32_FIND_DATA& winFindData,
		DWORD flags = FTP_TRANSFER_TYPE_BINARY,
		DWORD_PTR context = NULL
	){
		if( !this->isCreated() )
			return this->findFirstFile( connection, searchExpression, winFindData, flags, context );

		if( !this->findNextFile( winFindData ) ){
			Reconstruct( this );
			return false;
		}
		return true;
	}

	bool findFirstFile(
		Connection& connection,
		PTCHAR searchExpression,
		WIN32_FIND_DATA& winFindData,
		DWORD flags = FTP_TRANSFER_TYPE_BINARY,
		DWORD_PTR context = NULL
	){
		if( !connection.isCreated() || NULL == searchExpression )
			return false;

		Reconstruct( this );
		this->setHandle( ::FtpFindFirstFile( connection, searchExpression, &winFindData, flags, context ) );
		return this->isCreated();
	}

	inline bool findNextFile( WIN32_FIND_DATA& winFindData ){
		return( 0 != ::InternetFindNextFile( *this, &winFindData ) );
	}

	inline bool getFile(
		Connection& connection,
		PTCHAR remoteFile,
		PTCHAR localFile,
		DWORD flags = FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD )
	{
		if( !connection.isCreated() || NULL == remoteFile || NULL == localFile )
			return false;
		return( 0 != ::FtpGetFile( connection, remoteFile, localFile, false, FILE_ATTRIBUTE_NORMAL, flags, 0 ) );
	}

	inline bool putFile(
		Connection& connection,
		PTCHAR localFile,
		PTCHAR remoteFile,
		DWORD flags = FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD )
	{
		if( !connection.isCreated() || NULL == remoteFile || NULL == localFile )
			return false;
		return( 0 != ::FtpPutFile( connection, localFile, remoteFile, flags, 0 ) );
	}

	inline bool createDirectory( Connection& connection, PTCHAR dirName ){
		if( !connection.isCreated() || NULL == dirName )
			return false;
		return( 0 != ::FtpCreateDirectory( connection, dirName ) );
	}
};

// ============================================================
class Http : public Handle{
public:
	bool open(
		Connection& connection,
		PTCHAR objectName,
		PTCHAR verb = _T("GET"),
		PTCHAR version = _T("HTTP/1.1"),
		PTCHAR referer = NULL,
		DWORD flags = INTERNET_FLAG_RELOAD,
		DWORD context = 0
	){
		Reconstruct( this );
		if( !connection.isCreated() )
			return false;

		this->setHandle( ::HttpOpenRequest( connection, verb, objectName, version, referer, NULL, flags, context ) );
		return this->isCreated();
	}

	template< typename T, typename U >
	inline bool send( T headers = NULL, U optional = NULL ){
		return SendRequest( *this, headers, optional );
	}

	inline bool recv( void* outBuffer, DWORD outBufferSize, DWORD* outDataLen ){
		return( 0 != ::InternetReadFile( *this, outBuffer, outBufferSize, outDataLen ) );
	}
};

// ============================================================
inline bool SendRequest( Http& http, const char* headers, const char* optional ){
	return( 0 != ::HttpSendRequestA( http, headers, GetLength( headers ), (void*)optional, GetLength( optional ) ) );
}

inline bool SendRequest( Http& http, const wchar_t* headers, const wchar_t* optional ){
	return( 0 != ::HttpSendRequestW( http, headers, GetLength( headers ), (void*)optional, GetLength( optional ) ) );
}

}}}
// ===================================Namespace Tail==========================================

#endif
