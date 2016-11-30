// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_SYSTEM_SERVICE_H__
#define __WIZ_SYSTEM_SERVICE_H__

#include "Wiz/System/Forward.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace System{

// ============================================================
class Handle : public HandleT< SC_HANDLE, NULL >{
public:
	inline ~Handle(){
		if( this->isCreated() )
			::CloseServiceHandle( *this );
	}
};

// ============================================================
class Manager : public Handle{
public:
	bool create(
		DWORD desiredAccess = SC_MANAGER_ALL_ACCESS,
		TCHAR* machineName = NULL,
		TCHAR* databaseName = SERVICES_ACTIVE_DATABASE
	){
		Reconstruct( this );
		this->setHandle( ::OpenSCManager( machineName, databaseName, desiredAccess ) );
		return this->isCreated();
	}
};

// ============================================================
struct EnumOpt{
	typedef EnumOpt This;

	SC_HANDLE _manager;
	DWORD _serviceType;
	DWORD _serviceState;
	void* _buffer;
	DWORD _bufSize;
	DWORD _servicesCount;
	DWORD _resumeHandle;

	inline EnumOpt( SC_HANDLE manager, DWORD serviceType, DWORD serviceState = SERVICE_STATE_ALL ){
		MemoryReset( *this );
		this->setManager( manager ).setServiceType( serviceType ).setServiceState( serviceState );
	}

	inline This& setManager( SC_HANDLE v ){
		_manager = v;
		return (This&)*this;
	}

	inline This& setServiceType( DWORD v ){
		_serviceType = v;
		return (This&)*this;
	}

	inline This& setServiceState( DWORD v ){
		_serviceState = v;
		return (This&)*this;
	}

	inline This& setBuffer( void* v ){
		_buffer = v;
		return (This&)*this;
	}
};

// ============================================================
class Enumerator : public Buffer{

public:
	typedef ENUM_SERVICE_STATUS Entry;

protected:
	DWORD _count;

	bool enumServicesStatus( EnumOpt& opt ){
		return( 0 != ::EnumServicesStatus(
			opt._manager,
			opt._serviceType,
			opt._serviceState,
			(Entry*)opt._buffer,
			opt._bufSize,
			&opt._bufSize,
			&opt._servicesCount,
			&opt._resumeHandle
		) );
	}

public:
	inline Enumerator() : _count(0)
	{}

	inline DWORD getCount(){
		return _count;
	}

	inline Entry* getEntry( DWORD index ){
		if( !this->isCreated() )
			return NULL;
		return &( (Entry*)this->getHandle() )[ index ];
	}

	bool create( Manager& manager, DWORD serviceType = SERVICE_WIN32 ){
		EnumOpt opt( manager, serviceType );

		// Get length at first call.
		this->enumServicesStatus( opt );

		this->createBuffer( opt._bufSize );
		opt.setBuffer( *this );

		if( !this->enumServicesStatus( opt ) )
			return false;

		_count = opt._servicesCount;
		return true;
	}
};

// ============================================================
struct ServiceConfig : public Structure< QUERY_SERVICE_CONFIG >{
	typedef ServiceConfig This;

	TCHAR* _svcName;
	TCHAR* _password;
	DWORD _desiredAccess;

	inline ServiceConfig( 
		DWORD serviceType = SERVICE_NO_CHANGE,
		DWORD startType = SERVICE_NO_CHANGE,
		DWORD errorControl = SERVICE_NO_CHANGE
	){
		MemoryReset( *this );
	}

	inline This& buildInstall(
		TCHAR* svcName,
		TCHAR* path,
		DWORD desiredAccess = SERVICE_ALL_ACCESS,
		DWORD errorControl = SERVICE_ERROR_NORMAL
	){
		return this->setSvcName( svcName ).setPath( path ).setDesiredAccess( desiredAccess ).setErrorControl( errorControl );
	}

	inline This& buildOpen( TCHAR* svcName, DWORD desiredAccess = SERVICE_ALL_ACCESS ){
		return this->setSvcName( svcName ).setDesiredAccess( desiredAccess );
	}

	inline This& buildChange(
		DWORD serviceType = SERVICE_NO_CHANGE,
		DWORD startType = SERVICE_NO_CHANGE,
		DWORD errorControl = SERVICE_NO_CHANGE
	){
		return this->setServiceType( serviceType ).setStartType( startType ).setErrorControl( errorControl );
	}

	inline This& setSvcName( TCHAR* v ){
		 _svcName = v;
		return (This&)*this;
	}

	inline This& setDesiredAccess( DWORD v ){
		 _desiredAccess = v;
		return (This&)*this;
	}

	inline This& setServiceType( DWORD v ){
		(*this)->dwServiceType = v;
		return (This&)*this;
	}

	inline This& setStartType( DWORD v ){
		(*this)->dwStartType = v;
		return (This&)*this;
	}

	inline This& setPath( TCHAR* v ){
		(*this)->lpBinaryPathName = v;
		return (This&)*this;
	}

	inline This& setErrorControl( DWORD v ){
		(*this)->dwErrorControl = v;
		return (This&)*this;
	}
};

// ============================================================
struct ServiceStatus : public Structure< SERVICE_STATUS >{};

// ============================================================
class Service : public Handle{

public:
	bool install( Manager& manager, ServiceConfig& config ){
		Reconstruct( this );
		if( !manager.isCreated() )
			return false;

		this->setHandle( ::CreateService(
			manager,
			config._svcName,
			config->lpDisplayName,
			config._desiredAccess,
			config->dwServiceType,
			config->dwStartType,
			config->dwErrorControl,
			config->lpBinaryPathName,
			config->lpLoadOrderGroup,
			&config->dwTagId,
			config->lpDependencies,
			config->lpServiceStartName,
			config._password
		) );
		return this->isCreated();
	}

	inline bool uninstall(){
		return( 0 != ::DeleteService( *this ) );
	}

	bool open( Manager& manager, ServiceConfig& config ){
		Reconstruct( this );
		if( !manager.isCreated() )
			return false;

		this->setHandle( ::OpenService( manager, config._svcName, config._desiredAccess ) );
		return this->isCreated();
	}

	inline bool getConfig( QUERY_SERVICE_CONFIG& config ){
		DWORD bufSize = sizeof( config );
		//::QueryServiceConfig( *this, NULL, bufSize, &bufSize );
		//config.createBuffer( bufSize );
		return ::QueryServiceConfig( *this, &config, bufSize, &bufSize );
	}

	inline bool getStatus( SERVICE_STATUS& status ){
		return ::QueryServiceStatus( *this, &status );
	}

	inline bool setConfig( ServiceConfig& config ){
		return( 0 != ::ChangeServiceConfig(
			*this,
			config->dwServiceType,
			config->dwStartType,
			config->dwErrorControl,
			config->lpBinaryPathName,
			config->lpLoadOrderGroup,
			&config->dwTagId,
			config->lpDependencies,
			config->lpServiceStartName,
			config._password,
			config->lpDisplayName
		) );
	}

	inline bool setStartType( DWORD startType ){
		return this->setConfig( ServiceConfig().buildChange().setStartType( startType ) );
	}

	inline bool control( DWORD ctrlCode = SERVICE_CONTROL_PAUSE ){
		SERVICE_STATUS status = {0};
		return( 0 != ::ControlService( *this, ctrlCode, &status ) );
	}

	inline bool start( DWORD argNum = 0, PTCHAR* args = NULL ){
		return( 0 != ::StartService( *this, argNum, (PCTSTR*)args ) );
	}

	inline bool stop(){
		return this->control( SERVICE_CONTROL_STOP );
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
