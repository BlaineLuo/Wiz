// ============================================================
// @Brief	Include WinSock Header safety
// @Author	Blaine
// @Date	2015/08
// ============================================================
#ifndef __WIZ_NETWORK_WINSOCKETHEADER_H__
#define __WIZ_NETWORK_WINSOCKETHEADER_H__

// Include WinSock2.h before windows.h to avoid multiple Winsock definition
#include <WinSock2.h>
#include <Ws2TcpIp.h>
#include <MsWSock.h>

#ifndef _WIN32_WCE
	#pragma comment( lib, "Ws2_32.lib" )
#else
	#pragma comment( lib, "Ws2.lib" )
#endif

#endif
