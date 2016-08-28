// ============================================================
// @Author	Blaine
// @Date	2015/03/04
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
		class Target;
		struct MessageMap;
		struct MessageEntry;
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
// @Brief	Base Class for the All the
//			Class that can receive Message.
//			And MessageMap for The Message Map of Windows.
// ============================================================
//enum{ MessageMapNull = 0xFFFFFFFF };
//
//#ifndef DEFINE_MSGMAP
//	#define DEFINE_MSGMAP() \
//		virtual MessageMap* getMessageMap();\
//		static MessageMap _msgMap;\
//		static MessageEntry _msgEntries[];
//#endif
//
//#ifndef IMPLE_BASE_MSGMAP_BEGIN
//	#define IMPLE_BASE_MSGMAP_BEGIN( className ) \
//		MessageMap* className::getMessageMap(){return &className::_msgMap;}\
//		MessageMap className::_msgMap( NULL, &className::_msgEntries[0] );\
//		MessageEntry className::_msgEntries[] = {
//#endif
//
//#ifndef IMPLE_MSGMAP_BEGIN
//	#define IMPLE_MSGMAP_BEGIN( className, baseClassName ) \
//		MessageMap* className::getMessageMap(){return &className::_msgMap;}\
//		MessageMap className::_msgMap( &baseClassName::_msgMap, &className::_msgEntries[0] );\
//		MessageEntry className::_msgEntries[] = {
//#endif
//
//#ifndef IMPLE_MSGMAP_END
//	#define IMPLE_MSGMAP_END() \
//		OnMessage( (PtrHandler)NULL, MessageMapNull ) };
//#endif
//
//// Prototype of Method Pointer
//typedef DWORD(__cdecl Target::*PtrHandler)( ... );
//
//class Target : public HandleT< HWND, NULL >{
//public:
//	DEFINE_MSGMAP();
//};
//
//struct MessageEntry{
//	PtrHandler _handler;
//	UINT _message;
//	UINT _id;
//	UINT _code;
//
//	template< typename T >
//	inline MessageEntry( T handler, UINT message, UINT id, UINT code ) :
//		_handler( (PtrHandler)handler ),
//		_message( message ),
//		_id( id ),
//		_code( code )
//	{}
//};
//
//struct MessageMap{
//	MessageMap* _base;
//	MessageEntry* _entries;
//
//	inline MessageMap( MessageMap* base, MessageEntry* entries ) :
//		_base(base), _entries(entries)
//	{}
//
//	MessageEntry* find( UINT message, UINT id, UINT code ){
//
//		for( MessageEntry* entry = _entries ; entry->_message != MessageMapNull ; entry++ ){
//
//			if( entry->_message != message )
//				continue;
//
//			if( entry->_id != id && entry->_id != MessageMapNull )
//				continue;
//
//			if( entry->_code != code && entry->_code != MessageMapNull )
//				continue;
//
//			return entry;
//		}
//		return NULL;
//	}
//};

// ============================================================
//template< typename T >
//inline MessageEntry OnMessage( T handler, UINT message, UINT id = MessageMapNull, UINT code = MessageMapNull ){
//	return MessageEntry( handler, message, id, code );
//}
//
//template< typename T >
//inline MessageEntry OnCommand( T handler, UINT id, UINT code = MessageMapNull ){
//	return OnMessage( handler, WM_COMMAND, id, code );
//}
//
//template< typename T >
//inline MessageEntry OnNotify( T handler, UINT id, UINT code = MessageMapNull ){
//	return OnMessage( handler, WM_NOTIFY, id, code );
//}
//
//template< typename T >
//inline MessageEntry OnTimer( T handler, UINT id ){
//	return OnMessage( handler, WM_TIMER, id );
//}
//
//template< typename T >
//inline MessageEntry OnSize( T handler, UINT id ){
//	return OnMessage( handler, WM_SIZE, id );
//}

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

	virtual LRESULT __cdecl onMessage( UINT message, WPARAM wParam, LPARAM lParam ){
		UINT id = wParam;
		UINT code = lParam;
		HWND hwnd = NULL;

		if( message == WM_COMMAND ){
			id = LOWORD( wParam );
			code = HIWORD( wParam );
			hwnd = (HWND)lParam;
		}

		if( message == WM_NOTIFY ){
			NMHDR* nmhdr = (NMHDR*)lParam;
			id = nmhdr->idFrom;
			code = nmhdr->code;
			hwnd = nmhdr->hwndFrom;
		}

		HandlerCtx* entry = _handlerSet.find( message, id, hwnd, code );
		if( NULL != entry ){
			PtrHandler handler = entry->_handler;
			return ( this->*handler )( wParam, lParam );
		}

		// Handle Remain Message
		//return this->onDefault( message, wParam, lParam );
		return this->Window::onMessage( message, wParam, lParam );
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
	bool createFrame(
		PTCHAR name,
		DWORD style = 0,
		DWORD styleEx = 0,
		Rect& rect = Rect( 0, 0, 0, 0 )
	){
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

		if( !this->createWindowEx( name, name, style, styleEx, NULL, rect ) )
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
		return wnd->onMessage( message, wParam, lParam );
	}

	virtual LRESULT __cdecl onDefault( UINT message, WPARAM wParam, LPARAM lParam ){
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

	bool loadDialog( PTCHAR resourceName, Window* parent ){

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

	void createTray( UINT iconId, PTCHAR tip ){
		MemoryReset( _notifyIconData );
		_notifyIconData.cbSize = sizeof( _notifyIconData );
		_notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		_notifyIconData.uID = iconId;
		_notifyIconData.hIcon = ::LoadIcon( ::GetModuleHandle( NULL ), MAKEINTRESOURCE( iconId ) );
		_notifyIconData.hWnd = this->getHandle();
		_notifyIconData.uCallbackMessage = WM_TRAY;
		Copy( &_notifyIconData.szTip[0], tip );

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
#ifndef _WIN32_WCE
class OpenFileName : public Structure< OPENFILENAME >{

public:
	MaxPath<> _fileName;

	inline OpenFileName(){
		MemoryReset( *this );
		_fileName.clear();
	}

	bool open(){
		Entry& entry = *this;
		entry.lStructSize = _entrySize;
		entry.hwndOwner = NULL;
		entry.lpstrFile = _fileName;
		// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
		// use the contents of szFile to initialize itself.
		entry.lpstrFile[0] = '\0';
		entry.nMaxFile = _fileName._totalSize;
		//entry.lpstrFilter = _T("Executable File\0*.EXE;*.SCR\0");
		entry.nFilterIndex = 1;
		entry.lpstrFileTitle = NULL;
		entry.nMaxFileTitle = 0;
		entry.lpstrInitialDir = NULL;
		entry.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NODEREFERENCELINKS | OFN_NOCHANGEDIR;
		//entry.Flags = OFN_EXPLORER;
		return( 0 != ::GetOpenFileName( &entry ) );
	}

	bool save(){
		Entry& entry = *this;
		entry.lStructSize = _entrySize;
		entry.hwndOwner = NULL;
		entry.lpstrFile = _fileName;
		// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
		// use the contents of szFile to initialize itself.
		entry.lpstrFile[0] = '\0';
		entry.nMaxFile = _fileName._totalSize;
		//entry.lpstrFilter = _T("Executable File\0*.EXE;*.SCR\0");
		entry.nFilterIndex = 1;
		entry.lpstrFileTitle = NULL;
		entry.nMaxFileTitle = 0;
		entry.lpstrInitialDir = NULL;
		entry.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
		return( 0 != ::GetSaveFileName( &entry ) );
	}
};
#endif

}}
// ===================================Namespace Tail==========================================

#endif
