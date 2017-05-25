// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_WINDOWING_BASIC_H__
#define __WIZ_WINDOWING_BASIC_H__

#include "Wiz/Core/All.h"
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
namespace Wiz{ namespace Windowing{
	class WindowMap;
	class Window;
}}

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Windowing{

using namespace Wiz::Core;
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

	inline bool find( TCHAR* name, TCHAR* type, HMODULE module = NULL ){
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

		return this->setNotifyByPos();
	}

	Menu& createPopup(){
		this->setHandle( ::CreatePopupMenu() );
		if( !this->isCreated() )
			return *this;

		return this->setNotifyByPos();
	}

	Menu& setNotifyByPos(){
		MENUINFO menuInfo = {0};
		menuInfo.cbSize = sizeof(menuInfo);
		::GetMenuInfo( *this, &menuInfo );
		menuInfo.fMask = MIM_STYLE;
		menuInfo.dwStyle |= MNS_NOTIFYBYPOS;
		::SetMenuInfo( *this, &menuInfo );
		return *this;
	}

	inline Menu& insert( UINT index, UINT flags, UINT_PTR param = 0, TCHAR* string = NULL ){
		::InsertMenu( *this, index, flags, param, string );
		return *this;
	}

	inline Menu& insertItem( UINT index, TCHAR* string ){
		this->insert( index, MF_BYPOSITION | MF_STRING, 0, string );
		return *this;
	}

	inline Menu& insertMenu( UINT index, TCHAR* string, HMENU menu ){
		this->insert( index, MF_BYPOSITION | MF_POPUP, (UINT_PTR)menu, string );
		return *this;
	}

	inline Menu& insertSeparator( UINT index ){
		this->insert( index, MF_BYPOSITION | MF_SEPARATOR );
		return *this;
	}

	inline Menu& setChecked( UINT index, bool isChecked ){
		::CheckMenuItem( *this, index, MF_BYPOSITION | ( isChecked ? MF_CHECKED : MF_UNCHECKED ) );
		return *this;
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
struct WndOpt{
	typedef WndOpt This;

	Window* _parent;
	Rect _rect;
	DWORD _style;
	DWORD _styleEx;
	TCHAR* _className;
	TCHAR* _windowName;

	inline WndOpt(
		Window* parent = NULL,
		Rect& rect = Rect(),
		DWORD style = 0,
		DWORD styleEx = 0
	){
		MemoryReset( *this );
		this->setParent( parent ).setRect( rect ).setStyle( style ).setStyleEx( styleEx );
	}

	inline This& buildControl( TCHAR* className ){
		return this->setClassName( className ).addStyle( WS_VISIBLE | WS_CHILD );
	}

	inline This& addStyle( DWORD v ){
		_style |= v;
		return (This&)*this;
	}

	inline This& addStyleEx( DWORD v ){
		_styleEx |= v;
		return (This&)*this;
	}

	inline This& setParent( Window* v ){
		_parent = v;
		return (This&)*this;
	}

	inline This& setRect( Rect& v ){
		_rect = v;
		return (This&)*this;
	}

	inline This& setStyle( DWORD v ){
		_style = v;
		return (This&)*this;
	}

	inline This& setStyleEx( DWORD v ){
		_styleEx = v;
		return (This&)*this;
	}

	inline This& setClassName( TCHAR* v ){
		_className = v;
		return (This&)*this;
	}

	inline This& setWindowName( TCHAR* v ){
		_windowName = v;
		return (This&)*this;
	}
};

// ============================================================
struct MsgOpt{
	typedef MsgOpt This;

	UINT _message;
	WPARAM _wParam;
	LPARAM _lParam;

	inline MsgOpt( UINT message = 0, WPARAM wParam = 0, LPARAM lParam = 0 ){
		this->setMessage( message ).setWParam( wParam ).setLParam( lParam );
	}

	inline This& setMessage( UINT v ){
		_message = v;
		return (This&)*this;
	}

	inline This& setWParam( WPARAM v ){
		_wParam = v;
		return (This&)*this;
	}

	inline This& setLParam( LPARAM v ){
		_lParam = v;
		return (This&)*this;
	}
};

// ============================================================
class Window : public HandleT< HWND, NULL >{

public:
	typedef List< Window* > Children;

protected:
	Window* _parent;
	Window* _target;
	Children _children;

public:
	inline Window(){
		this->setParent( NULL );
		this->setTarget( NULL );
	}

	inline ~Window(){
		WindowMap::CreateInstance()->remove( *this );
		if( this->isCreated() )
			this->destroyWindow();
	}

	static LRESULT __stdcall WindowProcedure( HWND window, UINT message, WPARAM wParam, LPARAM lParam ){
		MsgOpt opt( message, wParam, lParam );
		Window* wnd = WindowMap::CreateInstance()->find( window );
		if( NULL != wnd )
			return wnd->onMessage( opt );

		// If hWnd not found in Window Map.
		return Window::DefWndProcedure( window, opt );
	}

	static LRESULT __stdcall DefWndProcedure( HWND window, MsgOpt& opt ){
		return ::DefWindowProc( window, opt._message, opt._wParam, opt._lParam );
	}

	static BOOL __stdcall EnumChildProcedure( HWND child, LPARAM lParam ){
		::SendMessage( child, WM_SETFONT, lParam, 0 );
		return true;
	}

	virtual LRESULT __cdecl onMessage( MsgOpt& opt ){
		return this->onDefault( opt );
	}

	virtual LRESULT __cdecl onDefault( MsgOpt& opt ){
		return Window::DefWndProcedure( *this, opt );
	}

	inline bool setHandle( HWND window ){
		this->HandleT::setHandle( window );
		return WindowMap::CreateInstance()->insertV( *this, this );
	}

	bool createWindowEx( WndOpt& opt ){
		this->setParent( opt._parent );

		HWND hParent = this->GetNullHandle();
		if( this->getParent() != NULL )
			hParent = this->getParent()->getHandle();

		Rect& rect = opt._rect;
		this->setHandle( ::CreateWindowEx(
			opt._styleEx,
			opt._className,
			opt._windowName,
			opt._style,
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
		return (WNDPROC)::GetWindowLong( *this, GWLP_WNDPROC );
	}

	inline void setWindowProcedure( WNDPROC procedure ){
		::SetWindowLong( *this, GWLP_WNDPROC, (LONG)procedure );
	}

	inline void getText( TCHAR* text, UINT textLen ){
		::GetWindowText( *this, text, textLen );
	}

	inline int getTextAsInt(){
		MaxPathT text;
		this->getText( text, text._maxCount );
		return ::_tstoi( text );
	}

	inline long getTextAsLong(){
		MaxPathT text;
		this->getText( text, text._maxCount );
		return ::_tstol( text );
	}

	inline bool setText( TCHAR* text ){
		return( 0 != ::SetWindowText( *this, text ) );
	}

	inline void setTitleIcon( HICON icon ){
		this->sendMessage( WM_SETICON, ICON_BIG, (LPARAM)icon );
	}

#ifndef _WIN32_WCE
	inline void setCursor( HCURSOR cursor ){
		::SetClassLong( *this, GCLP_HCURSOR, (DWORD)cursor );
	}
#endif

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

	inline bool getClientRect( RECT& rect ){
		return( 0 != ::GetClientRect( *this, &rect ) );
	}

	inline bool getWindowRect( RECT& rect ){
		return( 0 != ::GetWindowRect( *this, &rect ) );
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

	inline bool setPosition( HWND windowAfter, UINT flags, Rect& rect = Rect() ){
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

	inline bool setRect( Rect& rect = Rect() ){
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

	inline int showMessage( UINT type, TCHAR* text ){
		return ::MessageBox( *this, text, _T("Notice"), type | MB_TOPMOST | MB_APPLMODAL | MB_ICONINFORMATION );
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

	inline Window* getTarget(){
		return _target;
	}

	inline void setTarget( Window* target ){
		_target = target;
	}

	void sendAllTarget( UINT message, WPARAM wParam = 0, LPARAM lParam = 0 ){
		for( Window* target = this; target != NULL; target = target->getTarget() )
			target->sendMessage( message, wParam, lParam );
	}

	void postAllTarget( UINT message, WPARAM wParam = 0, LPARAM lParam = 0 ){
		for( Window* target = this; target != NULL; target = target->getTarget() )
			target->postMessage( message, wParam, lParam );
	}

	inline void sendAllTargetString( TCHAR* text ){
		this->sendAllTarget( WM_STRING, (WPARAM)text );
	}

	inline void sendAllTargetString( UINT tag, TCHAR* text ){
		this->sendAllTarget( WM_STRING, (WPARAM)text, tag );
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
