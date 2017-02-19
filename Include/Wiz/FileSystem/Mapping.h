// ============================================================
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_FILESYSTEM_MAPPING_H__
#define __WIZ_FILESYSTEM_MAPPING_H__

#include "Wiz/FileSystem/Forward.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace FileSystem{

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

}}
// ===================================Namespace Tail==========================================

#endif
