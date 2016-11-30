// ============================================================
// @Brief: GDI and Classes (for Win32)
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_GRAPHING_GDI_H__
#define __WIZ_GRAPHING_GDI_H__

#include "Wiz/System/Forward.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Graphing{ namespace Gdi{

using namespace Wiz::System;

// ============================================================
class BitmapHeader : BufferT< BITMAPINFO >{
public:
	bool create( UINT width, UINT height, UINT bitCount ){

		UINT colorTable[16] = {
			0x00000000,
			0x00000080,
			0x000000FF,
			0x00008000,
			0x00008080,
			0x0000FF00,
			0x0000FFFF,
			0x00800000,
			0x00800080,
			0x00808000,
			0x00808080,
			0x00C0C0C0,
			0x00FF0000,
			0x00FF00FF,
			0x00FFFF00,
			0x00FFFFFF
		};

		bitCount = 4;
		UINT colorNum = 1 << bitCount;
		UINT bufSize = sizeof(BITMAPINFO) + sizeof(RGBQUAD) * ( colorNum - 1 );

		this->createBuffer( bufSize );
		BITMAPINFO& bitmapInfo = *this->getHandle();
		BITMAPINFOHEADER& header = bitmapInfo.bmiHeader;

		header.biSize = sizeof(BITMAPINFO);
		header.biWidth = width;
		header.biHeight = height;
		header.biPlanes = 1;
		header.biCompression = BI_RGB;
		header.biBitCount = bitCount;
		header.biClrUsed = colorNum;
		header.biClrImportant = 0;
		header.biXPelsPerMeter = 1000000;
		header.biYPelsPerMeter = 1000000;
		//header.biSizeImage = ( ( uWidth + 7 ) & 0xFFFFFFF8 ) * uHeight / 8;
		header.biSizeImage = 0;

		for( UINT i = 0 ; i < colorNum ; i++ ){
			RGBQUAD& color = bitmapInfo.bmiColors[i];
			color.rgbRed = ( colorTable[ i ] & 0x00FF0000 ) >> 16;
			color.rgbGreen = ( colorTable[ i ] & 0x0000FF00 ) >> 8;
			color.rgbBlue = colorTable[ i ] & 0x000000FF;
			//color.rgbRed = ( i << 4 ) & 0xF0;
			//color.rgbGreen = ( i << 4 ) & 0xF0;
			//color.rgbBlue = ( i << 4 ) & 0xF0;
		}
		return true;
	}
};

// ============================================================
class DeviceCtx : public HandleT< HDC, NULL >{
public:
	inline bool setStretchBltMode( int mode ){
		return( ::SetStretchBltMode( *this, mode ) != 0 );
	}
};

// ============================================================
class DeviceCtxCommon : public DeviceCtx{

	HWND _hwnd;

public:
	inline DeviceCtxCommon(){
		_hwnd = NULL;
	}

	inline ~DeviceCtxCommon(){
		if( this->isCreated() )
			::ReleaseDC( _hwnd, *this );
	}

	inline bool create( HWND hwnd ){
		Reconstruct( this );
		this->setHandle( ::GetDC( _hwnd = hwnd ) );
		return this->isCreated();
	}
};

// ============================================================
class DeviceCtxCompatible : public DeviceCtx{
public:
	inline ~DeviceCtxCompatible(){
		if( this->isCreated() )
			::DeleteDC( *this );
	}

	inline bool create( HDC deviceCtx ){
		Reconstruct( this );
		this->setHandle( ::CreateCompatibleDC( deviceCtx ) );
		return this->isCreated();
	}
};

// ============================================================
// @Brief: GDI Object Base
// ============================================================
template< typename T = HGDIOBJ >
class GdiObject : public HandleT< T, NULL >{
public:
	inline ~GdiObject(){
		if( this->isCreated() )
			this->deleteObject();
	}

	inline bool selectObject( HDC deviceCtx, Handle object ){
		Reconstruct( this );
		this->setHandle( ::SelectObject( deviceCtx, object ) );
		return this->isCreated();
	}

	inline bool deleteObject(){
		return( 0 != ::DeleteObject( *this ) );
	}
};

// ============================================================
// @Brief: GDI Bitmap
// ============================================================
class Bitmap : public GdiObject< HBITMAP >{
public:
	inline bool createCompatibleBitmap( HDC deviceCtx, int width, int height ){
		Reconstruct( this );
		this->setHandle( ::CreateCompatibleBitmap( deviceCtx, width, height ) );
		return this->isCreated();
	}
};

// ============================================================
class LogicFont : public Structure< LOGFONT >{
public:
	inline LogicFont( TCHAR* name, long height, long width ){
		this->set( name );
		this->set( height, width );
	}

	inline LogicFont& set( TCHAR* name ){
		Copy( (*this)->lfFaceName, name );
		return *this;
	}

	inline LogicFont& set( long height, long width ){
		(*this)->lfHeight = height;
		(*this)->lfWidth = width;
		return *this;
	}
};

// ============================================================
class Font : public GdiObject< HFONT >{
public:
	inline bool Font::create( LOGFONT& logicFont ){
		this->setHandle( ::CreateFontIndirect( &logicFont ) );
		return this->isCreated();
	}
};

// ============================================================
enum Fid{
	Fid_Arial_14,
	Fid_MsShellDlg_12,
	Fid_Max, // Bounder, no used.
};

class FontSet : public Array< Font, Fid_Max >{

	DEFINE_SINGLE_EX( FontSet,
		this->get( Fid_Arial_14 ).create( LogicFont( _T("Arial"), 14, 5 ) );
		this->get( Fid_MsShellDlg_12 ).create( LogicFont( _T("MS Shell Dlg"), 12, 5 ) );
	, ; );

public:
	inline Font& get( Fid fid ){
		return (*this)[ fid ];
	}
};

}}}
// ===================================Namespace Tail==========================================

#endif
