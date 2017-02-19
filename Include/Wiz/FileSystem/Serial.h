// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_FILESYSTEM_SERIAL_H__
#define __WIZ_FILESYSTEM_SERIAL_H__

#include "Wiz/FileSystem/Forward.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace FileSystem{

// ============================================================
class Serial : protected File{

public:
	using File::read;
	//using File::write;

	inline ~Serial(){
		if( this->isCreated() )
			this->closeHandle();
	}

	bool create( DWORD portNum ){

		Reconstruct( this );

		StringT fileName;
		if( portNum < 10 )
			fileName.format( _T("COM%d:"), portNum );
		else
			fileName.format( _T("\\\\.\\COM%d"), portNum );

		return this->File::create( fileName, 0, OPEN_EXISTING, 0 );
	}

	inline bool clearError( DWORD* errorCode = NULL, COMSTAT* comStat = NULL ){
		return( 0 != ::ClearCommError( *this, errorCode, comStat ) );
	}

	inline bool getSetting( DCB& dcb ){
		return( 0 != ::GetCommState( *this, &dcb ) );
	}

	inline bool setSetting( DCB& dcb ){
		return( 0 != ::SetCommState( *this, &dcb ) );
	}

	bool setSetting(
		DWORD baudRate,
		BYTE stopBits = ONESTOPBIT,
		BYTE byteSize = 8,
		DWORD rtsControl = RTS_CONTROL_DISABLE
	){
		DCB dcb = {0};
		this->getSetting( dcb );
		dcb.DCBlength = sizeof(dcb);
		dcb.BaudRate = baudRate;
		dcb.StopBits = stopBits;
		dcb.ByteSize = byteSize;
		dcb.fRtsControl = rtsControl;
		return this->setSetting( dcb );
	}

	bool setSettingParity( BYTE parity = SPACEPARITY, bool isCheck = true ){
		DCB dcb = {0};
		this->getSetting( dcb );
		dcb.fParity = isCheck;
		dcb.Parity = parity;
		return this->setSetting( dcb );
	}

	inline bool getTimeouts( COMMTIMEOUTS& v ){
		return( 0 != ::GetCommTimeouts( *this, &v ) );
	}

	inline bool setTimeouts( COMMTIMEOUTS& v ){
		return( 0 != ::SetCommTimeouts( *this, &v ) );
	}

	inline bool purge( DWORD flags ){
		return( 0 != ::PurgeComm( *this, flags ) );
	}

	inline bool rxAbort(){
		return this->purge( PURGE_RXABORT );
	}

	inline bool rxClear(){
		return this->purge( PURGE_RXCLEAR );
	}

	inline bool txAbort(){
		return this->purge( PURGE_TXABORT );
	}

	inline bool txClear(){
		return this->purge( PURGE_TXCLEAR );
	}

	inline DWORD write( BYTE data ){
		return this->File::write( &data, sizeof(data) );
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
