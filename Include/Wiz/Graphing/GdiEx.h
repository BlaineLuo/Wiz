// ============================================================
// @Brief: GDI+ Classes (for Win32)
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_GRAPHING_GDIEX_H__
#define __WIZ_GRAPHING_GDIEX_H__

#ifndef _WIN32_WCE

#include "Wiz/Graphing/Gdi.h"
#include <Gdiplus.h>
#pragma comment( lib, "Gdiplus" )

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Graphing{ namespace GdiEx{

using namespace Wiz::Graphing::Gdi;

// ============================================================
static TCHAR const MimeTypeBmp[] = _T("image/bmp");
static TCHAR const MimeTypeJpeg[] = _T("image/jpeg");
static TCHAR const MimeTypeGif[] = _T("image/gif");
static TCHAR const MimeTypeTiff[] = _T("image/tiff");
static TCHAR const MimeTypePng[] = _T("image/png");

// ============================================================
// @Brief: GDI+ Startup
// ============================================================
class Startup{

	DEFINE_SINGLE_EX( Startup,
		// Caution: Don't Reset the _input to Zero
		_token = 0;
		Gdiplus::GdiplusStartup( &_token, &_input, NULL );
	,
		Gdiplus::GdiplusShutdown( _token );
	);

	Gdiplus::GdiplusStartupInput _input;
	ULONG_PTR _token;
};

// ============================================================
class ImageEncoder{

public:
	CLSID _classId;
	ULONG _quality;
	ULONG _colorDepth;
	Gdiplus::EncoderParameters*	_parameters;
	BufferT< Gdiplus::EncoderParameters > _bufParameters;

	ImageEncoder(){
		DWORD paramCount = 2;
		DWORD paramSize = sizeof(Gdiplus::EncoderParameters) + sizeof(Gdiplus::EncoderParameter) * ( paramCount - 1 );

		_bufParameters.createBuffer( paramSize );
		_parameters	= _bufParameters.getHandle();
		_quality = 100;
		_colorDepth	= 16L;
		MemoryReset( _classId );

		_parameters->Count = paramCount;
		_parameters->Parameter[0].NumberOfValues = 1;
		_parameters->Parameter[0].Guid = Gdiplus::EncoderQuality;
		_parameters->Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
		_parameters->Parameter[0].Value = &_quality;

		_parameters->Parameter[1].NumberOfValues = 1;
		_parameters->Parameter[1].Guid = Gdiplus::EncoderColorDepth;
		_parameters->Parameter[1].Type = Gdiplus::EncoderParameterValueTypeLong;
		_parameters->Parameter[1].Value = &_colorDepth;
	}

	bool create( PTCHAR mimeType ){
		UINT encoderNum = 0;
		UINT bufferSize = 0;
		if( Gdiplus::GetImageEncodersSize( &encoderNum, &bufferSize ) != Gdiplus::Ok )
			return false;

		BufferT< Gdiplus::ImageCodecInfo > bufImageCodecInfo;
		bufImageCodecInfo.createBuffer( bufferSize );
		Gdiplus::ImageCodecInfo* imageCodecInfo = bufImageCodecInfo.getHandle();
		if( Gdiplus::GetImageEncoders( encoderNum, bufferSize, imageCodecInfo ) != Gdiplus::Ok )
			return false;

		StringT< wchar_t > strMimeType = mimeType;
		for( UINT i = 0; i < encoderNum; i++ ){

			if( 0 != Compare( strMimeType, imageCodecInfo[i].MimeType ) )
				continue;

			this->setClassId( imageCodecInfo[i].Clsid );
			return true;
		}
		return false;
	}

	inline void setClassId( CLSID& classId ){
		_classId = classId;
	}

	inline void setQuality( ULONG quality ){
		_quality = quality;
	}

	inline void setColorDepth( ULONG colorDepth ){
		_colorDepth = colorDepth;
	}
};

// ============================================================
// @Brief: GDI+ Bitmap Class
// ============================================================
class BitmapEx : public HandleP< Gdiplus::Bitmap >{
public:
	using HandleP::operator =;
	using HandleP::operator ->;
};

}}}
// ===================================Namespace Tail==========================================

#endif

#endif
