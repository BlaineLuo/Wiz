// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_WINDOWING_FORWARD_H__
#define __WIZ_WINDOWING_FORWARD_H__

#include "Wiz/Time.h"
#include "Wiz/WmDef.h"
#include <WindowsX.h>
#include <CommCtrl.h>

#ifndef _WIN32_WCE
	#pragma comment( lib, "ComCtl32" )
	#pragma comment( linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

// ============================================================
// Forward Declaration
// ============================================================
namespace Wiz{
	namespace Windowing{
		class WindowMap;
		class Window;
	}
}

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Windowing{

using namespace Wiz::Time;

// ============================================================
struct Rect : public Structure< RECT >{

	inline Rect(){
	}

	inline Rect( int x, int y, int w, int h ){
		(*this)->left = x;
		(*this)->right = x + w;
		(*this)->top = y;
		(*this)->bottom = y + h;
	}

	inline unsigned int getW(){
		return (*this)->right - (*this)->left;
	}

	inline unsigned int getH(){
		return (*this)->bottom - (*this)->top;
	}
};

// ============================================================
struct Point{
	int _x;
	int _y;

	inline Point(){
	}

	inline Point( int x, int y ){
		_x = x;
		_y = y;
	}

	template< typename T >
	inline Point& operator <<( T v ){
		_x = v;
		_y = v;
		return *this;
	}

	inline Point& operator +=( Point& v ){
		_x += v._x;
		_y += v._y;
		return *this;
	}
};

// ============================================================
class Resource : public HandleT< HRSRC, NULL >{

	HMODULE _module;

public:
	inline Resource() : _module(NULL)
	{}

	inline bool find( PTCHAR name, PTCHAR type, HMODULE module = NULL ){
		_module = module;
		this->setHandle( ::FindResource( _module, name, type ) );
		return this->isCreated();
	}

	bool load( Buffer& buffer ){

		if( this->getSize() == 0 )
			return false;

		PVOID resource = ::LockResource( ::LoadResource( _module, *this ) );
		if( NULL == resource )
			return false;

		buffer.createBuffer( this->getSize() );
		buffer.copyFrom( resource, buffer.getSize() );
		return true;
	}

	inline UINT getSize(){
		return ::SizeofResource( _module, *this );
	}
};

// ============================================================
#ifndef _WIN32_WCE
class Menu : public HandleT< HMENU, NULL >{

public:
	inline ~Menu(){
		if( this->isCreated() )
			::DestroyMenu( *this );
	}

	Menu& createMenu(){
		this->setHandle( ::CreateMenu() );
		if( !this->isCreated() )
			return *this;

		this->setNotifyByPos();
		return *this;
	}

	Menu& createPopup(){
		this->setHandle( ::CreatePopupMenu() );
		if( !this->isCreated() )
			return *this;

		this->setNotifyByPos();
		return *this;
	}

	void setNotifyByPos(){
		MENUINFO menuInfo = {0};
		menuInfo.cbSize = sizeof(menuInfo);
		::GetMenuInfo( *this, &menuInfo );
		menuInfo.fMask = MIM_STYLE;
		menuInfo.dwStyle |= MNS_NOTIFYBYPOS;
		::SetMenuInfo( *this, &menuInfo );
	}

	inline bool insert( UINT index, UINT flags, UINT_PTR param = 0, PTCHAR string = NULL ){
		return( 0 != ::InsertMenu( *this, index, flags, param, string ) );
	}

	inline bool insertItem( UINT index, PTCHAR string ){
		return this->insert( index, MF_BYPOSITION | MF_STRING, 0, string );
	}

	inline bool insertMenu( UINT index, PTCHAR string, Menu& menu ){
		return this->insert( index, MF_BYPOSITION | MF_POPUP, (UINT_PTR)menu.getHandle(), string );
	}

	inline bool insertSeparator( UINT index ){
		return this->insert( index, MF_BYPOSITION | MF_SEPARATOR );
	}

	inline void setChecked( UINT index, bool isChecked ){
		::CheckMenuItem( *this, index, MF_BYPOSITION | ( isChecked ? MF_CHECKED : MF_UNCHECKED ) );
	}

	inline bool track( HWND parent, POINT& point, UINT flags = TPM_LEFTALIGN | TPM_RIGHTBUTTON ){
		return( 0 != ::TrackPopupMenu( *this, flags, point.x, point.y, 0, parent, NULL ) );
	}
};
#endif

// ============================================================
class WindowMap : public PoolMap< HWND, NULL, Window*, 4096 >{

	DEFINE_SINGLE_EX( WindowMap, ;, ; );

public:
	inline Entry find( Key key ){
		Node* node = this->PoolMap::find( key );
		if( NULL == node )
			return NULL;
		return node->_entry;
	}
};

// ============================================================
class Window : public HandleT< HWND, NULL >{

public:
	typedef List< Window* > Children;

protected:
	Window* _parent;
	Window* _nextTarget;
	Children _children;

public:
	inline Window(){
		this->setParent( NULL );
		this->setNextTarget( NULL );
	}

	inline ~Window(){
		WindowMap::CreateInstance()->remove( *this );
		if( this->isCreated() )
			this->destroyWindow();
	}

	static LRESULT __stdcall WindowProcedure( HWND window, UINT message, WPARAM wParam, LPARAM lParam ){

		Window* wnd = WindowMap::CreateInstance()->find( window );
		if( NULL != wnd )
			return wnd->onMessage( message, wParam, lParam );

		// If hWnd not found in Window Map.
		return Window::DefWndProcedure( window, message, wParam, lParam );
	}

	static LRESULT __stdcall DefWndProcedure( HWND window, UINT message, WPARAM wParam, LPARAM lParam ){
		return ::DefWindowProc( window, message, wParam, lParam );
	}

	static BOOL __stdcall EnumChildProcedure( HWND child, LPARAM lParam ){
		::SendMessage( child, WM_SETFONT, lParam, 0 );
		return true;
	}

	virtual LRESULT __cdecl onMessage( UINT message, WPARAM wParam, LPARAM lParam ){
		UINT id = wParam;
		UINT code = lParam;

		// Handle WM_COMMAND Message
		if( message == WM_COMMAND ){
			id = LOWORD( wParam );
			code = HIWORD( wParam );
		}

		// Handle WM_NOTIFY Message
		if( message == WM_NOTIFY ){
			NMHDR* nmhdr = (NMHDR*)lParam;
			id = nmhdr->idFrom;
			code = nmhdr->code;
		}

		// Handle the Message defined in the MsgMap.
		//for( MessageMap* msgMap = this->getMessageMap(); msgMap != NULL; msgMap = msgMap->_base ){

		//	MessageEntry* entry = msgMap->find( message, id, code );
		//	if( entry == NULL )
		//		continue;

		//	PtrHandler handler = entry->_handler;
		//	return ( this->*handler )( wParam, lParam );
		//}

		// Handle Remain Message
		return this->onDefault( message, wParam, lParam );
	}

	virtual LRESULT __cdecl onDefault( UINT message, WPARAM wParam, LPARAM lParam ){
		return Window::DefWndProcedure( *this, message, wParam, lParam );
	}

	inline bool setHandle( HWND window ){
		this->HandleT::setHandle( window );
		return WindowMap::CreateInstance()->insertV( *this, this );
	}

	bool createWindowEx(
		PTCHAR className,
		PTCHAR windowName,
		DWORD style,
		DWORD styleEx,
		Window* parent,
		Rect& rect = Rect( 0, 0, 0, 0 )
	){
		this->setParent( parent );

		HWND hParent = this->GetNullHandle();
		if( this->getParent() != NULL )
			hParent = this->getParent()->getHandle();

		this->setHandle( ::CreateWindowEx(
			styleEx,
			className,
			windowName,
			style,
			rect->left,
			rect->top,
			rect->right - rect->left,
			rect->bottom - rect->top,
			hParent,
			NULL,
			::GetModuleHandle( NULL ),
			NULL
		) );
		return this->isCreated();
	}

	inline bool createControl(
		PTCHAR className,
		Window* parent,
		Rect& rect,
		DWORD style = 0,
		DWORD styleEx = 0
	){
		return this->createWindowEx( className, NULL, style | WS_VISIBLE | WS_CHILD, styleEx, parent, rect );
	}

	inline bool destroyWindow(){
		return( 0 != ::DestroyWindow( *this ) );
	}

	inline bool enableWindow( bool enable ){
		return( 0 != ::EnableWindow( *this, enable ) );
	}

	inline bool showWindow( int cmdShow ){
		return( 0 != ::ShowWindow( *this, cmdShow ) );
	}

	inline void insertStyle( DWORD s ){
		this->setStyle( this->getStyle() | s );
	}

	inline void removeStyle( DWORD s ){
		this->setStyle( this->getStyle() & (~s) );
	}

	inline DWORD getStyle(){
		return ::GetWindowLong( *this, GWL_STYLE );
	}

	inline void setStyle( DWORD s ){
		::SetWindowLong( *this, GWL_STYLE, s );
	}

	inline void insertStyleEx( DWORD s ){
		this->setStyleEx( this->getStyleEx() | s );
	}

	inline void removeStyleEx( DWORD s ){
		this->setStyleEx( this->getStyleEx() & (~s) );
	}

	inline DWORD getStyleEx(){
		return ::GetWindowLong( *this, GWL_EXSTYLE );
	}

	inline void setStyleEx( DWORD s ){
		::SetWindowLong( *this, GWL_EXSTYLE, s );
	}

	inline WNDPROC getWindowProcedure(){
		return (WNDPROC)::GetWindowLong( *this, GWL_WNDPROC );
	}

	inline void setWindowProcedure( WNDPROC procedure ){
		::SetWindowLong( *this, GWL_WNDPROC, (LONG)procedure );
	}

	inline void getText( PTCHAR text, UINT textLen ){
		::GetWindowText( *this, text, textLen );
	}

	inline int getTextAsInt(){
		MaxPath<> text;
		this->getText( text, text._maxCount );
		return ::_tstoi( text );
	}

	inline long getTextAsLong(){
		MaxPath<> text;
		this->getText( text, text._maxCount );
		return ::_tstol( text );
	}

	inline bool setText( PTCHAR text ){
		return( 0 != ::SetWindowText( *this, text ) );
	}

	inline bool setTextEx( PTCHAR format, ... ){
		Text1024<> text;
		VPRINTF( &text[0], text._maxCount, format );
		return this->setText( text );
	}

	inline void setTitleIcon( HICON icon ){
		this->sendMessage( WM_SETICON, ICON_BIG, (LPARAM)icon );
	}

	inline void setCursor( HCURSOR cursor ){
		::SetClassLong( *this, GCL_HCURSOR, (DWORD)cursor );
	}

	inline void setFocus(){
		::SetFocus( *this );
	}

	inline HFONT getFont(){
		return (HFONT)this->sendMessage( WM_GETFONT, 0, 0 );
	}

	inline void setFont( HFONT font ){
		this->sendMessage( WM_SETFONT, (WPARAM)font, 0 );
	}

#ifndef _WIN32_WCE
	inline void setFontAll( HFONT font ){
		this->setFont( font );
		::EnumChildWindows( *this, &Window::EnumChildProcedure, (LPARAM)font );
	}
#endif

	inline bool getClientRect( RECT& r ){
		return( 0 != ::GetClientRect( *this, &r ) );
	}

	inline bool getWindowRect( RECT& r ){
		return( 0 != ::GetWindowRect( *this, &r ) );
	}

	inline Window* getParent(){
		return _parent;
	}

	bool setParent( Window* parent ){

		_parent = parent;

		if( !this->isCreated() )
			return true;

		HWND hParent = this->GetNullHandle();
		if( this->getParent() != NULL )
			hParent = this->getParent()->getHandle();

		::SetParent( *this, hParent );
		return true;
	}

	inline bool addChild( Window& child ){
		return _children.insertNodeAtTail( &child );
	}

	inline bool setPosition( HWND windowAfter, UINT flags, Rect& rect = Rect( 0, 0, 0, 0 ) ){
		return( 0 != ::SetWindowPos(
			*this,
			windowAfter,
			rect->left,
			rect->top,
			rect->right - rect->left,
			rect->bottom - rect->top,
			flags
		) );
	}

	inline bool setRect( Rect& rect = Rect( 0, 0, 0, 0 ) ){
		return this->setPosition( NULL, SWP_NOZORDER, rect );
	}

	inline bool setPos( int x, int y ){
		return this->setPosition( NULL, SWP_NOSIZE, Rect( x, y, 0, 0 ) );
	}

	inline bool setSize( int w, int h ){
		return this->setPosition( NULL, SWP_NOMOVE, Rect( 0, 0, w, h ) );
	}

#ifndef _WIN32_WCE
	// Note: Do not used for WS_CHILE Window Style, neither CS_OWNDC or CS_CLASSDC Class Style.
	bool setOpacity( BYTE opacity ){
		this->setStyleEx( this->getStyleEx() | WS_EX_LAYERED );
		return( 0 != ::SetLayeredWindowAttributes( *this, 0, opacity, LWA_ALPHA ) );
	}
#endif

	inline int showMessage( UINT type, PTCHAR format, ... ){
		Text1024<> text;
		VPRINTF( &text[0], text._maxCount, format );
		return ::MessageBox( *this, text, _T("Notice"), type | MB_TOPMOST | MB_APPLMODAL | MB_ICONINFORMATION );
		//return MessageBox( *this, text, _T("Notice"), MB_OKCANCEL | MB_TOPMOST | MB_APPLMODAL | MB_ICONQUESTION );
	}

	inline LRESULT sendMessage( UINT message, WPARAM wParam, LPARAM lParam ){
		return ::SendMessage( *this, message, wParam, lParam );
	}

	inline bool postMessage( UINT message, WPARAM wParam, LPARAM lParam ){
		return( 0 != ::PostMessage( *this, message, wParam, lParam ) );
	}

	inline bool setTimer( UINT_PTR id, UINT elapseMilliseconds ){
		return( 0 != ::SetTimer( *this, id, elapseMilliseconds, NULL ) );
	}

	inline bool killTimer( UINT_PTR id ){
		return( 0 != ::KillTimer( *this, id ) );
	}

	inline Window* getNextTarget(){
		return _nextTarget;
	}

	inline void setNextTarget( Window* nextTarget ){
		_nextTarget = nextTarget;
	}

	void sendAllTarget( UINT message, WPARAM wParam = 0, LPARAM lParam = 0 ){
		for( Window* target = this; target != NULL; target = target->getNextTarget() )
			target->sendMessage( message, wParam, lParam );
	}

	void postAllTarget( UINT message, WPARAM wParam = 0, LPARAM lParam = 0 ){
		for( Window* target = this; target != NULL; target = target->getNextTarget() )
			target->postMessage( message, wParam, lParam );
	}

	inline void sendAllTargetString( PTCHAR format, ... ){
		Text1024<> text;
		VPRINTF( &text[0], text._maxCount, format );
		this->sendAllTarget( WM_STRING, (WPARAM)&text );
	}

	inline void sendAllTargetString( UINT tag, PTCHAR format, ... ){
		Text1024<> text;
		VPRINTF( &text[0], text._maxCount, format );
		this->sendAllTarget( WM_STRING, (WPARAM)&text, tag );
	}

	inline void sendAllTargetError( PTCHAR fileName, DWORD lineNum, DWORD errorCode = ::GetLastError() ){
		this->sendAllTargetString(
			_T("File=%s, Line=%d, ECode=%d, %s."),
			fileName,
			lineNum,
			errorCode,
			ErrorString().getErrorString( errorCode ).getString()
		);
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
