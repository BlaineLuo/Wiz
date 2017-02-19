// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_WIDGET_H__
#define __WIZ_WIDGET_H__

#ifndef _WIN32_WCE

#include "Wiz/Com/Forward.h"
#include "Wiz/Graphing/GdiEx.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Widget{

using namespace Wiz::Com;
using namespace Wiz::Graphing::GdiEx;

// ============================================================
struct Resolution{
	int _width;
	int _height;

	inline Resolution& build_A(){
		_width = 320;
		_height = 180;
	}

	inline Resolution& build_B(){
		_width = 640;
		_height = 360;
	}

	inline Resolution& build_C(){
		_width = 960;
		_height = 540;
	}

	inline Resolution& build_D(){
		_width = 1280;
		_height = 720;
	}

	inline Resolution& build_E(){
		_width = 1600;
		_height = 900;
	}

	inline Resolution& build_F(){
		_width = 1920;
		_height = 1080;
	}
};

// ============================================================
class ScreenCapturer{

public:
	enum{ _bufSize = 1 * 1024 * 1024 };

private:
	Resolution _resSrc;
	Resolution _resDst;
	UINT _quality;

	DeviceCtxCommon _dcScreen;
	DeviceCtxCompatible _dcMemory;
	Bitmap _gdiBitmap;
	GdiObject<> _gdiObject;
	ImageEncoder _imageEncoder;
	Stream _stream;

public:
	ScreenCapturer(){
		_resSrc._width = ::GetSystemMetrics( SM_CXSCREEN );
		_resSrc._height = ::GetSystemMetrics( SM_CYSCREEN );
		_imageEncoder.create( (PTCHAR)MimeTypeJpeg );
	}

	bool create( HGLOBAL buffer ){

		if( S_OK != _stream.createStreamOnHGlobal( buffer ) )
			return false;

		return this->setParameters( Resolution().build_A(), 50 );
	}

	bool setParameters( const Resolution& resDst, const UINT quality ){
		_resDst = resDst;
		_quality = quality;
		Arrange( _quality, 0, 100 );

		_imageEncoder.setQuality( _quality );

		_dcScreen.create( NULL );
		_dcMemory.create( _dcScreen );
		_gdiBitmap.createCompatibleBitmap( _dcScreen, _resDst._width, _resDst._height );

		// Bind the Memory DC to Bitmap.
		_gdiObject.selectObject( _dcMemory, _gdiBitmap );

		_dcMemory.setStretchBltMode( HALFTONE );
		return true;
	}

	bool capture(){

		if( !_stream.isCreated() )
			return false;

		::StretchBlt(
			_dcMemory,
			0,
			0,
			_resDst._width,
			_resDst._height,
			_dcScreen,
			0,
			0,
			_resSrc._width,
			_resSrc._height,
			SRCCOPY
		);

		_stream.setSeekOffset( 0 );
		Gdiplus::Bitmap bitmap( _gdiBitmap, NULL );
		bitmap.Save( _stream, &_imageEncoder._classId, _imageEncoder._parameters );
		return true;
	}
};

// ============================================================
class ScreenDisplayer{

	HWND _window;
	Stream _stream;

public:
	bool create( HGLOBAL buffer, HWND window ){

		if( S_OK != _stream.createStreamOnHGlobal( buffer ) )
			return false;

		_window = window;
		return true;
	}

	bool display(){

		if( !_stream.isCreated() )
			return false;

		RECT rect = {0};
		::GetClientRect( _window, &rect );

		Gdiplus::Graphics graphics( _window );
		Gdiplus::Image image( _stream );

		graphics.DrawImage( &image, 0, 0, rect.right, rect.bottom );
		return true;
	}
};

}}
// ===================================Namespace Tail==========================================

#endif

#endif
