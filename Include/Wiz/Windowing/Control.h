// ============================================================
// @Author	Blaine
// @Date	2015/03/04
// ============================================================
#ifndef __WIZ_WINDOWING_CONTROL_H__
#define __WIZ_WINDOWING_CONTROL_H__

#include "Forward.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Windowing{

// ============================================================
// @Brief	Base of Control
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
class ListView : public Control{

public:
	typedef struct Column{
		StringArray< TCHAR, 64 > _name;
		COLORREF _colorText;
		COLORREF _colorTextBk;
		Column(){
			_colorText = RGB( 0x00, 0x00, 0x00 );
			_colorTextBk = RGB( 0xFF, 0xFF, 0xFF );
		}
	};

protected:
	List< Column > _columnList;

public:
	LRESULT __cdecl onCustomDraw( WPARAM wParam, NMLVCUSTOMDRAW* customDraw ){

		if( customDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
			return CDRF_NOTIFYITEMDRAW;

		if( customDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT ){

			for( UINT i = 0; i < this->getCount(); i++ ){

				if( this->isItemSel( i ) && i == customDraw->nmcd.dwItemSpec ){
					customDraw->clrText = RGB( 0xFF, 0xFF, 0xFF );
					customDraw->clrTextBk = RGB( 0x85, 0xD1, 0xFF );
					return CDRF_NOTIFYSUBITEMDRAW;
				}
			}

			customDraw->clrText = RGB( 0x00, 0x00, 0x00 );
			customDraw->clrTextBk = RGB( 0xFF, 0xFF, 0xFF );
			return CDRF_NOTIFYSUBITEMDRAW;
		}

		if( customDraw->nmcd.dwDrawStage == ( CDDS_ITEMPREPAINT | CDDS_SUBITEM ) ){

			Column* column = this->getColumn( customDraw->iSubItem );

			if( column != NULL ){
				customDraw->clrText = column->_colorText;
				//customDraw->clrTextBk = column->_colorTextBk;
			}else{
				customDraw->clrText = RGB( 0x00, 0x00, 0x00 );
				//customDraw->clrTextBk = RGB( 0xFF, 0xFF, 0xFF );
			}
			return CDRF_NEWFONT;
		}

		return CDRF_DODEFAULT;
	}

	inline bool create( Window* parent, Rect& rect, DWORD style = 0, DWORD styleEx = 0 ){
		return this->createControl(
			_T("SysListView32"),
			parent,
			rect,
			style,
			styleEx
		);
	}

	bool addColumn( int indexCol, PTCHAR name, int width = 100, int format = LVCFMT_LEFT ){
		LVCOLUMN lvColumn = {0};
		lvColumn.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
		lvColumn.iSubItem = indexCol;
		lvColumn.pszText = name;
		lvColumn.cx = width;
		lvColumn.fmt = format;
		if( -1 == ListView_InsertColumn( *this, indexCol, &lvColumn ) )
			return false;

		Column column;
		column._name << name;
		_columnList.insertNodeAt( indexCol, &column );
		return true;
	}

	void addItem( int indexRow ){
		LVITEM item = {0};
		//item.mask = LVIF_STATE;
		//item.state = LVIS_SELECTED | LVIS_FOCUSED;
		//item.state = LVIS_FOCUSED;
		//item.stateMask = -1;
		item.iItem = indexRow;
		ListView_InsertItem( *this, &item );
	}

	bool deleteColumn( int indexCol ){

		if( !ListView_DeleteColumn( *this, indexCol ) )
			return false;

		_columnList.removeNodeAt( indexCol );
		return true;
	}

	inline bool deleteItem( int indexRow ){
		return( 0 != ListView_DeleteItem( *this, indexRow ) );
	}

	inline bool deleteItemAll(){
		return( 0 != ListView_DeleteAllItems( *this ) );
	}

	inline Column* getColumn( int indexCol ){
		return _columnList.findEntryAt( indexCol );
	}

	inline UINT getColumnCount(){
		return _columnList.getCurCount();
	}

	inline UINT getCount(){
		return ListView_GetItemCount( *this );
	}

	inline UINT getSelCount(){
		return ListView_GetSelectedCount( *this );
	}

	inline UINT getSelMark(){
		return ListView_GetSelectionMark( *this );
	}

	LPARAM getItemParam( int indexRow ){
		LVITEM item = {0};
		item.mask = LVIF_PARAM;
		item.iItem = indexRow;
		ListView_GetItem( *this, &item );
		return item.lParam;
	}

	inline bool getItemRect( int indexRow, int indexCol, Rect& rect ){
		return( 0 != ListView_GetSubItemRect( *this, indexRow, indexCol, LVIR_LABEL, &rect ) );
	}

	inline UINT getItemState( int indexRow, UINT mask = LVIS_SELECTED ){
		return ListView_GetItemState( *this, indexRow, mask );
	}

	inline void getItemText( int indexRow, int indexCol, PTCHAR string, UINT strLen ){
		ListView_GetItemText( *this, indexRow, indexCol, string, strLen );
	}

	inline int getItemTextAsInt( int indexRow, int indexCol ){
		MaxPath<> text;
		this->getItemText( indexRow, indexCol, text, text._maxCount );
		return ::_tstoi( text );
	}

	inline __int64 getItemTextAsInt64( int indexRow, int indexCol ){
		MaxPath<> text;
		this->getItemText( indexRow, indexCol, text, text._maxCount );
		return ::_tstoi64( text );
	}

	inline long getItemTextAsLong( int indexRow, int indexCol ){
		MaxPath<> text;
		this->getItemText( indexRow, indexCol, text, text._maxCount );
		return ::_tstol( text );
	}

#ifndef _WIN32_WCE
	inline double getItemTextAsDouble( int indexRow, int indexCol ){
		MaxPath<> text;
		this->getItemText( indexRow, indexCol, text, text._maxCount );
		return ::_tstof( text );
	}
#endif

	inline void getSelText( int indexCol, PTCHAR string, UINT strLen ){
		this->getItemText( this->getSelMark(), indexCol, string, strLen );
	}

	inline bool isItemSel( int indexRow ){
		return( this->getItemState( indexRow, LVIS_SELECTED ) == LVIS_SELECTED );
	}

	inline bool rowScroll( int indexRow ){
		Rect rect;
		this->getItemRect( 0, 0, rect );
		return( 0 != ListView_Scroll( *this, 0, ( rect->bottom - rect->top ) * indexRow ) );
	}

	bool setColumn( int indexCol, PTCHAR name, int width = 100 ){
		LVCOLUMN lvColumn = {0};
		lvColumn.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.iSubItem = indexCol;
		lvColumn.pszText = name;
		lvColumn.cx = width;
		return( 0 != ListView_SetColumn( *this, indexCol, &lvColumn ) );
	}

	bool setColumnColor( int indexCol, COLORREF colorText, COLORREF colorTextBk ){

		Column* column = this->getColumn( indexCol );
		if( NULL == column )
			return false;

		column->_colorText = colorText;
		column->_colorTextBk = colorTextBk;
		return true;
	}

	inline void setCurSel( int indexRow ){
		ListView_SetSelectionMark( *this, indexRow );
	}

	inline bool setImageList( HIMAGELIST imageList ){
		return( NULL != ListView_SetImageList( *this, imageList, LVSIL_SMALL ) );
	}

	void setItemImage( int indexRow, int indexImage ){
		LVITEM item = {0};
		item.mask = LVIF_IMAGE;
		item.iItem = indexRow;
		item.iImage = indexImage;
		ListView_SetItem( *this, &item );
	}

	void setItemParam( int indexRow, LPARAM lParam ){
		LVITEM item = {0};
		item.mask = LVIF_PARAM;
		item.iItem = indexRow;
		item.lParam = lParam;
		ListView_SetItem( *this, &item );
	}

	inline void setItemState( int indexRow, UINT mask ){
		ListView_SetItemState( *this, indexRow, mask, mask );
	}

	void setItemText( int indexRow, int indexCol, PTCHAR string ){
		LVITEM item = {0};
		item.mask = LVIF_TEXT;
		item.iItem = indexRow;
		item.iSubItem = indexCol;
		item.pszText = string;
		ListView_SetItem( *this, &item );
	}

	inline void setItemTextEx( int indexRow, int indexCol, PTCHAR format, ... ){
		Text1024<> text;
		VPRINTF( &text[0], text._maxCount, format );
		this->setItemText( indexRow, indexCol, text );
	}

	inline void setItemText( int indexRow, int indexCol, SYSTEMTIME& time ){
		Text32<> text;
		SystemTime::CopyTo( time, &text[0] );
		this->setItemTextEx( indexRow, indexCol, text );
	}

	inline void setLvStyleEx( DWORD lvStyle ){
		ListView_SetExtendedListViewStyleEx( *this, lvStyle, lvStyle );
	}
};

// ============================================================
class ListViewEx : public ListView{

protected:
	int _focusedIndexRow;
	int _focusedIndexCol;

public:
	Edit _focusedEdit;
	ComboBox _focusedComboBox;
	DateTimePicker _focusedDateTimePicker;

	inline ListViewEx(){
		_focusedIndexRow = 0;
		_focusedIndexCol = 0;
	}

	void createItemEdit( Window* target ){
		_focusedEdit.create( target, Rect() );
		_focusedEdit.setParent( this );
		_focusedEdit.setFont( this->getFont() );
	}

	void enableItemEdit( int indexRow, int indexCol, PTCHAR string ){
		Rect rect;
		_focusedIndexRow = indexRow;
		_focusedIndexCol = indexCol;
		this->getItemRect( _focusedIndexRow, _focusedIndexCol, rect );

		_focusedEdit.setPosition( HWND_TOP, SWP_SHOWWINDOW, rect );
		_focusedEdit.setText( string );
		_focusedEdit.setSelText( 0, -1 );
		_focusedEdit.setFocus();
	}

	void cancelItemEdit( int* indexRow, int* indexCol, PTCHAR string, UINT strLen ){
		_focusedEdit.showWindow( SW_HIDE );

		if( NULL != indexRow )
			*indexRow = _focusedIndexRow;
		if( NULL != indexCol )
			*indexCol = _focusedIndexCol;
		if( NULL != string )
			_focusedEdit.getText( string, strLen );
	}

	void createItemComboBox( Window* target ){
		_focusedComboBox.create( target, Rect() );
		_focusedComboBox.setParent( this );
		_focusedComboBox.setFont( this->getFont() );
	}

	void enableItemComboBox( int indexRow, int indexCol, int indexItem ){
		Rect rect;
		_focusedIndexRow = indexRow;
		_focusedIndexCol = indexCol;
		this->getItemRect( _focusedIndexRow, _focusedIndexCol, rect );

		_focusedComboBox.setPosition( HWND_TOP, SWP_SHOWWINDOW, rect );
		_focusedComboBox.setCurSel( indexItem );
		_focusedComboBox.setFocus();
	}

	void cancelItemComboBox( int* indexRow, int* indexCol, PTCHAR string, UINT strLen ){
		_focusedComboBox.showWindow( SW_HIDE );

		if( NULL != indexRow )
			*indexRow = _focusedIndexRow;
		if( NULL != indexCol )
			*indexCol = _focusedIndexCol;
		if( NULL != string )
			_focusedComboBox.getString( _focusedComboBox.getCurSel(), string, strLen );
	}

	void createItemDateTimePicker( Window* target ){
		_focusedDateTimePicker.create( target, Rect() );
		_focusedDateTimePicker.setParent( this );
		_focusedDateTimePicker.setFont( this->getFont() );
	}

	void enableItemDateTimePicker( int indexRow, int indexCol, SYSTEMTIME* systemTime ){
		Rect rect;
		_focusedIndexRow = indexRow;
		_focusedIndexCol = indexCol;
		this->getItemRect( _focusedIndexRow, _focusedIndexCol, rect );

		//TODO: Remove this.
		rect->bottom += 5;

		_focusedDateTimePicker.setPosition( HWND_TOP, SWP_SHOWWINDOW, rect );
		_focusedDateTimePicker.setTime( *systemTime );
		_focusedDateTimePicker.setFocus();
	}

	void cancelItemDateTimePicker( int* indexRow, int* indexCol, SYSTEMTIME* systemTime ){
		_focusedDateTimePicker.showWindow( SW_HIDE );

		if( NULL != indexRow )
			*indexRow = _focusedIndexRow;
		if( NULL != indexCol )
			*indexCol = _focusedIndexCol;
		if( NULL != systemTime )
			_focusedDateTimePicker.getTime( *systemTime );
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
