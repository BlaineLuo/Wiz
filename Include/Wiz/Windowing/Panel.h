// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_WINDOWING_PANEL_H__
#define __WIZ_WINDOWING_PANEL_H__

#include "Wiz/Windowing/Basic.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Windowing{

// ============================================================
enum{ MessageMapNull = 0xFFFFFFFF };

// Prototype of Method Pointer
typedef DWORD(__cdecl Window::*PtrHandler)( ... );

struct HandlerCtx{
	PtrHandler _handler;
	UINT _message;
	UINT _id;
	HWND _hwnd;
	UINT _code;
};

template< unsigned int MaxCount >
struct HandlerSetT : public LimitedContainer< HandlerCtx, MaxCount >{

	Entry* find( UINT message, UINT id, HWND hwnd, UINT code ){
		for( int i = 0; i < _maxCount; i++ ){
			Entry& entry = (*this)[i];
			if( NULL == entry._handler )
				return NULL;

			if( entry._message != message && entry._message != MessageMapNull )
				continue;

			if( NULL == hwnd ){
				if( entry._id != id && entry._id != MessageMapNull )
					continue;
			}else{
				if( entry._hwnd != hwnd )
					continue;
			}

			if( entry._code != code && entry._code != MessageMapNull )
				continue;

			return &entry;
		}
		return NULL;
	}

	bool insert( Entry& entry ){
		HandlerCtx* found = this->find( entry._message, entry._id, entry._hwnd, entry._code );
		if( NULL != found ){
			*found = entry;
			return true;
		}

		(*this)[ this->getCurCount() ] = entry;
		this->incCurCount();
		return true;
	}
};

// ============================================================
class Panel : public Window{
protected:
	typedef HandlerSetT< 64 > HandlerSet;
	HandlerSet _handlerSet;

	template< typename T >
	inline bool onMessage( T handler, UINT message, UINT id = MessageMapNull, HWND hwnd = NULL, UINT code = MessageMapNull ){
		HandlerCtx entry = { (PtrHandler)handler, message, id, hwnd, code };
		return _handlerSet.insert( entry );
	}

	template< typename T >
	inline bool onCommand( T handler, HWND hwnd, UINT code = MessageMapNull ){
		return this->onMessage( handler, WM_COMMAND, MessageMapNull, hwnd, code );
	}

	template< typename T >
	inline bool onNotify( T handler, HWND hwnd, UINT code = MessageMapNull ){
		return this->onMessage( handler, WM_NOTIFY, MessageMapNull, hwnd, code );
	}

	template< typename T >
	inline bool onTimer( T handler, UINT id ){
		return this->onMessage( handler, WM_TIMER, id );
	}

	template< typename T >
	inline bool onSize( T handler, UINT id ){
		return this->onMessage( handler, WM_SIZE, id );
	}

	virtual LRESULT __cdecl onMessage( MsgOpt& opt ){
		UINT id = opt._wParam;
		UINT code = opt._lParam;
		HWND hwnd = NULL;

		if( WM_COMMAND == opt._message ){
			id = LOWORD( opt._wParam );
			code = HIWORD( opt._wParam );
			hwnd = (HWND)opt._lParam;
		}

		if( WM_NOTIFY == opt._message ){
			NMHDR* nmhdr = (NMHDR*)opt._lParam;
			id = nmhdr->idFrom;
			code = nmhdr->code;
			hwnd = nmhdr->hwndFrom;
		}

		HandlerCtx* entry = _handlerSet.find( opt._message, id, hwnd, code );
		if( NULL != entry ){
			PtrHandler handler = entry->_handler;
			return ( this->*handler )( opt._wParam, opt._lParam );
		}

		// Handle Remain Message
		return this->onDefault( opt );
	}
};

// ============================================================
class Frame : public Panel{

protected:
	void __cdecl onClose(){
		::PostQuitMessage( 0 );
	}

	virtual bool onPreTranslateMessage( MSG& msg ){

		for( Children::Iterator iter( _children ); iter != NULL; ++iter ){
			Window** entry = iter.getCurEntry();
			if( NULL == *entry )
				continue;

			if( (*entry)->isCreated() && ::IsDialogMessage( **entry, &msg ) )
				return true;
		}
		return false;
	}

public:
	bool create( TCHAR* name ){
		WNDCLASS wndClass = {0};
		//wndClass.cbSize = sizeof( wndClass );
		wndClass.style = CS_DBLCLKS;
		wndClass.lpfnWndProc = Window::WindowProcedure;
		wndClass.lpszClassName = name;
		wndClass.lpszMenuName = NULL;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = ::GetModuleHandle( NULL );
		wndClass.hbrBackground = (HBRUSH)COLOR_BTNSHADOW;
		wndClass.hCursor = ::LoadCursor( NULL, IDC_ARROW );
		//wndClass.hIcon = ::LoadIcon( NULL, IDI_APPLICATION );
		//wndClass.hIconSm = ::LoadIcon( NULL, IDI_APPLICATION );
		if( !::RegisterClass( &wndClass ) )
			return false;

		if( !this->createWindowEx( WndOpt().setClassName( name ).setWindowName( name ) ) )
			return false;

		this->onMessage( &Frame::onClose, WM_CLOSE );
		return true;
	}

	int runMessageLoop(){

		// Run the message loop. It will run until GetMessage() returns 0.
		MSG msg = {0};
		while( ::GetMessage( &msg, NULL, 0, 0 ) ){

			// If already PreTranslate Message, then don't Dispatch Message.
			if( this->onPreTranslateMessage( msg ) )
				continue;

			// Translate virtual-key messages into character messages.
			::TranslateMessage( &msg );
			// Send message to WindowProcedure.
			::DispatchMessage( &msg );
		}

		// The program return-value is 0 - The value that PostQuitMessage() gave.
		return msg.wParam;
	}
};

// ============================================================
class Dialog : public Panel{

protected:
	DLGTEMPLATE _dlgTemplate;
	DLGITEMTEMPLATE _dlgItemTemplate;
	BOOL _isModal;

public:
	inline Dialog(){
		MemoryReset( _dlgTemplate );
		MemoryReset( _dlgItemTemplate );
		_isModal = false;
	}

	inline ~Dialog(){
		this->doModal( false );
	}

	static INT_PTR __stdcall DialogProcedure( HWND window, UINT message, WPARAM wParam, LPARAM lParam ){
		Window* wnd = WindowMap::CreateInstance()->find( window );
		if( NULL == wnd )
			return false;
		return wnd->onMessage( MsgOpt( message, wParam, lParam ) );
	}

	virtual LRESULT __cdecl onDefault( MsgOpt& opt ){
		return false;
	}

	virtual bool exchangeData(){
		return true;
	}

	void __cdecl onClose(){
		this->setPosition( HWND_TOP, SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE );
		this->doModal( false );
	}

	bool createIndirect( DLGTEMPLATE& dlgTemplate, Window* parent ){
		_dlgTemplate = dlgTemplate;
		this->setParent( parent );

		HWND hParent = this->GetNullHandle();
		if( this->getParent() != NULL )
			hParent = this->getParent()->getHandle();

		this->setHandle( ::CreateDialogIndirect(
			NULL,
			&_dlgTemplate,
			hParent,
			&Dialog::DialogProcedure
		) );

		if( !this->isCreated() )
			return false;

		this->onMessage( &Dialog::onClose, WM_CLOSE );
		return true;
	}

	bool loadDialog( TCHAR* resourceName, Window* parent ){

		this->setParent( parent );

		HWND hParent = this->GetNullHandle();
		if( this->getParent() != NULL )
			hParent = this->getParent()->getHandle();

		this->setHandle( ::CreateDialog(
			::GetModuleHandle( NULL ),
			resourceName,
			hParent,
			&Dialog::DialogProcedure
		) );
		return this->isCreated();
	}

	bool doModal( bool isModal ){

		if( this->getParent() == NULL )
			return false;

		if( _isModal == (BOOL)isModal )
			return true;

		_isModal = isModal;

		if( _isModal ){
			this->showWindow( SW_SHOW );
			this->getParent()->enableWindow( false );
		}else{
			this->getParent()->enableWindow( true );
			this->getParent()->setPosition( HWND_TOP, SWP_NOSIZE | SWP_NOMOVE );
		}
		return true;
	}

	inline HWND getDlgItem( UINT itemId ){
		return ::GetDlgItem( *this, itemId );
	}
};

// ============================================================
class TrayDialog : public Dialog{

protected:
	NOTIFYICONDATA _notifyIconData;

	template< typename T >
	inline bool onTray( T handler, UINT code ){
		return this->onMessage( handler, WM_TRAY, MessageMapNull, NULL, code );
	}

	void __cdecl onDoubleClick( WPARAM wParam, LPARAM lParam ){
		this->showWindow( SW_RESTORE );
		this->deleteTray();
		this->setPosition( HWND_TOPMOST, SWP_NOMOVE | SWP_NOSIZE );
		this->setPosition( HWND_NOTOPMOST, SWP_NOMOVE | SWP_NOSIZE );
	}

	void __cdecl onMinimized(){
		this->showWindow( SW_HIDE );
		this->insertTray();
	}

public:
	inline TrayDialog(){
		MemoryReset( _notifyIconData );
	}

	void createTray( UINT iconId, TCHAR* tip ){
		MemoryReset( _notifyIconData );
		_notifyIconData.cbSize = sizeof( _notifyIconData );
		_notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		_notifyIconData.uID = iconId;
		_notifyIconData.hIcon = ::LoadIcon( ::GetModuleHandle( NULL ), MAKEINTRESOURCE( iconId ) );
		_notifyIconData.hWnd = *this;
		_notifyIconData.uCallbackMessage = WM_TRAY;
		Copy( _notifyIconData.szTip, tip );

		this->onTray( &TrayDialog::onDoubleClick, WM_LBUTTONDBLCLK );
		this->onTray( &TrayDialog::onDoubleClick, WM_RBUTTONDBLCLK );
		this->onSize( &TrayDialog::onMinimized, SIZE_MINIMIZED );
	}

	inline bool insertTray(){
		return( 0 != ::Shell_NotifyIcon( NIM_ADD, &_notifyIconData ) );
	}

	inline bool modifyTray(){
		return( 0 != ::Shell_NotifyIcon( NIM_MODIFY, &_notifyIconData ) );
	}

	inline bool deleteTray(){
		return( 0 != ::Shell_NotifyIcon( NIM_DELETE, &_notifyIconData ) );
	}
};

// ============================================================
class Layouter{

protected:
	Dialog* _dialog;
	unsigned int _margin;
	int _x;
	int _y;

public:
	enum Align{
		Align_Left,
		Align_Right,
	};
	enum Style{
		Style_Horizontal,
		Style_Vertical,
	};

	inline Layouter( Dialog& dialog, unsigned int margin = 0 ){
		MemoryReset( *this );
		_dialog = &dialog;
		_margin = margin;
	}

	inline void addItem( int width, int height, Style style = Style_Horizontal ){
		;
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
