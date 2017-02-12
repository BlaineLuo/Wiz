// ============================================================
// @Author: Blaine Luo
// @Date: 2016/12
// ============================================================
#ifndef __WIZ_SYSTEM_HOOK_H__
#define __WIZ_SYSTEM_HOOK_H__

#include "Wiz/System/Forward.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace System{

// ============================================================
struct HookOpt{
	WPARAM _wParam;
	LPARAM _lParam;
};

template< int T >
class HookObj : public HandleT< HHOOK, NULL >{

protected:
	static LRESULT __stdcall HookProc( int code, WPARAM wParam, LPARAM lParam ){
		HookOpt opt = { wParam, lParam };
		if( 0 <= code )
			HookFtor::CreateInstance()->onHook( opt );

		return ::CallNextHookEx( NULL, code, opt._wParam, opt._lParam );
	}

	virtual void onHook( HookOpt& opt ) = 0;

public:
	DEFINE_SINGLE_EX( HookObj, ;,
		if( this->isCreated() )
			::UnhookWindowsHookEx( *this );
	);

	enum{ HookType = T };

	inline bool create( DWORD threadId = 0, HINSTANCE instance = ::GetModuleHandle( NULL ) ){
		Reconstruct( this );
		this->setHandle( ::SetWindowsHookEx(
			HookType,
			&HookProc,
			instance,
			threadId
		) );
		return this->isCreated();
	}
};

// ============================================================
class HookKeyboard : public HookObj< WH_KEYBOARD_LL >{

protected:
	Array< BYTE, 256 > _keyboardState;

	void onHook( HookOpt& opt ){

		if( WM_KEYDOWN != opt._wParam )
			return;

		if( NULL == opt._lParam )
			return;

		KBDLLHOOKSTRUCT& hs = *(KBDLLHOOKSTRUCT*)opt._lParam;

		for( int i = 0 ; i < _keyboardState._maxCount; i++ )
			_keyboardState[i] = ( ::GetAsyncKeyState( i ) & 0xFF00 ) >> 8;

		WORD ascii = 0;
		::ToAscii( hs.vkCode, hs.scanCode, _keyboardState, &ascii, 0 );

		//putString( (TCHAR*)&ascii );
	}
};

// ============================================================
class HookMouse : public HookObj< WH_MOUSE_LL >{

protected:
	void onHook( HookOpt& opt ){

		if( WM_LBUTTONUP != opt._wParam && WM_RBUTTONUP != opt._wParam )
			return;

		if( NULL == opt._lParam )
			return;

		MSLLHOOKSTRUCT& hs = *(MSLLHOOKSTRUCT*)opt._lParam;
		MaxPathT text;
		::GetWindowText( ::GetForegroundWindow(), text, text._maxCount );
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
