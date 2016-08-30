// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_WINDOWING_CONTROL_H__
#define __WIZ_WINDOWING_CONTROL_H__

#include "Wiz/Windowing/Forward.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Windowing{

// ============================================================
class ImageList : public HandleT< HIMAGELIST, NULL >{
public:
	inline bool loadBitmap( PTCHAR resourceName, UINT eachWidth, UINT growNumber = 1 ){
		this->setHandle( ImageList_LoadBitmap(
			::GetModuleHandle( NULL ),
			resourceName,
			eachWidth,
			growNumber,
			CLR_NONE
		) );
		return this->isCreated();
	}
};

// ============================================================
// @Brief: Base of Control
// ============================================================
class Control : public Window{

	WNDPROC _defaultWindowProcedure;

public:
	inline void setControlProcedure(){
		_defaultWindowProcedure = this->getWindowProcedure();
		this->setWindowProcedure( &Window::WindowProcedure );
	}

	virtual LRESULT __cdecl onDefault( UINT message, WPARAM wParam, LPARAM lParam ){
		return ::CallWindowProc( _defaultWindowProcedure, *this, message, wParam, lParam );
	}
};

// ============================================================
class Button : public Window{
public:
	inline bool create( Window* parent, Rect& rect, DWORD style = 0, DWORD styleEx = 0 ){
		return this->createControl(
			_T("Button"),
			parent,
			rect,
			style,
			styleEx
		);
	}

	inline bool isChecked(){
		return( BST_CHECKED & Button_GetState( *this ) );
	}

	inline void setCheck( bool isChecked ){
		Button_SetCheck( *this, isChecked );
	}
};

// ============================================================
class ComboBox : public Window{
public:
	inline bool create( Window* parent, Rect& rect, DWORD style = 0, DWORD styleEx = 0 ){
		return this->createControl(
			_T("ComboBox"),
			parent,
			rect,
			style | CBS_DROPDOWNLIST | WS_VSCROLL,
			styleEx
		);
	}

	inline int getCount(){
		return ComboBox_GetCount( *this );
	}

	inline int getCurSel(){
		return ComboBox_GetCurSel( *this );
	}

	inline bool setCurSel( int index ){
		return( CB_ERR != ComboBox_SetCurSel( *this, index ) );
	}

	inline bool addString( PTCHAR string ){
		return( CB_ERR != ComboBox_AddString( *this, string ) );
	}

	inline bool getString( int index, PTCHAR string, UINT strLen ){
		UINT srcStrLen = ComboBox_GetLBTextLen( *this, index ) + 1;
		if( srcStrLen > strLen )
			return false;

		return( CB_ERR != ComboBox_GetLBText( *this, index, string ) );
	}

	inline bool getSelectedString( PTCHAR string, UINT strLen ){
		return this->getString( this->getCurSel(), string, strLen );
	}

	inline bool deleteString( int index ){
		return( CB_ERR != ComboBox_DeleteString( *this, index ) );
	}

	inline bool deleteStringAll(){
		return( CB_ERR != ComboBox_ResetContent( *this ) );
	}
};

// ============================================================
class DateTimePicker : public Window{
public:
	inline bool create( Window* parent, Rect& rect, DWORD style = 0, DWORD styleEx = 0 ){
		return this->createControl(
			_T("SysDateTimePick32"),
			parent,
			rect,
			style,
			styleEx
		);
	}

	inline bool getTime( SYSTEMTIME& time ){
		return( GDT_ERROR != DateTime_GetSystemtime( *this, &time ) );
	}

	inline bool setTime( SYSTEMTIME& time ){
		return( 0 != DateTime_SetSystemtime( *this, GDT_VALID, &time ) );
	}

	inline bool setFormat( PTCHAR format ){
		return( 0 != DateTime_SetFormat( *this, format ) );
	}
};

// ============================================================
class Edit : public Control{
public:
	enum{ _maxTextLen = 0x7FFFFFFE };

	inline bool create( Window* parent, Rect& rect, DWORD style = 0, DWORD styleEx = 0 ){
		if( !this->createControl(
			_T("Edit"),
			parent,
			rect,
			style | WS_BORDER | WS_TABSTOP | ES_LEFT,
			styleEx ) )
			return false;

		this->setLimit( _maxTextLen );
		return true;
	}

	inline void setLimit( int limitLen ){
		Edit_LimitText( *this, limitLen );
	}

	inline void setSelText( int start, int end ){
		Edit_SetSel( *this, start, end );
	}
};

// ============================================================
#ifndef _WIN32_WCE
class IpAddress : public Window{
public:
	inline bool create( Window* parent, Rect& rect, DWORD style = 0, DWORD styleEx = 0 ){
		return this->createControl(
			_T("SysIPAddress32"),
			parent,
			rect,
			style,
			styleEx
		);
	}

	inline DWORD getIp(){
		DWORD ip = 0;
		this->sendMessage( IPM_GETADDRESS, 0, (LPARAM)&ip );
		return ip;
	}

	inline void setIp( DWORD ip ){
		this->sendMessage( IPM_SETADDRESS, 0, ip );
	}
};
#endif

// ============================================================
class ListBox : public Window{
public:
	inline bool create( Window* parent, Rect& rect, DWORD style = 0, DWORD styleEx = 0 ){
		return this->createControl(
			_T("ListBox"),
			parent,
			rect,
			style,
			styleEx
		);
	}

	inline int getCount(){
		return this->sendMessage( LB_GETCOUNT, 0, 0 );
	}

	inline int getCurSel(){
		return this->sendMessage( LB_GETCURSEL, 0, 0 );
	}

	inline bool setCurSel( int index ){
		this->sendMessage( LB_SETCURSEL, index, 0 );
		return true;
	}

	inline bool addString( PTCHAR string ){
		return( LB_ERR != this->sendMessage( LB_ADDSTRING, 0, (LPARAM)string ) );
	}

	inline bool insertString( int index, PTCHAR string ){
		return( LB_ERR != this->sendMessage( LB_INSERTSTRING, index, (LPARAM)string ) );
	}

	inline bool getString( int index, PTCHAR string, UINT strLen ){
		UINT srcStrLen = this->sendMessage( LB_GETTEXTLEN, index, 0 ) + 1;
		if( srcStrLen > strLen )
			return false;

		return( LB_ERR != this->sendMessage( LB_GETTEXT, index, (LPARAM)string ) );
	}

	inline bool getSelectedString( PTCHAR string, UINT strLen ){
		return this->getString( this->getCurSel(), string, strLen );
	}

	inline bool deleteString( int index ){
		return( LB_ERR != this->sendMessage( LB_DELETESTRING, index, 0 ) );
	}

	inline bool deleteStringAll(){
		this->sendMessage( LB_RESETCONTENT, 0, 0 );
		return true;
	}

	inline bool scroll( int index ){
		return( LB_ERR != this->sendMessage( LB_SETCURSEL, index, 0 ) );
	}
};

// ============================================================
class Slider : public Window{
public:
	inline bool create( Window* parent, Rect& rect, DWORD style = 0, DWORD styleEx = 0 ){
		return this->createControl(
			_T("msctls_trackbar32"),
			parent,
			rect,
			style,
			styleEx
		);
	}

	void setPosition( UINT pos ){
		this->sendMessage( TBM_SETPOS, true, pos );
	}

	void setRange( UINT min, UINT max ){
		this->sendMessage( TBM_SETRANGEMIN, true, min );
		this->sendMessage( TBM_SETRANGEMAX, true, max );
	}

	void setSelectionRange( UINT start, UINT end ){
		this->sendMessage( TBM_SETSELSTART, true, start );
		this->sendMessage( TBM_SETSELEND, true, end );
	}

	void showThumb( bool enable ){
		if( enable )
			this->removeStyle( TBS_NOTHUMB );
		else
			this->insertStyle( TBS_NOTHUMB );
	}
};

// ============================================================
class Static : public Window{
public:
	inline bool create( Window* parent, Rect& rect, DWORD style = 0, DWORD styleEx = 0 ){
		return this->createControl(
			_T("Static"),
			parent,
			rect,
			style,
			styleEx
		);
	}

	void setBitmap( HBITMAP bitmap ){
		this->removeStyle( SS_ICON );
		this->insertStyle( SS_BITMAP );
		this->sendMessage( STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bitmap );
	}

	void setIcon( HICON icon ){
		this->removeStyle( SS_BITMAP );
		this->insertStyle( SS_ICON );
		this->sendMessage( STM_SETIMAGE, IMAGE_ICON, (LPARAM)icon );
	}
};

// ============================================================
class Tab : public Window{

public:
	inline bool create( Window* parent, Rect& rect, DWORD style = 0, DWORD styleEx = 0 ){
		return this->createControl(
			_T("SysTabControl32"),
			parent,
			rect,
			style | TCS_TABS,
			styleEx
		);
	}

	bool addItem( int index, PTCHAR text, Window* window, Rect& rect ){

		if( NULL == window )
			return false;

		TCITEM tabItem = {0};
		tabItem.mask = TCIF_TEXT | TCIF_IMAGE;
		tabItem.pszText = text;
		tabItem.iImage = -1;
		TabCtrl_InsertItem( *this, index, &tabItem );

		window->setPosition( NULL, SWP_NOSIZE, rect );
		_children.insertNodeAfter( index - 1, window );
		return true;
	}

	inline bool deleteItemAll(){
		_children.removeNodeAll();
		return( 0 != TabCtrl_DeleteAllItems( *this ) );
	}

	inline int getSelectedTab(){
		return TabCtrl_GetCurSel( *this );
	}

	bool setSelectedTab( int index ){

		TabCtrl_SetCurSel( *this, index );

		Window** entry = _children.findEntryAt( index );
		if( NULL == entry || NULL == *entry )
			return false;

		this->hideSubWindowAll();
		(*entry)->showWindow( SW_SHOW );
		return true;
	}

	void hideSubWindowAll(){
		for( Children::Iterator iter( _children ); iter != NULL; ++iter ){
			Window** entry = iter.getCurEntry();
			if( NULL != *entry )
				(*entry)->showWindow( SW_HIDE );
		}
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
