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
struct InternetParam{
	DWORD _flags;
	DWORD_PTR _context;

	inline T& setFlags( DWORD v ){
		_flags = v;
		return (T&)*this;
	}

	inline T& setContext( DWORD_PTR v ){
		_context = v;
		return (T&)*this;
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
struct ConnectionParam : InternetParam< ConnectionParam >{
	TCHAR* _server;
	DWORD _port;
	TCHAR* _username;
	TCHAR* _password;
	DWORD _service;

	inline ConnectionParam(){
		MemoryReset( *this );
	}

	inline ConnectionParam& buildFtp( TCHAR* server, WORD port, TCHAR* username, TCHAR* password ){
		this->setServer( server ).
			setPort( port ).
			setService( INTERNET_SERVICE_FTP ).
			setUsername( username ).
			setPassword( password );
		return *this;
	}

	inline ConnectionParam& buildHttp( TCHAR* server, WORD port ){
		this->setServer( server ).setPort( port ).setService( INTERNET_SERVICE_HTTP );
		return *this;
	}

	inline ConnectionParam& setServer( TCHAR* v ){
		_server = v;
		return *this;
	}

	inline ConnectionParam& setPort( WORD v ){
		_port = v;
		return *this;
	}

	inline ConnectionParam& setUsername( TCHAR* v ){
		_username = v;
		return *this;
	}

	inline ConnectionParam& setPassword( TCHAR* v ){
		_password = v;
		return *this;
	}

	inline ConnectionParam& setService( DWORD v ){
		_service = v;
		return *this;
	}
};

// ============================================================
class Connection : public Handle{
public:
	inline bool open( Internet& internet, ConnectionParam& param ){
		Reconstruct( this );
		if( !internet.isCreated() )
			return false;

		this->setHandle( ::InternetConnect(
			internet,
			param._server,
			param._port,
			param._username,
			param._password,
			param._service,
			param._flags,
			param._context
		) );
		return this->isCreated();
	}

	inline bool ftpSetCurrentDirectory( TCHAR* directory ){
		return( 0 != ::FtpSetCurrentDirectory( *this, directory ) );
	}
};

// ============================================================
struct FtpFindParam : InternetParam< FtpFindParam >{
	WIN32_FIND_DATA _foundData;
	TCHAR* _filter;

	inline FtpFindParam( TCHAR* filter, DWORD flags = FTP_TRANSFER_TYPE_BINARY ){
		MemoryReset( *this );
		this->setFilter( filter ).setFlags( flags );
	}

	inline FtpFindParam& setFilter( TCHAR* v ){
		_filter = v;
		return *this;
	}
};

// ============================================================
struct FtpTransferParam : InternetParam< FtpTransferParam >{
	TCHAR* _remoteFile;
	TCHAR* _localFile;
	BOOL _isFailIfExists;
	DWORD _fileAttributes;

	inline FtpTransferParam(
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

	inline FtpTransferParam& setRemoteFile( TCHAR* v ){
		_remoteFile = v;
		return *this;
	}

	inline FtpTransferParam& setLocalFile( TCHAR* v ){
		_localFile = v;
		return *this;
	}

	inline FtpTransferParam& setIsFailIfExists( BOOL v ){
		_isFailIfExists = v;
		return *this;
	}

	inline FtpTransferParam& setFileAttributes( DWORD v ){
		_fileAttributes = v;
		return *this;
	}
};

// ============================================================
class Ftp : public Handle{

protected:
	Connection* _connection;

	bool findFirstFile( FtpFindParam& param ){
		this->setHandle( ::FtpFindFirstFile(
			*_connection,
			param._filter,
			&param._foundData,
			param._flags,
			param._context
		) );
		return this->isCreated();
	}

	inline bool findNextFile( WIN32_FIND_DATA& winFindData ){
		return( 0 != ::InternetFindNextFile( *this, &winFindData ) );
	}

public:
	inline Ftp( Connection* connection = NULL ) : _connection( connection ){
	}

	bool findFile( FtpFindParam& param ){
		if( !this->isCreated() )
			return this->findFirstFile( param );
		return this->findNextFile( param._foundData );
	}

	inline bool getFile( FtpTransferParam& param ){
		return( 0 != ::FtpGetFile(
			*_connection,
			param._remoteFile,
			param._localFile,
			param._isFailIfExists,
			param._fileAttributes,
			param._flags,
			param._context
		) );
	}

	inline bool putFile( FtpTransferParam& param ){
		return( 0 != ::FtpPutFile(
			*_connection,
			param._localFile,
			param._remoteFile,
			param._flags,
			param._context
		) );
	}

	inline bool createDirectory( TCHAR* dirName ){
		return( 0 != ::FtpCreateDirectory( *_connection, dirName ) );
	}
};

// ============================================================
struct HttpOpenParam : InternetParam< HttpOpenParam >{
	TCHAR* _method;
	TCHAR* _target;
	TCHAR* _version;
	TCHAR* _referer;
	TCHAR* _acceptType;

	inline HttpOpenParam( TCHAR* target ){
		MemoryReset( *this );
		this->setTarget( target ).setFlags( INTERNET_FLAG_RELOAD );
	}

	inline HttpOpenParam& setMethod( TCHAR* v = _T("GET") ){
		_method = v;
		return *this;
	}

	inline HttpOpenParam& setTarget( TCHAR* v ){
		_target = v;
		return *this;
	}

	inline HttpOpenParam& setVersion( TCHAR* v = _T("HTTP/1.1") ){
		_version = v;
		return *this;
	}

	inline HttpOpenParam& setReferer( TCHAR* v ){
		_referer = v;
		return *this;
	}

	inline HttpOpenParam& setAcceptType( TCHAR* v = _T("*/*") ){
		_acceptType = v;
		return *this;
	}
};

// ============================================================
class Http : public Handle{
public:
	bool open( Connection& connection, HttpOpenParam& param ){
		Reconstruct( this );
		if( !connection.isCreated() )
			return false;

		TCHAR* text[] = { param._acceptType, NULL };
		this->setHandle( ::HttpOpenRequest(
			connection,
			param._method,
			param._target,
			param._version,
			param._referer,
			(LPCTSTR*)&text,
			param._flags,
			param._context
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
