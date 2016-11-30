// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_WINDOWING_LISTVIEW_H__
#define __WIZ_WINDOWING_LISTVIEW_H__

#include "Wiz/Windowing/Control.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Windowing{

// ============================================================
template< typename T, typename E >
struct LvElem : public Structure< E >{

	typedef T This;

protected:
	inline This& addMask( unsigned int v ){
		(*this)->mask |= v;
		return (This&)*this;
	}

	inline This& subMask( unsigned int v ){
		(*this)->mask &= ~v;
		return (This&)*this;
	}

	inline This& setRow( int v ){
		(*this)->iItem = v;
		return (This&)*this;
	}

	inline This& setCol( int v ){
		(*this)->iSubItem = v;
		return (This&)*this;
	}

	inline This& setText( TCHAR* v ){
		(*this)->pszText = v;
		return (This&)*this;
	}

	inline This& setImage( int v ){
		(*this)->iImage = v;
		return (This&)*this;
	}
};

// ============================================================
struct LvColumn : public LvElem< LvColumn, LVCOLUMN >{

	inline LvColumn(){}

	inline LvColumn( int idxCol ){
		this->buildCol( idxCol );
	}

	inline LvColumn( int idxCol, TCHAR* text, int witdh, int format = LVCFMT_LEFT ){
		this->buildCol( idxCol ).buildText( text ).buildWidth( witdh ).buildFormat( format );
	}

	inline This& buildCol( int v = 0 ){
		return this->setCol( v ).addMask( LVCF_SUBITEM );
	}

	inline This& buildText( TCHAR* v = NULL ){
		return this->setText( v ).addMask( LVCF_TEXT );
	}

	inline This& buildWidth( int v = 0 ){
		(*this)->cx = v;
		return this->addMask( LVCF_WIDTH );
	}

	inline This& buildFormat( int v = 0 ){
		(*this)->fmt = v;
		return this->addMask( LVCF_FMT );
	}
};

// ============================================================
struct LvItem : public LvElem< LvItem, LVITEM >{

	inline LvItem(){}

	inline LvItem( int idxRow, int idxCol = 0, TCHAR* text = NULL ){
		this->setRow( idxRow ).setCol( idxCol ).buildText( text );
	}

	inline This& buildText( TCHAR* v = NULL ){
		return this->setText( v ).addMask( LVIF_TEXT );
	}

	inline This& buildImage( int v = 0 ){
		return this->setImage( v ).addMask( LVIF_IMAGE );
	}

	inline This& buildParam( LPARAM v = 0 ){
		(*this)->lParam = v;
		return this->addMask( LVIF_PARAM );
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

	inline bool create( WndOpt& opt ){
		return this->createWindowEx( opt.buildControl( _T("SysListView32") ) );
	}

	bool addColumn( int indexCol, PTCHAR name, int width = 100, int format = LVCFMT_LEFT ){

		if( -1 == ListView_InsertColumn( *this, indexCol, &LvColumn( indexCol, name, width, format ) ) )
			return false;

		Column column;
		column._name << name;
		_columnList.insertNodeAt( indexCol, &column );
		return true;
	}

	inline ListView& addItem( int indexRow ){
		ListView_InsertItem( *this, &LvItem( indexRow ) );
		return *this;
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

	inline LvItem& getItem( LvItem& item ){
		ListView_GetItem( *this, &item );
		return item;
	}

	LPARAM getItemParam( int indexRow ){
		return this->getItem( LvItem( indexRow ).buildParam() )->lParam;
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

	inline bool setColumn( LvColumn& column ){
		return( 0 != ListView_SetColumn( *this, column->iSubItem, &column ) );
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

	inline ListView& setItem( LvItem& item ){
		ListView_SetItem( *this, &item );
		return *this;
	}

	inline void setItemState( int indexRow, UINT mask ){
		ListView_SetItemState( *this, indexRow, mask, mask );
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
		_focusedEdit.create( WndOpt( target ) );
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
		_focusedComboBox.create( WndOpt( target ) );
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
		_focusedDateTimePicker.create( WndOpt( target ) );
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

}}
// ===================================Namespace Tail==========================================

#endif
