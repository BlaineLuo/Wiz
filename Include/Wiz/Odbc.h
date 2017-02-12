// ============================================================
// @Brief: Win32 ODBC Classes
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_ODBC_H__
#define __WIZ_ODBC_H__

#ifndef _WIN32_WCE

#include "Wiz/Threading/Forward.h"
#include "Wiz/Windowing/Backward.h"
#include <Sql.h>
#include <SqlExt.h>
#pragma comment( lib, "Odbc32.lib" )

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Odbc{

using namespace Wiz::Threading;
using namespace Wiz::Windowing;

// ============================================================
class Timestamp : public Structure< TIMESTAMP_STRUCT >{
public:
	enum{ _scaleMilliToNano = 1000000 };

	Timestamp& operator <<( SYSTEMTIME& time ){
		(*this)->year = time.wYear;
		(*this)->month = time.wMonth;
		(*this)->day = time.wDay;
		(*this)->hour = time.wHour;
		(*this)->minute = time.wMinute;
		(*this)->second = time.wSecond;
		(*this)->fraction = time.wMilliseconds * _scaleMilliToNano;
		return *this;
	}

	SYSTEMTIME& operator >>( SYSTEMTIME& time ){
		time.wYear = (*this)->year;
		time.wMonth = (*this)->month;
		time.wDay = (*this)->day;
		time.wHour = (*this)->hour;
		time.wMinute = (*this)->minute;
		time.wSecond = (*this)->second;
		time.wMilliseconds = (WORD)( (*this)->fraction / _scaleMilliToNano );
		return time;
	}

	inline char* operator >>( char* string ){
		return SystemTime::CopyTo( *this >> SystemTime(), string );
	}

	inline wchar_t* operator >>( wchar_t* string ){
		return SystemTime::CopyTo( *this >> SystemTime(), string );
	}

	inline Timestamp& getLocalTime(){
		return( *this << SystemTime().getLocalTime() );
	}
};

// ============================================================
template< typename T = TCHAR >
class ConnectionString : public Wiz::String::String< T >{

public:
	bool getValue( Character* key, Character* value ){

		int idxHead = Search( this->getString(), 0, key );
		if( 0 > idxHead )
			return false;

		idxHead += GetLength( key );

		int idxTail = Search( this->getString(), idxHead, CONST_TEXT( Character*, ";" ) );
		if( 0 > idxTail || idxHead > idxTail )
			return false;

		Copy( value, this->getString() + idxHead, idxTail - idxHead );
		return true;
	}

	inline bool getServer( Character* string ){
		return this->getValue( CONST_TEXT( Character*, "SERVER=" ), string );
	}

	inline bool getDatabase( Character* string ){
		return this->getValue( CONST_TEXT( Character*, "DATABASE=" ), string );
	}

	inline bool getUsername( Character* string ){
		return this->getValue( CONST_TEXT( Character*, "UID=" ), string );
	}

	inline bool getPassword( Character* string ){
		return this->getValue( CONST_TEXT( Character*, "PWD=" ), string );
	}

	inline ConnectionString& setValue(
		Character* server,
		Character* database,
		Character* username,
		Character* password
	){
		this->format(
			CONST_TEXT( Character*, "DRIVER=SQL Server;SERVER=%s;DATABASE=%s;UID=%s;PWD=%s;" ),
			server,
			database,
			username,
			password
		);
		return *this;
	}
};

typedef ConnectionString< char > ConnectionStringA;
typedef ConnectionString< wchar_t > ConnectionStringW;
typedef ConnectionString< TCHAR > ConnectionStringT;

// ============================================================
// @Brief: Diag Queue for handle error.
// ============================================================
struct DiagEntry{
	LONG _nativeCode;
	TCHAR _state[8];
	TCHAR _message[ SQL_MAX_MESSAGE_LENGTH ];
};

class DiagQueue : protected SafeCircularQueue< DiagEntry, 64 >{
public:
	typedef SafeCircularQueue Parent;
	using Parent::getCurCount;
	using Parent::peek;

	bool push( SQLHANDLE handle, short handleType ){

		//NOTE: row Must Start with 1.
		for( UINT row = 1; ; row++ ){
			DiagEntry entry = {0};
			short strLen = 0;
			short ret = ::SQLGetDiagRec(
				handleType,
				handle,
				row,
				(SQLTCHAR*)entry._state,
				&entry._nativeCode,
				(SQLTCHAR*)entry._message,
				sizeof(entry._message) / sizeof(TCHAR) - 1,
				&strLen );

			if( !SQL_SUCCEEDED( ret ) || SQL_NO_DATA == ret )
				break;

			this->Parent::push( entry );
		}
		return true;
	}

	void sendLastError( Window& target ){
		DiagEntry* entry = this->peek( this->getCurCount() - 1 );
		if( NULL == entry )
			return;

		target.sendAllTargetString( Text1024T(
			_T("NativeCode = %d, State = %s, Message = %s\r\n"),
			entry->_nativeCode,
			&entry->_state,
			&entry->_message
		) );
	}
};

// ============================================================
// @Brief: SQL Handle Base Class
// ============================================================
template< short HandleType >
class Handle : public HandleT< SQLHANDLE, SQL_NULL_HANDLE >{

	DiagQueue* _diagQueue;

public:
	enum{ _handleType = HandleType };

	inline Handle(){
		this->setDiagQueue( NULL );
	}

	inline ~Handle(){
		if( this->isCreated() )
			this->freeHandle();
	}

	inline bool createHandle( SQLHANDLE parent ){
		return SQL_SUCCEEDED( ::SQLAllocHandle( _handleType, parent, this->getHandleAddress() ) );
	}

	inline bool endTran( const short completionType = SQL_COMMIT ){
		return SQL_SUCCEEDED( ::SQLEndTran( _handleType, *this, completionType ) );
	}

	inline bool freeHandle(){
		return SQL_SUCCEEDED( ::SQLFreeHandle( _handleType, *this ) );
	}

	inline DiagQueue* getDiagQueue(){
		return _diagQueue;
	}

	inline void setDiagQueue( DiagQueue* diagQueue ){
		_diagQueue = diagQueue;
	}

	inline bool pushDiagQueue(){
		if( this->getDiagQueue() == NULL )
			return false;
		return this->getDiagQueue()->push( *this, _handleType );
	}
};

// ============================================================
class Environment : public Handle< SQL_HANDLE_ENV >{
public:
	bool create(){
		DiagQueue* diagQueue = this->getDiagQueue();
		Reconstruct( this );
		this->setDiagQueue( diagQueue );

		do{
			if( !this->createHandle( SQL_NULL_HANDLE ) )
				break;

			// Set ODBC Version.
			if( !this->setAttribute( SQL_ATTR_ODBC_VERSION, (PVOID)SQL_OV_ODBC3 ) )
				break;

			return true;
		}while(0);

		this->pushDiagQueue();
		return false;
	}

	inline bool getAttribute( DWORD attribute, PVOID value, DWORD valueSize, PLONG valueLen = NULL ){
		return SQL_SUCCEEDED( ::SQLGetEnvAttr( *this, attribute, value, valueSize, valueLen ) );
	}

	inline bool setAttribute( DWORD attribute, PVOID value, DWORD valueLen = 0 ){
		return SQL_SUCCEEDED( ::SQLSetEnvAttr( *this, attribute, value, valueLen ) );
	}
};

// ============================================================
class Connection : public Handle< SQL_HANDLE_DBC >{

	Environment _environment;

protected:
	inline bool connect( PTCHAR connectionString ){
		TCHAR buffer[ SQL_MAX_MESSAGE_LENGTH * 2 ] = {0};// MSDN suggest is 1024.
		short strLen = 0;
		return SQL_SUCCEEDED( ::SQLDriverConnect(
			*this,
			NULL,
			(SQLTCHAR*)connectionString,
			::lstrlen( connectionString ),
			(SQLTCHAR*)buffer,
			sizeof(buffer) / sizeof(TCHAR),
			&strLen,
			SQL_DRIVER_NOPROMPT
		) );
	}

public:
	inline ~Connection(){
		if( this->isCreated() )
			::SQLDisconnect( *this );
	}

	inline bool create(
		PTCHAR server,
		PTCHAR database,
		PTCHAR username,
		PTCHAR password,
		DWORD timeoutSecond = 5
	){
		return this->create(
			ConnectionStringT().setValue( server, database, username, password ),
			timeoutSecond
		);
	}

	bool create( PTCHAR connectionString, DWORD timeoutSecond = 5 ){
		DiagQueue* diagQueue = this->getDiagQueue();
		Reconstruct( this );
		this->setDiagQueue( diagQueue );

		do{
			_environment.setDiagQueue( this->getDiagQueue() );

			if( !_environment.create() )
				break;

			if( !this->createHandle( _environment ) )
				break;

			if( !this->setLoginTimeout( timeoutSecond ) )
				break;

			if( !this->setConnectionTimeout( timeoutSecond ) )
				break;

			if( !this->connect( connectionString ) )
				break;

			return true;
		}while(0);

		this->pushDiagQueue();
		return false;
	}

	inline bool isConnected(){
		DWORD isConnectionDead = SQL_CD_TRUE;
		this->getAttribute( SQL_ATTR_CONNECTION_DEAD, &isConnectionDead, sizeof(isConnectionDead) );
		return( SQL_CD_TRUE != isConnectionDead );
	}

	inline bool setLoginTimeout( DWORD timeoutSecond ){
		return this->setAttribute( SQL_ATTR_LOGIN_TIMEOUT, (PVOID)timeoutSecond );
	}

	inline bool setConnectionTimeout( DWORD timeoutSecond ){
		return this->setAttribute( SQL_ATTR_CONNECTION_TIMEOUT, (PVOID)timeoutSecond );
	}

	inline bool getAttribute( DWORD attribute, PVOID value, DWORD valueSize, PLONG valueLen = NULL ){
		return SQL_SUCCEEDED( ::SQLGetConnectAttr( *this, attribute, value, valueSize, valueLen ) );
	}

	inline bool setAttribute( DWORD attribute, PVOID value, DWORD valueLen = 0 ){
		return SQL_SUCCEEDED( ::SQLSetConnectAttr( *this, attribute, value, valueLen ) );
	}
};

// ============================================================
struct ParameterType{
	short _cType;
	DWORD _cSize;
	short _sqlType;
	DWORD _sqlSize;

	ParameterType( const short cType, const DWORD cSize = 0 ){
		MemoryReset( *this );

		_cType = cType;
		_sqlType = this->GetSqlType( _cType );

		if( this->IsFixedSize( _cType ) ){
			_cSize = this->GetCSize( _cType );
			_sqlSize = this->GetSqlSize( _sqlType );
		}else{
			_cSize = cSize;
			_sqlSize = cSize;
		}
	}

	static bool IsFixedSize( const short cType ){
		return !(
			SQL_C_BINARY == cType ||
			SQL_C_CHAR == cType ||
			SQL_C_WCHAR == cType
		);
	}

	static const short GetCSize( const short cType ){
		switch( cType ){
		case SQL_C_TINYINT: return sizeof(char);
		case SQL_C_SHORT: return sizeof(short);
		case SQL_C_LONG: return sizeof(int);
		case SQL_C_SBIGINT: return sizeof(__int64);
		case SQL_C_FLOAT: return sizeof(float);
		case SQL_C_DOUBLE: return sizeof(double);
		case SQL_C_TIMESTAMP: return sizeof(TIMESTAMP_STRUCT);
		default: return 0;
		}
	}

	static const short GetSqlType( const short cType ){
		switch( cType ){
		case SQL_C_TINYINT: return SQL_TINYINT;
		case SQL_C_SHORT: return SQL_SMALLINT;
		case SQL_C_LONG: return SQL_INTEGER;
		case SQL_C_SBIGINT: return SQL_BIGINT;
		case SQL_C_FLOAT: return SQL_FLOAT;
		case SQL_C_DOUBLE: return SQL_DOUBLE;
		case SQL_C_TIMESTAMP: return SQL_TIMESTAMP;

		case SQL_C_BINARY: return SQL_BINARY;
		case SQL_C_CHAR: return SQL_CHAR;
		case SQL_C_WCHAR: return SQL_WCHAR;
		default: return 0;
		}
	}

	static const DWORD GetSqlSize( const short sqlType ){
		switch( sqlType ){
		case SQL_TINYINT: return 3;
		case SQL_SMALLINT: return 5;
		case SQL_INTEGER: return 10;
		case SQL_BIGINT: return 20;
		case SQL_FLOAT: return 15;
		case SQL_DOUBLE: return 15;
		case SQL_TIMESTAMP: return 23;
		default: return 0;
		}
	}
};

// ============================================================
class Statement : public Handle< SQL_HANDLE_STMT >{

public:
	inline ~Statement(){
		if( this->isCreated() )
			::SQLCloseCursor( *this );
	}

	inline bool create( Connection& connection ){
		Reconstruct( this );
		this->setDiagQueue( connection.getDiagQueue() );
		return this->createHandle( connection );
	}

	bool createAndExecute( Connection& connection, PTCHAR sqlCmd ){
		do{
			if( !this->create( connection ) )
				break;

			if( !this->setConcurrencyType() )
				break;

			if( !this->execDirect( sqlCmd ) )
				break;

			return true;
		}while(0);

		this->pushDiagQueue();
		return false;
	}

	inline bool getAttribute( DWORD attribute, PVOID value, DWORD valueSize, PLONG valueLen = NULL ){
		return SQL_SUCCEEDED( ::SQLGetStmtAttr( *this, attribute, value, valueSize, valueLen ) );
	}

	inline bool setAttribute( DWORD attribute, PVOID value, DWORD valueLen = 0 ){
		return SQL_SUCCEEDED( ::SQLSetStmtAttr( *this, attribute, value, valueLen ) );
	}

	inline UINT getColCount(){
		short colCount = 0;
		::SQLNumResultCols( *this, &colCount );
		return colCount;
	}

	inline UINT getRowCount(){
		DWORD rowCount = 0;
		this->getAttribute( SQL_ATTR_ROW_NUMBER, &rowCount, sizeof(rowCount) );
		return rowCount;
	}

	inline bool setBookmarkType( DWORD bookmarkType = SQL_UB_VARIABLE ){
		return false;
	}

	inline bool setConcurrencyType( DWORD concurrencyType = SQL_CONCUR_READ_ONLY ){
		return this->setAttribute( SQL_ATTR_CONCURRENCY, (PVOID)concurrencyType );
	}

	inline bool setCursorType( DWORD cursorType = SQL_CURSOR_KEYSET_DRIVEN ){
		return this->setAttribute( SQL_ATTR_CURSOR_TYPE, (PVOID)cursorType );
	}

	inline bool setRowArraySize( DWORD arraySize ){
		return this->setAttribute( SQL_ATTR_ROW_ARRAY_SIZE, (PVOID)arraySize );
	}

	// Column-Wise or Row-Wise.
	inline bool setRowBindType( DWORD size = SQL_BIND_BY_COLUMN ){
		return this->setAttribute( SQL_ATTR_ROW_BIND_TYPE, (PVOID)size );
	}

	inline bool setParameterArraySize( DWORD arraySize ){
		return this->setAttribute( SQL_ATTR_PARAMSET_SIZE, (PVOID)arraySize );
	}

	inline bool setParameterBindType( DWORD size = SQL_PARAM_BIND_BY_COLUMN ){
		return this->setAttribute( SQL_ATTR_PARAM_BIND_TYPE, (PVOID)size );
	}

	inline bool bindColumn(
		WORD idx,
		PVOID value,
		DWORD valueSize,
		WORD cType = SQL_C_DEFAULT,
		SQLLEN* valueLenOrInd = NULL
	){
		return SQL_SUCCEEDED( ::SQLBindCol( *this, idx, cType, value, valueSize, valueLenOrInd ) );
	}

	inline bool bindParameterNative(
		const WORD idx,
		PVOID value,
		const DWORD valueSize,
		const short cType = SQL_C_DEFAULT,
		const short sqlType = SQL_INTEGER,
		const DWORD sqlSize = 0,
		const short ioType = SQL_PARAM_INPUT,
		SQLLEN* valueLenOrInd = NULL
	){
		if( SQL_SUCCEEDED( ::SQLBindParameter( *this, idx, ioType, cType, sqlType, sqlSize, 0, value, valueSize, valueLenOrInd ) ) )
			return true;

		this->pushDiagQueue();
		return false;
	}

	inline bool bindParameter(
		const WORD idx,
		PVOID value,
		const short cType,
		const DWORD cSize,
		const short ioType = SQL_PARAM_INPUT,
		SQLLEN* valueLenOrInd = NULL
	){
		ParameterType parameterType( cType, cSize );
		return this->bindParameterNative(
			idx,
			value,
			parameterType._cSize,
			parameterType._cType,
			parameterType._sqlType,
			parameterType._sqlSize,
			ioType,
			valueLenOrInd
		);
	}

	inline bool bindParameterEx( const WORD idx, char* value, const DWORD cSize, const short ioType = SQL_PARAM_INPUT, SQLLEN* valueLenOrInd = NULL ){
		return this->bindParameter( idx, value, SQL_C_CHAR, cSize, ioType, valueLenOrInd );
	}

	inline bool bindParameterEx( const WORD idx, wchar_t* value, const DWORD cSize, const short ioType = SQL_PARAM_INPUT, SQLLEN* valueLenOrInd = NULL ){
		return this->bindParameter( idx, value, SQL_C_WCHAR, cSize, ioType, valueLenOrInd );
	}

	inline bool bindParameterEx( const WORD idx, char& value, const short ioType = SQL_PARAM_INPUT ){
		return this->bindParameter( idx, &value, SQL_C_TINYINT, 0, ioType );
	}

	inline bool bindParameterEx( const WORD idx, unsigned char& value, const short ioType = SQL_PARAM_INPUT ){
		return this->bindParameter( idx, &value, SQL_C_TINYINT, 0, ioType );
	}

	inline bool bindParameterEx( const WORD idx, short& value, const short ioType = SQL_PARAM_INPUT ){
		return this->bindParameter( idx, &value, SQL_C_SHORT, 0, ioType );
	}

	inline bool bindParameterEx( const WORD idx, unsigned short& value, const short ioType = SQL_PARAM_INPUT ){
		return this->bindParameter( idx, &value, SQL_C_SHORT, 0, ioType );
	}

	inline bool bindParameterEx( const WORD idx, int& value, const short ioType = SQL_PARAM_INPUT ){
		return this->bindParameter( idx, &value, SQL_C_LONG, 0, ioType );
	}

	inline bool bindParameterEx( const WORD idx, unsigned int& value, const short ioType = SQL_PARAM_INPUT ){
		return this->bindParameter( idx, &value, SQL_C_LONG, 0, ioType );
	}

	inline bool bindParameterEx( const WORD idx, __int64& value, const short ioType = SQL_PARAM_INPUT ){
		return this->bindParameter( idx, &value, SQL_C_SBIGINT, 0, ioType );
	}

	inline bool bindParameterEx( const WORD idx, unsigned __int64& value, const short ioType = SQL_PARAM_INPUT ){
		return this->bindParameter( idx, &value, SQL_C_SBIGINT, 0, ioType );
	}

	inline bool bindParameterEx( const WORD idx, double& value, const short ioType = SQL_PARAM_INPUT ){
		return this->bindParameter( idx, &value, SQL_C_DOUBLE, 0, ioType );
	}

	inline bool bindParameterEx( const WORD idx, TIMESTAMP_STRUCT& value, const short ioType = SQL_PARAM_INPUT ){
		return this->bindParameter( idx, &value, SQL_C_TIMESTAMP, 0, ioType );
	}

	inline SQLRETURN moreResults(){
		return ::SQLMoreResults( *this );
	}

	inline bool prepare( PTCHAR sqlCmd, DWORD sqlCmdLen = SQL_NTS ){
		return SQL_SUCCEEDED( ::SQLPrepare( *this, (SQLTCHAR*)sqlCmd, sqlCmdLen ) );
	}

	inline SQLRETURN execNative(){
		return ::SQLExecute( *this );
	}

	inline bool exec(){
		SQLRETURN sqlRet = this->execNative();
		if( SQL_SUCCEEDED( sqlRet ) || SQL_NO_DATA == sqlRet )
			return true;

		this->pushDiagQueue();
		return false;
	}

	inline bool execDirect( PTCHAR sqlCmd, DWORD sqlCmdLen = SQL_NTS ){
		return SQL_SUCCEEDED( ::SQLExecDirect( *this, (SQLTCHAR*)sqlCmd, sqlCmdLen ) );
	}

	inline bool execBulk( WORD operation = SQL_FETCH_BY_BOOKMARK ){
		return SQL_SUCCEEDED( ::SQLBulkOperations( *this, operation ) );
	}

	inline bool fetchNext(){
		return SQL_SUCCEEDED( ::SQLFetch( *this ) );
	}

	inline bool fetchScroll( WORD fetchType = SQL_FETCH_NEXT, DWORD offset = 0 ){
		return SQL_SUCCEEDED( ::SQLFetchScroll( *this, fetchType, offset ) );
	}

	inline bool getDataNative(
		const WORD idx,
		PVOID value,
		const DWORD valueSize,
		const WORD cType = SQL_C_DEFAULT,
		SQLLEN* valueLenOrInd = NULL
	){
		return SQL_SUCCEEDED( ::SQLGetData( *this, idx, cType, value, valueSize, valueLenOrInd ) );
	}

	template< typename T >
	inline bool getDataT( const WORD idx, T& value, const WORD cType = SQL_C_DEFAULT, SQLLEN* valueLenOrInd = NULL ){
		return this->getDataNative( idx, &value, sizeof(value), cType, valueLenOrInd );
	}

	inline bool getData( const WORD idx, char* value, const DWORD cSize ){
		return this->getDataNative( idx, value, cSize, SQL_C_CHAR );
	}

	inline bool getData( const WORD idx, wchar_t* value, const DWORD cSize ){
		return this->getDataNative( idx, value, cSize, SQL_C_WCHAR );
	}

	inline bool getData( const WORD idx, bool& value ){
		return this->getDataT( idx, value, SQL_C_BIT );
	}

	inline bool getData( const WORD idx, char& value ){
		return this->getDataT( idx, value, SQL_C_TINYINT );
	}

	inline bool getData( const WORD idx, unsigned char& value ){
		return this->getDataT( idx, value, SQL_C_UTINYINT );
	}

	inline bool getData( const WORD idx, short& value ){
		return this->getDataT( idx, value, SQL_C_SHORT );
	}

	inline bool getData( const WORD idx, unsigned short& value ){
		return this->getDataT( idx, value, SQL_C_USHORT );
	}

	inline bool getData( const WORD idx, int& value ){
		return this->getDataT( idx, value, SQL_C_LONG );
	}

	inline bool getData( const WORD idx, unsigned int& value ){
		return this->getDataT( idx, value, SQL_C_ULONG );
	}

	inline bool getData( const WORD idx, __int64& value ){
		return this->getDataT( idx, value, SQL_C_SBIGINT );
	}

	inline bool getData( const WORD idx, unsigned __int64& value ){
		return this->getDataT( idx, value, SQL_C_UBIGINT );
	}

	inline bool getData( const WORD idx, double& value ){
		return this->getDataT( idx, value, SQL_C_DOUBLE );
	}

	inline bool getData( const WORD idx, TIMESTAMP_STRUCT& value ){
		return this->getDataT( idx, value, SQL_C_TIMESTAMP );
	}
};

}}
// ===================================Namespace Tail==========================================

#endif

#endif
