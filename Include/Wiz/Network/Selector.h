// ============================================================
// @Brief: Winsock IO Selector
// @Author: Blaine Luo
// @Date: 2016/12
// ============================================================
#ifndef __WIZ_NETWORK_SELECTOR_H__
#define __WIZ_NETWORK_SELECTOR_H__

#include "Wiz/Network/Socket.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Network{ namespace Socket{

// ============================================================
struct Context{
	unsigned short _index;
	unsigned short _isAutoRelease;
	Socket _socket;
	Seeker _seeker;
	Counter _aliveTime;
};

// ============================================================
template< unsigned int MaxCount >
class Selector : protected Pool< Context, MaxCount >{

public:
	enum IoType{
		IoType_Task,
		IoType_RecvTcp,
		IoType_RecvUdp,
		IoType_SendTcp,
		IoType_SendUdp,
	};

	struct IoTask{
		unsigned int _id;
		Socket* _socket;
		SockAddr _remoteAddr;
		void* _data;
		unsigned int _len;
		IoType _ioType;
	};

protected:
	typedef Array< Buffer, _maxCount > BufferSet;
	BufferSet _receivingSet;
	BufferSet _bufferingSet;
	typename BufferSet::Indexer _bufferingIdx;

	AsyncQueue< IoTask, _maxCount > _recvTaskQueue;

	inline Buffer& getBuffering(){
		return _bufferingSet[ _bufferingIdx++ ];
	}

	virtual void onError( Entry& entry, WORD netEvent, WORD netError ){
	}

	virtual void onAccept( Entry& entry ){
		SOCKET s = entry._socket.accept( SockAddr() );
		Socket* socketA = (Socket*)&s;
		if( !socketA->isCreated() )
			return;

		Socket* socketB = this->acquire( true );
		if( NULL == socketB ){
			Reconstruct( socketA );
			return;
		}
		*socketB = *socketA;

		this->observe( *socketB );
	}

	virtual void onClose( Entry& entry ){
		entry._socket.shutdown( SD_SEND );
		::Sleep(1); // Delay for receive remaining data.
		//TODO: FD_CLOSE will not signal when Client Socket is not close.
		if( 0 != entry._isAutoRelease )
			this->release( entry._index );
	}

	virtual bool onRecvTcp( Entry& entry ){
		return false;
	}

	virtual bool onRecvUdp( Entry& entry ){
		return false;
	}

	virtual bool onSendTcp( Entry& entry ){
		return false;
	}

	virtual bool onSendUdp( Entry& entry ){
		return false;
	}

	virtual bool observe( Socket& socket ){
		return true;
	}

public:
	void create( unsigned int bufferSizePerCtx ){
		for( unsigned int i = 0; i < _maxCount; i++ ){
			_receivingSet[i].createBuffer( bufferSizePerCtx );
			_bufferingSet[i].createBuffer( bufferSizePerCtx );
		}
	}

	Socket* acquire( bool isAutoRelease = false ){
		unsigned int idx = 0;
		Entry* entry = this->Pool::acquire( &idx );
		if( NULL == entry )
			return NULL;

		Buffer& buffer = _receivingSet[ idx ];
		entry->_index = idx;
		entry->_isAutoRelease = isAutoRelease;
		entry->_seeker.create( buffer, buffer.getSize() );
		return &entry->_socket;
	}

	template< typename Data >
	bool pushTask( unsigned int id, Data& data ){
		int len = sizeof(data);
		IoTask ioTask = {0};
		ioTask._id = id;
		ioTask._ioType = IoType_Task;
		if( 0 < len ){
			Buffer& buffer = this->getBuffering();
			buffer.copyFrom( &data, len );
			ioTask._data = buffer.getHandle();
			ioTask._len = len;
		}
		return _recvTaskQueue.push( ioTask );
	}

	bool pushRecvTaskTcp( unsigned int id, Socket* socket, void* data, unsigned int len ){
		Buffer& buffer = this->getBuffering();
		buffer.copyFrom( data, len );

		IoTask ioTask = {0};
		ioTask._id = id;
		ioTask._ioType = IoType_RecvTcp;
		ioTask._socket = socket;
		ioTask._data = buffer.getHandle();
		ioTask._len = len;
		return _recvTaskQueue.push( ioTask );
	}

	bool pushRecvTaskUdp( unsigned int id, Socket* socket, SockAddr& remoteAddr, void* data, unsigned int len ){
		Buffer& buffer = this->getBuffering();
		buffer.copyFrom( data, len );

		IoTask ioTask = {0};
		ioTask._id = id;
		ioTask._ioType = IoType_RecvUdp;
		ioTask._socket = socket;
		ioTask._data = buffer.getHandle();
		ioTask._len = len;
		ioTask._remoteAddr = remoteAddr;
		return _recvTaskQueue.push( ioTask );
	}

	bool popRecvTask( IoTask& ioTask ){
		return _recvTaskQueue.pop( &ioTask, INFINITE );
	}

	bool buildTcpClient( Socket& socket, TCHAR* remoteIp, WORD remotePort, WORD localPort = PortNumberMin ){
		if( !socket.create( Protocol() )
		 || !socket.bind( SockAddr().setPort( localPort ) )
		 || !socket.connect( SockAddr( remoteIp, remotePort ) )
		 || !this->observe( socket ) ){
			Reconstruct( &socket );
			return false;
		}
		return true;
	}

	bool buildTcpServer( Socket& socket, WORD localPort ){
		if( !socket.create( Protocol() )
		 || !socket.bind( SockAddr().setPort( localPort ) )
		 || !socket.listen()
		 || !this->observe( socket ) ){
			Reconstruct( &socket );
			return false;
		}
		return true;
	}

	bool buildUdp( Socket& socket, TCHAR* localIp, WORD localPort, bool isReuseAddress = false ){
		if( !socket.create( Protocol( AF_INET, SOCK_DGRAM ) )
		 || !socket.setReuseAddress( true )
		 || !socket.bind( SockAddr( localIp, localPort ) )
		 || !this->observe( socket ) ){
			Reconstruct( &socket );
			return false;
		}
		return true;
	}

	bool buildMulticastUdp( Socket& socket, TCHAR* localIp, WORD localPort, TCHAR* groupIp = _T(MULTICAST_IP) ){
		if( !this->buildUdp( socket, localIp, localPort )
		 || !socket.enterMulticastGroup( groupIp, localIp )
		 || !socket.setMulticastTtl() )
			return false;
		return true;
	}
};

// ============================================================
struct FdSet : Structure< fd_set >{

	enum{ _maxCount = FD_SETSIZE };

	inline FdSet(){
		this->clear();
	}

	inline FdSet& clear(){
		FD_ZERO( &(*this) );
		return *this;
	}

	inline FdSet& insert( SOCKET s ){
		FD_SET( s, &(*this) );
		return *this;
	}

	inline FdSet& remove( SOCKET s ){
		FD_CLR( s, &(*this) );
		return *this;
	}

	inline bool check( SOCKET s ){
		return( 0 != FD_ISSET( s, &(*this) ) );
	}
};

// ============================================================
struct TimeVal : Structure< timeval >{
	inline TimeVal( long sec = 0, long microSec = 0 ){
		(*this)->tv_sec = sec;
		(*this)->tv_usec = microSec;
	}
};

// ============================================================
// @Brief: Berkeley style Selector
// ============================================================
class NativeSelector : public Selector< FD_SETSIZE >{

protected:
	Trigger< NativeSelector, Event > _trigger;
	FdSet _fdSet;

	DEFINE_SINGLE_EX( NativeSelector,
		_trigger.create( this, &NativeSelector::trigger, 1 );
	, ; );

	inline int select( timeval* timeout = NULL ){
		return ::select( 0, &_fdSet, NULL, NULL, timeout );
	}

	void prepare(){
		MemoryReset( _fdSet );
		for( int i = 0, matchCount = 0; i < _fdSet._maxCount && matchCount < (int)this->getCurCount(); i++ ){
			Entry& entry = (*this)[i];
			Socket& socket = entry._socket;
			if( NULL == socket.getHandle() || !socket.isCreated() )
				continue;

			_fdSet.insert( socket );
			matchCount++;
		};
	}

	void __cdecl trigger(){

		if( 0 == this->getCurCount() )
			return;

		this->prepare();

		int eventCount = this->select();
		if( SOCKET_ERROR == eventCount )
			return;

		for( int i = 0, matchCount = 0; i < _fdSet._maxCount && matchCount < eventCount; i++ ){
			Entry& entry = (*this)[i];
			Socket& socket = entry._socket;
			if( !_fdSet.check( socket ) )
				continue;

			if( socket.isListening() )
				this->onAccept( entry );

			else if( socket.isTcp() ){
				if( !this->onRecvTcp( entry ) )
					this->onClose( entry );
			}

			else if( socket.isUdp() )
				this->onRecvUdp( entry );
		}
	}
};

// ============================================================
// @Brief: Message-Based Selector
// ============================================================
template< unsigned int MaxCount >
class MessageSelector : public Dialog, public Selector< MaxCount >{

protected:
	DEFINE_SINGLE_EX( MessageSelector,
		DLGTEMPLATE dlgTemplate = {0};
		this->createIndirect( dlgTemplate, NULL );
	, ; );

	virtual LRESULT __cdecl onMessage( MsgOpt& opt ){
		WORD netEvent = WSAGETSELECTEVENT( opt._lParam );
		WORD netError = WSAGETSELECTERROR( opt._lParam );

		unsigned int idx = opt._message - WM_SOCKET;
		if( IsOutOf( idx, 0, _maxCount ) )
			return this->Panel::onMessage( opt );

		Entry* entry = this->fetch( idx );
		if( NULL == entry )
			return 0;

		Socket& socket = entry->_socket;
		if( socket != (SOCKET)opt._wParam )
			return 0;

		if( ERROR_SUCCESS != netError ){
			this->onError( *entry, netEvent, netError );
			return 0;
		}

		switch( netEvent ){

		case FD_READ:
			if( socket.isTcp() )
				this->onRecvTcp( *entry );
			if( socket.isUdp() )
				this->onRecvUdp( *entry );
			break;

		case FD_WRITE:
			if( socket.isTcp() )
				this->onSendTcp( *entry );
			if( socket.isUdp() )
				this->onSendUdp( *entry );
			break;

		case FD_ACCEPT:
			this->onAccept( *entry );
			break;

		case FD_CLOSE:
			this->Selector::onClose( *entry );
			break;
		}
		return 0;
	}

	virtual bool observe( Socket& socket ){
		if( !this->isCreated() )
			return false;

		long netEvent = ( FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE );

		//TODO: Move this Function to Other Place.
		socket.setRecvBufferSize( BufferSize_64K );
		socket.setSendBufferSize( BufferSize_64K );
		//socket.setNoDelay( true );

		Entry* entry = this->find( socket );

		// Bind the Socket to the HWND with Network Events.
		return( SOCKET_ERROR != ::WSAAsyncSelect( socket, *this, WM_SOCKET + entry->_index, netEvent ) );
	}

	Entry* find( SOCKET socket ){

		if( this->isEmpty() || Socket::GetNullHandle() == socket )
			return NULL;

		for( int i = 0; i < _maxCount; i++ ){
			Entry& entry = (*this)[i];
			if( _isAcquiredSet[i] == false )
				continue;

			if( socket != entry._socket )
				continue;

			return &entry;
		}
		return NULL;
	}
};

}}}
// ===================================Namespace Tail==========================================

#endif
