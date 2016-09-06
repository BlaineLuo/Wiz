// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_FILESYSTEM_FORWARD_H__
#define __WIZ_FILESYSTEM_FORWARD_H__

#include "Wiz/Time.h"

#ifndef _WIN32_WCE
	#pragma comment( lib, "Version" )
#endif

// ===================================Namespace Head==========================================
namespace Wiz{ namespace FileSystem{

using namespace Wiz::Time;

// ============================================================
static TCHAR const VersionSubBlockComments[] = _T("Comments");
static TCHAR const VersionSubBlockCompanyName[] = _T("CompanyName");
static TCHAR const VersionSubBlockFileDescription[] = _T("FileDescription");
static TCHAR const VersionSubBlockFileVersion[] = _T("FileVersion");

static TCHAR const VersionSubBlockInternalName[] = _T("InternalName");
static TCHAR const VersionSubBlockLegalCopyright[] = _T("LegalCopyright");
static TCHAR const VersionSubBlockLegalTrademarks[] = _T("LegalTrademarks");
static TCHAR const VersionSubBlockOriginalFilename[] = _T("OriginalFilename");

static TCHAR const VersionSubBlockProductName[] = _T("ProductName");
static TCHAR const VersionSubBlockProductVersion[] = _T("ProductVersion");
static TCHAR const VersionSubBlockPrivateBuild[] = _T("PrivateBuild");
static TCHAR const VersionSubBlockSpecialBuild[] = _T("SpecialBuild");

// ============================================================
// @Brief: File Version Information
// ============================================================
class VersionInfo : public Buffer{

public:
	struct Translation{
		WORD _language;
		WORD _codePage;
	};

private:
	VS_FIXEDFILEINFO* _fixedFileInfo;
	Translation* _translationArray;
	UINT _translationNum;

public:
	inline VersionInfo() :
		_fixedFileInfo(NULL),
		_translationArray(NULL)
	{
		this->setTranslationNum( 0 );
	}

	bool create( TCHAR* fileName ){

		Reconstruct( this );

		UINT bufferSize = ::GetFileVersionInfoSize( fileName, NULL );
		if( bufferSize <= 0 )
			return false;

		this->createBuffer( bufferSize );
		if( !::GetFileVersionInfo( fileName, NULL, this->getSize(), *this ) )
			return false;

		UINT valueLen = 0;
		if( !::VerQueryValue( *this, _T("\\"), (LPVOID*)&_fixedFileInfo, &valueLen ) )
			return false;

		valueLen = 0;
		if( !::VerQueryValue( *this, _T("\\VarFileInfo\\Translation"), (LPVOID*)&_translationArray, &valueLen ) )
			return false;

		if( valueLen <= 0 )
			return false;

		this->setTranslationNum( valueLen / sizeof(Translation) );
		return true;
	}

	inline VS_FIXEDFILEINFO* getFileInfoFixed(){
		return _fixedFileInfo;
	}

	Translation* getTranslation( UINT index = 0 ){

		if( this->getTranslationNum() <= 0 )
			return NULL;

		return &_translationArray[ index % this->getTranslationNum() ];
	}

	inline UINT getTranslationNum(){
		return _translationNum;
	}

	inline void setTranslationNum( UINT number ){
		_translationNum = number;
	}

	TCHAR* getFileInfoString( const TCHAR* subBlock, UINT translationIndex = 0 ){

		Translation* translation = this->getTranslation( translationIndex );
		if( NULL == translation )
			return NULL;

		StringT<> strSubBlock;
		strSubBlock.format(
			_T("\\StringFileInfo\\%04x%04x\\%s"),
			translation->_language,
			translation->_codePage,
			subBlock
		);

		TCHAR* string = NULL;
		::VerQueryValue( *this, strSubBlock, (LPVOID*)&string, NULL );
		return string;
	}
};

// ============================================================
class InstanceInfo{

	DEFINE_SINGLE_EX( InstanceInfo, ;, ; );

public:
	StringT<> _exe;
	StringT<> _name;
	StringT<> _path;
	VersionInfo _versionInfo;

	bool create( TCHAR* name ){
		MaxPath<> instancePath;
		::GetModuleFileName( NULL, instancePath, instancePath._maxCount );

		_exe.format( _T("%s.exe"), name );
		_name = name;
		_path = instancePath;
		return _versionInfo.create( _path );
	}
};

// ============================================================
class Directory : public HandleT< HANDLE, INVALID_HANDLE_VALUE >{
public:
	inline ~Directory(){
		if( this->isCreated() )
			::FindClose( *this );
	}

	bool findFile( TCHAR* searchExpression, WIN32_FIND_DATA& winFindData ){

		if( !this->isCreated() )
			return this->findFirstFile( searchExpression, winFindData );

		if( !this->findNextFile( winFindData ) ){
			Reconstruct( this );
			return false;
		}
		return true;
	}

	bool findFirstFile( TCHAR* searchExpression, WIN32_FIND_DATA& winFindData ){

		if( NULL == searchExpression )
			return false;

		Reconstruct( this );

		this->setHandle( ::FindFirstFile( searchExpression, &winFindData ) );
		return this->isCreated();
	}

	bool findNextFile( WIN32_FIND_DATA& winFindData ){
		return( 0 != ::FindNextFile( *this, &winFindData ) );
	}
};

// ============================================================
class File : public HandleT< HANDLE, INVALID_HANDLE_VALUE >{

public:
	enum{
		_invalidSetFilePointer = INVALID_SET_FILE_POINTER,
		_moveMethodBegin = FILE_BEGIN,
		_moveMethodCurrent = FILE_CURRENT,
		_moveMethodEnd = FILE_END
	};

	static inline bool isDirectory( DWORD fileAttributes ){
		return( 0 != ( fileAttributes & FILE_ATTRIBUTE_DIRECTORY ) );
	}

	static inline bool isEncrypted( DWORD fileAttributes ){
		return( 0 != ( fileAttributes & FILE_ATTRIBUTE_ENCRYPTED ) );
	}

	static inline bool isHidden( DWORD fileAttributes ){
		return( 0 != ( fileAttributes & FILE_ATTRIBUTE_HIDDEN ) );
	}

	StringT<> _fileName;

	inline ~File(){
		if( this->isCreated() )
			this->closeHandle();
	}

	bool create(
		TCHAR* fileName,
		DWORD shareMode = 0,
		DWORD creationDisposition = OPEN_ALWAYS,
		DWORD flagsAndAttributes = FILE_ATTRIBUTE_NORMAL
	){
		Reconstruct( this );

		this->setHandle( ::CreateFile(
			fileName,
			GENERIC_READ | GENERIC_WRITE,
			shareMode,
			NULL,
			creationDisposition,
			flagsAndAttributes,
			NULL
		) );

		if( !this->isCreated() )
			return false;

		_fileName = fileName;
		return true;
	}

	inline QWORD getFileSize(){
		LARGE_INTEGER size = {0};
		size.LowPart = ::GetFileSize( *this, (LPDWORD)&size.HighPart );
		return size.QuadPart;
	}

	bool setFileSize( QWORD size ){
		QWORD prevSeek = this->getSeek();
		this->setSeek( _moveMethodBegin, size );
		this->setEndOfFile();
		this->setSeek( _moveMethodBegin, prevSeek );
		return true;
	}

	inline bool setEndOfFile(){
		return( 0 != ::SetEndOfFile( *this ) );
	}

	inline QWORD getSeek(){
		LARGE_INTEGER seek = {0};
		seek.LowPart = ::SetFilePointer( *this, seek.LowPart, &seek.HighPart, _moveMethodCurrent );
		return seek.QuadPart;
	}

	inline bool setSeek( DWORD moveMethod, __int64 offset = 0 ){
		LARGE_INTEGER seek = {0};
		seek.QuadPart = offset;
		//TODO: check negative.
		return( _invalidSetFilePointer != ::SetFilePointer( *this, seek.LowPart, &seek.HighPart, moveMethod ) );
	}

	inline DWORD read( void* buffer, unsigned int size ){
		DWORD readNum = 0;
		::ReadFile( *this, buffer, size, &readNum, NULL );
		return readNum;
	}

	inline DWORD write( void* buffer, unsigned int size ){
		DWORD writtenNum = 0;
		::WriteFile( *this, buffer, size, &writtenNum, NULL );
		return writtenNum;
	}
};

// ============================================================
#ifndef _WIN32_WCE

class Profile{

public:
	typedef std::list< TCHAR* > KeySet;

protected:
	MaxPath<> _fileName;
	MaxPath<> _section;
	KeySet _keySet;

public:
	template< typename T >
	inline void reconstruct( T& v ){
		Destroy( &_keySet );
		MemoryReset( v );
		Construct( &_keySet );
	}

	inline bool create( TCHAR* fileName, TCHAR* section = NULL ){
		File file;
		if( !file.create( fileName ) )
			return false;

		_fileName << fileName;
		_section << section;
		return true;
	}

	// Note: Profile Encode in UTF-8 without BOM
	inline bool getValueStr(
		TCHAR* valueReturn,
		const UINT valueSize,
		const TCHAR* key,
		const TCHAR* valueDefault = _T("")
	){
		return( 0 < ::GetPrivateProfileString(
			_section,
			key,
			valueDefault,
			valueReturn,
			valueSize,
			_fileName
		) );
	}

	inline int getValueInt( const TCHAR* key, const int valueDefault = 0 ){
		return ::GetPrivateProfileInt( _section, key, valueDefault, _fileName );
	}

	inline bool setValue( const TCHAR* key, TCHAR* format, ... ){
		Text1024<> text;
		VPRINTF( text, format );
		return( 0 != ::WritePrivateProfileString( _section, key, text, _fileName ) );
	}
};

#endif

// ============================================================
class Logger : public File{
public:
	inline bool create( TCHAR* fileName ){
		Reconstruct( this );
		if( !this->File::create( fileName, FILE_SHARE_READ ) )
			return false;
		return this->setSeek( _moveMethodEnd );
	}

	inline void putString( TCHAR* string ){
		this->write( string, GetLength( string ) * sizeof(TCHAR) );
	}

	inline void putStringEx( TCHAR* format, ... ){
		Text1024<> text;
		VPRINTF( text, format );
		this->putString( text );
	}

	inline void putLastError( TCHAR* fileName, DWORD lineNum, TCHAR* label, DWORD errorCode = ::GetLastError() ){
		this->putStringEx(
			_T("%s File=%s, Line=%d, Label=%s, ErrCode=%d\r\n"),
			SystemTime().getLocalTime() >> &Text32<>()[0],
			fileName,
			lineNum,
			label,
			errorCode
		);
	}

	inline void putFormatMessage( TCHAR* fileName, DWORD lineNum, TCHAR* label, DWORD errorCode = ::GetLastError() ){
		this->putStringEx(
			_T("%s File=%s, Line=%d, Label=%s, ErrCode=%d, ErrMsg=%s\r\n"),
			SystemTime().getLocalTime() >> &Text32<>()[0],
			fileName,
			lineNum,
			label,
			errorCode,
			ErrorString().getErrorString( errorCode ).getString()
		);
	}
};

// ============================================================
class LoggerEx : public Logger{

protected:
	StringT<> _dirName;
	StringT<> _prefix;
	QWORD _expireMilliseconds;

	bool create( SYSTEMTIME& time ){
		( (SystemTime*)&time )->getLocalTime();
		StringT<> filePath;
		filePath.format(
			_T("%s/%s%04d.%02d.%02d.%02d.%02d.%02d.log"),
			_dirName.getString(),
			_prefix.getString(),
			time.wYear,
			time.wMonth,
			time.wDay,
			time.wHour,
			time.wMinute,
			time.wSecond
		);
		return this->Logger::create( filePath );
	}

public:
	bool create( TCHAR* dirName, TCHAR* prefix, QWORD expireMilliseconds ){
		_dirName = dirName;
		_prefix = prefix;
		_expireMilliseconds = expireMilliseconds;

		::CreateDirectory( _dirName, NULL );
		return this->create( SystemTime().getLocalTime() );
	}

	void deleteExpired(){
		StringT<> searchExpression;
		searchExpression.format( _T("%s/%s*"), _dirName.getString(), _prefix.getString() );

		SystemTime timeNow;
		timeNow.getSystemTime();
		Directory directory;
		WIN32_FIND_DATA findData = {0};
		for( ; directory.findFile( searchExpression, findData ); ){

			if( File::isDirectory( findData.dwFileAttributes ) )
				continue;

			if( timeNow.toMilliseconds() - FileTime::ToMilliseconds( findData.ftCreationTime ) < _expireMilliseconds )
				continue;

			StringT<> fullName;
			fullName.format( _T("%s/%s"), _dirName.getString(), &findData.cFileName );
			::DeleteFile( fullName );
		}
		this->create( timeNow );
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
