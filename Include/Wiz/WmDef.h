// ============================================================
// @Brief: Define Windows User Messages
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_WMDEF_H__
#define __WIZ_WMDEF_H__

#include <WinUser.h>

// ============================================================
#define	WM_STRING WM_USER + 0x0100 // WPARAM = PTCHAR.
#define	WM_TRAY WM_USER + 0x0200
#define	WM_SOCKET WM_USER + 0x0300
#define	WM_NET_MSG_BASE WM_USER + 0x8000

#endif
