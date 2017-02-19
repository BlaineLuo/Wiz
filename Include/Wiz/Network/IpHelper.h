// ============================================================
// @Brief: Windows IP Helper API
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_NETWORK_IPHELPER_H__
#define __WIZ_NETWORK_IPHELPER_H__

#include "Wiz/Memory.h"
#include <IpHlpApi.h>
#pragma comment( lib, "IpHlpApi.lib" )

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Network{ namespace IpHelper{

using namespace Wiz::Memory;

// ============================================================
class AdapterInfo{

	Buffer _buffer;
	IP_ADAPTER_INFO* _head;
	IP_ADAPTER_INFO* _tail;

public:
	AdapterInfo(){
		_head = NULL;
		_tail = NULL;

		ULONG size = 0;
		::GetAdaptersInfo( NULL, &size );
		_buffer.createBuffer( size );

		_head = (IP_ADAPTER_INFO*)_buffer.getHandle();
		::GetAdaptersInfo( _head, &size );
	}

	IP_ADAPTER_INFO* next(){

		if( NULL == _tail )
			_tail = _head;
		else
			_tail = _tail->Next;

		return _tail;
	}
};

}}}
// ===================================Namespace Tail==========================================

#endif
