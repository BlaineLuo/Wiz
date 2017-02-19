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
template< typename T >
struct InternetOpt{
	typedef T This;

	DWORD _flags;
	DWORD_PTR _context;

	inline This& setFlags( DWORD v ){
		_flags = v;
		return (This&)*this;
	}

	inline This& setContext( DWORD_PTR v ){
		_context = v;
		return (This&)*this;
	}
};

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
		TCHAR* agent,
		DWORD accessType = INTERNET_OPEN_TYPE_DIRECT,
		TCHAR* proxyName = NULL,
		TCHAR* proxyBypass = NULL,
		DWORD flags = 0
	){
		Reconstruct( this );
		this->setHandle( ::InternetOpen( agent, accessType, proxyName, proxyBypass, flags ) );
		return this->isCreated();
	}
};

// ============================================================
struct ConnectionOpt : InternetOpt< ConnectionOpt >{
	TCHAR* _server;
	DWORD _port;
	TCHAR* _username;
	TCHAR* _password;
	DWORD _service;

	inline ConnectionOpt(){
		MemoryReset( *this );
	}

	inline This& buildFtp( TCHAR* server, WORD port, TCHAR* username, TCHAR* password ){
		this->setServer( server ).
			setPort( port ).
			setService( INTERNET_SERVICE_FTP ).
			setUsername( username ).
			setPassword( password );
		return *this;
	}

	inline This& buildHttp( TCHAR* server, WORD port ){
		this->setServer( server ).setPort( port ).setService( INTERNET_SERVICE_HTTP );
		return *this;
	}

	inline This& setServer( TCHAR* v ){
		_server = v;
		return *this;
	}

	inline This& setPort( WORD v ){
		_port = v;
		return *this;
	}

	inline This& setUsername( TCHAR* v ){
		_username = v;
		return *this;
	}

	inline This& setPassword( TCHAR* v ){
		_password = v;
		return *this;
	}

	inline This& setService( DWORD v ){
		_service = v;
		return *this;
	}
};

// ============================================================
class Connection : public Handle{
public:
	inline bool open( Internet& internet, ConnectionOpt& opt ){
		Reconstruct( this );
		if( !internet.isCreated() )
			return false;

		this->setHandle( ::InternetConnect(
			internet,
			opt._server,
			opt._port,
			opt._username,
			opt._password,
			opt._service,
			opt._flags,
			opt._context
		) );
		return this->isCreated();
	}

	inline bool ftpSetCurrentDirectory( TCHAR* directory ){
		return( 0 != ::FtpSetCurrentDirectory( *this, directory ) );
	}
};

// ============================================================
struct FtpFindOpt : InternetOpt< FtpFindOpt >{
	WIN32_FIND_DATA _foundData;
	TCHAR* _filter;

	inline FtpFindOpt( TCHAR* filter, DWORD flags = FTP_TRANSFER_TYPE_BINARY ){
		MemoryReset( *this );
		this->setFilter( filter ).setFlags( flags );
	}

	inline This& setFilter( TCHAR* v ){
		_filter = v;
		return *this;
	}
};

// ============================================================
struct FtpTransferOpt : InternetOpt< FtpTransferOpt >{
	TCHAR* _remoteFile;
	TCHAR* _localFile;
	BOOL _isFailIfExists;
	DWORD _fileAttributes;

	inline FtpTransferOpt(
		TCHAR* remoteFile,
		TCHAR* localFile,
		DWORD flags = FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD
	){
		MemoryReset( *this );
		this->setRemoteFile( remoteFile ).
			setLocalFile( localFile ).
			setIsFailIfExists( false ).
			setFileAttributes( FILE_ATTRIBUTE_NORMAL ).
			setFlags( flags );
	}

	inline This& setRemoteFile( TCHAR* v ){
		_remoteFile = v;
		return *this;
	}

	inline This& setLocalFile( TCHAR* v ){
		_localFile = v;
		return *this;
	}

	inline This& setIsFailIfExists( BOOL v ){
		_isFailIfExists = v;
		return *this;
	}

	inline This& setFileAttributes( DWORD v ){
		_fileAttributes = v;
		return *this;
	}
};

// ============================================================
class Ftp : public Handle{

protected:
	Connection* _connection;

	bool findFirstFile( FtpFindOpt& opt ){
		this->setHandle( ::FtpFindFirstFile(
			*_connection,
			opt._filter,
			&opt._foundData,
			opt._flags,
			opt._context
		) );
		return this->isCreated();
	}

	inline bool findNextFile( WIN32_FIND_DATA& winFindData ){
		return( 0 != ::InternetFindNextFile( *this, &winFindData ) );
	}

public:
	inline Ftp( Connection* connection = NULL ) : _connection( connection ){
	}

	bool findFile( FtpFindOpt& opt ){
		if( !this->isCreated() )
			return this->findFirstFile( opt );
		return this->findNextFile( opt._foundData );
	}

	inline bool getFile( FtpTransferOpt& opt ){
		return( 0 != ::FtpGetFile(
			*_connection,
			opt._remoteFile,
			opt._localFile,
			opt._isFailIfExists,
			opt._fileAttributes,
			opt._flags,
			opt._context
		) );
	}

	inline bool putFile( FtpTransferOpt& opt ){
		return( 0 != ::FtpPutFile(
			*_connection,
			opt._localFile,
			opt._remoteFile,
			opt._flags,
			opt._context
		) );
	}

	inline bool createDirectory( TCHAR* dirName ){
		return( 0 != ::FtpCreateDirectory( *_connection, dirName ) );
	}
};

// ============================================================
struct HttpOpenOpt : InternetOpt< HttpOpenOpt >{
	TCHAR* _method;
	TCHAR* _target;
	TCHAR* _version;
	TCHAR* _referer;
	TCHAR* _acceptType;

	inline HttpOpenOpt( TCHAR* target ){
		MemoryReset( *this );
		this->setTarget( target ).setFlags( INTERNET_FLAG_RELOAD );
	}

	inline This& setMethod( TCHAR* v = _T("GET") ){
		_method = v;
		return *this;
	}

	inline This& setTarget( TCHAR* v ){
		_target = v;
		return *this;
	}

	inline This& setVersion( TCHAR* v = _T("HTTP/1.1") ){
		_version = v;
		return *this;
	}

	inline This& setReferer( TCHAR* v ){
		_referer = v;
		return *this;
	}

	inline This& setAcceptType( TCHAR* v = _T("*/*") ){
		_acceptType = v;
		return *this;
	}
};

// ============================================================
class Http : public Handle{
public:
	bool open( Connection& connection, HttpOpenOpt& opt ){
		Reconstruct( this );
		if( !connection.isCreated() )
			return false;

		TCHAR* text[] = { opt._acceptType, NULL };
		this->setHandle( ::HttpOpenRequest(
			connection,
			opt._method,
			opt._target,
			opt._version,
			opt._referer,
			(LPCTSTR*)&text,
			opt._flags,
			opt._context
		) );
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
