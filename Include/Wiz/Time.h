// ============================================================
// @Brief	Time Classes 
// @Author	Blaine
// @Date	2014/05/06
// ============================================================
#ifndef __WIZ_TIME_H__
#define __WIZ_TIME_H__

#include "Wiz/String/StringT.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Time{

using namespace Wiz::String;

enum{ SecondToMillisecond = 1000 };
enum{ MinuteToSecond = 60 };
enum{ HourToMinute = 60 };
enum{ DayToHour = 24 };
enum : QWORD{ MinuteToMillisecond = MinuteToSecond * SecondToMillisecond };
enum : QWORD{ HourToMillisecond = HourToMinute * MinuteToMillisecond };
enum : QWORD{ DayToMillisecond = DayToHour * HourToMillisecond };

// ============================================================
class Frequency : public Structure< LARGE_INTEGER >{
	DEFINE_SINGLE_EX( Frequency,
		::QueryPerformanceFrequency( (Entry*)this );
	, ; );
};

// ============================================================
class Counter : public Structure< double >{
public:
	inline Counter& operator =( double second ){
		(Entry&)(*this) = second;
		return *this;
	}

	inline bool isTimeout( double second ){
		Counter now;
		now.query();
		return now > ( *this + second );
	}

	inline bool query(){
		LARGE_INTEGER now = {0};
		if( !::QueryPerformanceCounter( &now ) )
			return false;

		*this = ( (double)now.QuadPart / (double)( *Frequency::CreateInstance() )->QuadPart );
		return true;
	}
};

// ============================================================
class FileTime : public Structure< FILETIME >{

public:
	enum{ _scaleMilliToHundredNano = 10000 };

	static inline bool CopyFrom( FILETIME& fileTime, const SYSTEMTIME& sysTime ){
		return( 0 != ::SystemTimeToFileTime( &sysTime, &fileTime ) );
	}

	static inline QWORD ToMilliseconds( FILETIME& time ){
		QWORD hundredNanoseconds = ( (QWORD)time.dwHighDateTime << 32 ) | time.dwLowDateTime;
		return hundredNanoseconds / _scaleMilliToHundredNano;
	}

	static inline void IncMilliseconds( FILETIME& time, QWORD milliseconds ){
		QWORD hundredNanoseconds = ( (QWORD)time.dwHighDateTime << 32 ) | time.dwLowDateTime;
		hundredNanoseconds += milliseconds * _scaleMilliToHundredNano;
		::CopyMemory( &time, &hundredNanoseconds, sizeof(time) );
	}

	inline FileTime& operator <<( SYSTEMTIME& time ){
		FileTime::CopyFrom( *this, time );
		return *this;
	}

	inline QWORD toMilliseconds(){
		return FileTime::ToMilliseconds( *this );
	}

	inline FileTime& incMilliseconds( QWORD milliseconds ){
		FileTime::IncMilliseconds( *this, milliseconds );
		return *this;
	}
};

// ============================================================
class SystemTime : public Structure< SYSTEMTIME >{

public:
	static inline bool CopyFrom( SYSTEMTIME& sysTime, const FILETIME& fileTime ){
		return( 0 != ::FileTimeToSystemTime( &fileTime, &sysTime ) );
	}

	static inline QWORD ToMilliseconds( SYSTEMTIME& time ){
		FileTime fileTime;
		fileTime << time;
		return fileTime.toMilliseconds();
	}

	static inline bool IncMilliseconds( SYSTEMTIME& time, QWORD milliseconds ){
		FileTime fileTime;
		fileTime << time;
		fileTime.incMilliseconds( milliseconds );
		return SystemTime::CopyFrom( time, fileTime );
	}

	template< typename T >
	static inline T* CopyTo( const SYSTEMTIME& time, T* string ){
		return Printf(
			string,
			20,
			CONST_TEXT( T*, "%04d/%02d/%02d %02d:%02d:%02d" ),
			time.wYear,
			time.wMonth,
			time.wDay,
			time.wHour,
			time.wMinute,
			time.wSecond
		);
	}
	template< typename T >
	static inline T* CopyDateTo( const SYSTEMTIME& time, T* string ){
		return Printf(
			string,
			8,
			CONST_TEXT( T*, "%04d/%02d/%02d" ),
			time.wYear,
			time.wMonth,
			time.wDay
		);
	}
	template< typename T >
	static inline T* CopyTimeTo( const SYSTEMTIME& time, T* string ){
		return Printf(
			string,
			8,
			CONST_TEXT( T*, "%02d:%02d:%02d" ),
			time.wHour,
			time.wMinute,
			time.wSecond
		);
	}

	using Structure::operator =;

	inline SystemTime& operator <<( FILETIME& time ){
		SystemTime::CopyFrom( *this, time );
		return *this;
	}

	template< typename T >
	SystemTime& operator <<( T string ){
		StringT< wchar_t > str = string;
		DATE date = 0;
		if( ::VarDateFromStr( str, 0, 0, &date ) == S_OK )
			::VariantTimeToSystemTime( date, (Entry*)this );
		return *this;
	}

	template< typename T >
	T* operator >>( T* string ){
		do{
			if( 0 == (*this)->wYear && 0 == (*this)->wMonth && 0 == (*this)->wDay ){
				SystemTime::CopyTimeTo( *this, string );
				break;
			}
			if( 0 == (*this)->wHour && 0 == (*this)->wMinute && 0 == (*this)->wSecond ){
				SystemTime::CopyDateTo( *this, string );
				break;
			}
			SystemTime::CopyTo( *this, string );
		}while(0);
		return string;
	}

	inline __int64 operator -( SYSTEMTIME& time ){
		return( this->toMilliseconds() - SystemTime::ToMilliseconds( time ) );
	}

	inline SystemTime& resetMilliseconds(){
		(*this)->wMilliseconds = 0;
		return *this;
	}

	inline SystemTime& resetSeconds(){
		(*this)->wSecond = 0;
		return this->resetMilliseconds();
	}

	inline SystemTime& resetMinutes(){
		(*this)->wMinute = 0;
		return this->resetSeconds();
	}

	inline SystemTime& resetHours(){
		(*this)->wHour = 0;
		return this->resetMinutes();
	}

	inline SystemTime& setDate( WORD year = 0, WORD month = 0, WORD day = 0 ){
		(*this)->wYear = year;
		(*this)->wMonth = month;
		(*this)->wDay = day;
		return *this;
	}

	inline SystemTime& setTime( WORD hour = 0, WORD minute = 0, WORD second = 0, WORD milliseconds = 0 ){
		(*this)->wHour = hour;
		(*this)->wMinute = minute;
		(*this)->wSecond = second;
		(*this)->wMilliseconds = milliseconds;
		return *this;
	}

	inline QWORD toMilliseconds(){
		return SystemTime::ToMilliseconds( *this );
	}

	inline SystemTime& incMilliseconds( QWORD v ){
		SystemTime::IncMilliseconds( *this, v );
		return *this;
	}

	inline SystemTime& incSeconds( QWORD v ){
		return this->incMilliseconds( v * SecondToMillisecond );
	}

	inline SystemTime& incMinutes( QWORD v ){
		return this->incSeconds( v * MinuteToSecond );
	}

	inline SystemTime& incHours( QWORD v ){
		return this->incMinutes( v * HourToMinute );
	}

	inline SystemTime& incDays( QWORD v ){
		return this->incHours( v * DayToHour );
	}

	inline SystemTime& getSystemTime(){
		::GetSystemTime( (Entry*)this );
		return *this;
	}

	inline SystemTime& getLocalTime(){
		::GetLocalTime( (Entry*)this );
		return *this;
	}

	inline bool setLocalTime(){
		return( 0 != ::SetLocalTime( (Entry*)this ) );
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
