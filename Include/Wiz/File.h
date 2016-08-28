// ============================================================
// @Brief	File Management Classes
// @Author	Blaine
// @Date	2014/03/24
// ============================================================
#ifndef __WIZ_FILE_H__
#define __WIZ_FILE_H__

#include "Wiz/Time.h"

#ifndef _WIN32_WCE
	#pragma comment( lib, "Version" )
#endif

// ===================================Namespace Head==========================================
namespace Wiz{ namespace File{

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
// @Brief	File Version Information
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

	bool create( PTCHAR fileName ){

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

	PTCHAR getFileInfoString( PTCHAR subBlock, UINT translationIndex = 0 ){

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

		PTCHAR string = NULL;
		::VerQueryValue( *this, strSubBlock, (LPVOID*)&string, NULL );
		return string;
	}
};

// ============================================================
// @Brief	Instance Information
// ============================================================
class InstanceInfo{

	DEFINE_SINGLE_EX( InstanceInfo, ;, ; );

public:
	StringT<> _exe;
	StringT<> _name;
	StringT<> _path;
	VersionInfo _versionInfo;

	bool create( PTCHAR name ){
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

	bool findFile( PTCHAR searchExpression, WIN32_FIND_DATA& winFindData ){

		if( !this->isCreated() )
			return this->findFirstFile( searchExpression, winFindData );

		if( !this->findNextFile( winFindData ) ){
			Reconstruct( this );
			return false;
		}
		return true;
	}

	bool findFirstFile( PTCHAR searchExpression, WIN32_FIND_DATA& winFindData ){

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
		PTCHAR fileName,
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
class Mapping : public HandleT< HANDLE, NULL >{
public:
	inline ~Mapping(){
		if( this->isCreated() )
			this->closeHandle();
	}

	inline bool create( File& file, PTCHAR name = NULL, DWORD maxSize = 0 ){

		Reconstruct( this );
		if( !file.isCreated() )
			return false;

		this->setHandle( ::CreateFileMapping( file, NULL, PAGE_READWRITE, 0, maxSize, name ) );
		return this->isCreated();
	}

	inline bool commit(){
		return( 0 != ::FlushFileBuffers( *this ) );
	}
};

// ============================================================
class View : public HandleT< PVOID, NULL >{
public:
	inline ~View(){
		if( this->isCreated() )
			::UnmapViewOfFile( *this );
	}

	inline bool create( Mapping& mapping, DWORD fileOffset = 0, DWORD viewSize = 0 ){

		Reconstruct( this );
		if( !mapping.isCreated() )
			return false;

		//Mapping View to File.
		this->setHandle( ::MapViewOfFile( mapping, FILE_MAP_ALL_ACCESS, 0, fileOffset, viewSize ) );
		return this->isCreated();
	}

	inline bool commit( DWORD viewSize ){
		if( !this->isCreated() )
			return false;
		return( 0 != ::FlushViewOfFile( *this, viewSize ) );
	}
};

// ============================================================
template< typename T >
class MemoryMappedFile{

protected:
	File _file;
	Mapping	_mapping;
	View _view;

public:
	typedef T Entry;

	bool create( PTCHAR filePath, DWORD shareMode = 0 ){

		Reconstruct( this );
		if( !_file.create( filePath, shareMode ) )
			return false;

		_file.setFileSize( sizeof(Entry) );
		if( !_mapping.create( _file, NULL, sizeof(Entry) ) )
			return false;

		if( !_view.create( _mapping ) )
			return false;
		return true;
	}

	bool commit(){
		QWORD fileSize = _file.getFileSize();
		if( !_view.commit( (DWORD)fileSize ) )
			return false;

		if( !_mapping.commit() )
			return false;
		return true;
	}

	inline Entry* getHandle(){
		return (Entry*)( _view.getHandle() );
	}

	inline bool isCreated(){
		return _view.isCreated();
	}
};

// ============================================================
#pragma pack( push, 1 )
template< typename T, unsigned int MagicHead, unsigned int MagicTail >
class TrunkGuard : public StaticContainer< T, 1 >{

protected:
	unsigned int _head;
	Entry _entry;
	unsigned int _tail;

public:
	enum{
		_magicHead = MagicHead,
		_magicTail = MagicTail,
	};

	inline unsigned int getTotalSize(){
		return _entrySize + sizeof(_head) + sizeof(_tail);
	}

	inline Entry& getEntry(){
		return _entry;
	}

	inline bool check(){
		return( _magicHead == _head && _magicTail == _tail );
	}

	inline void reset(){
		MemoryReset( *this );
		_head = _magicHead;
		_tail = _magicTail;
	}
};
#pragma pack( pop )

// ============================================================
template< typename T, unsigned int MagicHead, unsigned int MagicTail, unsigned int MaxCount >
class TrunkArray : public Array< TrunkGuard< T, MagicHead, MagicTail >, MaxCount >{

public:
	typedef typename Entry::Entry Trunk;

	Trunk* find( unsigned int id ){

		for( int i = 0; i < _maxCount; i++ ){
			Entry& entry = (*this)[i];
			if( entry.check() == false )
				entry.reset();

			if( entry.getEntry()._id != id )
				continue;

			return &entry.getEntry();
		}
		return NULL;
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
		Text1024<> value;
		VPRINTF( &value[0], value._maxCount, format );
		return( 0 != ::WritePrivateProfileString( _section, key, value, _fileName ) );
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
		VPRINTF( &text[0], text._maxCount, format );
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

// ============================================================
class Serial : protected File{

public:
	using File::read;
	//using File::write;

	inline ~Serial(){
		if( this->isCreated() )
			this->closeHandle();
	}

	bool create( DWORD portNum ){

		Reconstruct( this );

		StringT<> fileName;
		if( portNum < 10 )
			fileName.format( _T("COM%d:"), portNum );
		else
			fileName.format( _T("\\\\.\\COM%d"), portNum );

		return this->File::create( fileName, 0, OPEN_EXISTING, 0 );
	}

	inline bool clearError( DWORD* errorCode = NULL, COMSTAT* comStat = NULL ){
		return( 0 != ::ClearCommError( *this, errorCode, comStat ) );
	}

	inline bool getSetting( DCB& dcb ){
		return( 0 != ::GetCommState( *this, &dcb ) );
	}

	inline bool setSetting( DCB& dcb ){
		return( 0 != ::SetCommState( *this, &dcb ) );
	}

	bool setSetting(
		DWORD baudRate,
		BYTE stopBits = ONESTOPBIT,
		BYTE byteSize = 8,
		DWORD rtsControl = RTS_CONTROL_DISABLE
	){
		DCB dcb = {0};
		this->getSetting( dcb );
		dcb.DCBlength = sizeof(dcb);
		dcb.BaudRate = baudRate;
		dcb.StopBits = stopBits;
		dcb.ByteSize = byteSize;
		dcb.fRtsControl = rtsControl;
		return this->setSetting( dcb );
	}

	bool setSettingParity( BYTE parity = SPACEPARITY, bool isCheck = true ){
		DCB dcb = {0};
		this->getSetting( dcb );
		dcb.fParity = isCheck;
		dcb.Parity = parity;
		return this->setSetting( dcb );
	}

	inline bool getTimeouts( COMMTIMEOUTS& v ){
		return( 0 != ::GetCommTimeouts( *this, &v ) );
	}

	inline bool setTimeouts( COMMTIMEOUTS& v ){
		return( 0 != ::SetCommTimeouts( *this, &v ) );
	}

	inline bool purge( DWORD flags ){
		return( 0 != ::PurgeComm( *this, flags ) );
	}

	inline bool rxAbort(){
		return this->purge( PURGE_RXABORT );
	}

	inline bool rxClear(){
		return this->purge( PURGE_RXCLEAR );
	}

	inline bool txAbort(){
		return this->purge( PURGE_TXABORT );
	}

	inline bool txClear(){
		return this->purge( PURGE_TXCLEAR );
	}

	inline DWORD write( BYTE data ){
		return this->File::write( &data, sizeof(data) );
	}
};

}}
// ===================================Namespace Tail==========================================

#endif
