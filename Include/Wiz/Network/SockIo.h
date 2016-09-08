// ============================================================
// @Brief: Winsock IO Control API
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_NETWORK_SOCKIO_H__
#define __WIZ_NETWORK_SOCKIO_H__

#include "Wiz/Network/Socket.h"

#ifndef _WIN32_WCE

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Network{ namespace Socket{

// ============================================================
template< unsigned int MaxCount >
class SocketPool : protected Pool< Context, MaxCount >{

protected:
	unsigned int _bufferSizePerCtx;

	virtual bool onErrorReturn( Context& context, WORD netEvent, WORD netError ){
		return true;
	}

	virtual bool onErrorSocket( Context& context, WORD netEvent, WORD netError ){
		return true;
	}

	virtual bool onAccept( Context& context ){
		return true;
	}

	virtual bool onClose( Context& context ){
		return true;
	}

	virtual bool onRecvTcp( Context& context ){
		return true;
	}

	virtual bool onRecvUdp( Context& context ){
		return true;
	}

	virtual bool onSendTcp( Context& context ){
		return true;
	}

	virtual bool onSendUdp( Context& context ){
		return true;
	}

	virtual bool selectSocket( Socket& socket ){
		return true;
	}

	virtual bool ignoreSocket( Socket& socket ){
		return true;
	}

	Context* acquire(){
		Context* context = this->Pool::acquire();
		if( NULL == context )
			return NULL;

		Reconstruct( context );

		context->_recvBuffer.createBuffer( _bufferSizePerCtx );
		return context;
	}

	bool release( Context* context ){

		if( NULL == context )
			return false;

		this->ignoreSocket( context->_socket );
		Reconstruct( context );

		return this->Pool::release( context );
	}

	Context* find( SOCKET socket ){

		if( this->isEmpty() || Socket::GetNullHandle() == socket )
			return NULL;

		for( int i = 0; i < _maxCount; i++ ){

			if( (*this)[i]._isAcquired == false )
				continue;

			if( (*this)[i]._entry._socket != socket )
				continue;

			return &(*this)[i]._entry;
		}
		return NULL;
	}

public:
	Socket* acquireS(){
		Context* context = this->acquire();
		if( NULL == context )
			return NULL;
		return &context->_socket;
	}

	bool release( Socket* socket ){

		if( NULL == socket )
			return false;

		Context* context = this->find( *socket );
		if( NULL == context )
			return false;

		return this->release( context );
	}
};

// ============================================================
// @Brief: Message-Based Socket Controller
// ============================================================
template< unsigned int MaxCount >
class SocketWindow : public Dialog, public SocketPool< MaxCount >{

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
	DEFINE_SINGLE_EX( SocketWindow,
		DLGTEMPLATE dlgTemplate = {0};
		this->createIndirect( dlgTemplate, NULL );
		this->onMessage( &SocketWindow::onSocketMessage, WM_SOCKET );
	, ; );

	AsyncQueue< IoTask, _maxCount > _recvTaskQueue;

	typedef Array< Buffer, _maxCount > BufferArray;
	BufferArray _bufferSet;
	typename BufferArray::Indexer _bufferIdx;

	bool __cdecl onSocketMessage( WPARAM wParam, LPARAM lParam ){
		SOCKET socket = (SOCKET)wParam;
		WORD netEvent = WSAGETSELECTEVENT( lParam );
		WORD netError = WSAGETSELECTERROR( lParam );

		Context* context = this->find( socket );
		if( NULL == context )
			return true;

		bool ret = false;
		do{
			if( netError != ERROR_SUCCESS ){
				ret = this->onErrorSocket( *context, netEvent, netError );
				break;
			}

			switch( netEvent ){

			case FD_READ:
				if( context->_socket.isTcp() )
					ret = this->onRecvTcp( *context );

				if( context->_socket.isUdp() )
					ret = this->onRecvUdp( *context );
				break;

			case FD_WRITE:
				if( context->_socket.isTcp() )
					ret = this->onSendTcp( *context );

				if( context->_socket.isUdp() )
					ret = this->onSendUdp( *context );
				break;

			case FD_ACCEPT:
				ret = this->onAccept( *context );
				break;

			case FD_CLOSE:
				ret = this->onClose( *context );
				break;

			default:
				break;
			}
		}while( 0 );

		if( !ret )
			this->onErrorReturn( *context, netEvent, netError );

		return true;
	}

	virtual bool onAccept( Context& context ){
		Context* acceptContext = this->acquire();
		if( NULL == acceptContext )
			return false;

		SockAddr sockAddr;
		bool ret = false;
		do{
			if( !context._socket.accept( acceptContext->_socket, sockAddr ) )
				break;

			//TODO: Don't selectSocket because Accepted Socket is forked from Listen Socket.
			if( !this->selectSocket( acceptContext->_socket ) )
				break;

			ret = true;
		}while(0);

		if( !ret )
			this->release( acceptContext );

		Text64<> text;
		sockAddr.getHost( text, text._maxCount );
		this->sendAllTargetString( _T("onAccept: Remote Address = %s."), &text );
		return ret;
	}

	virtual bool onClose( Context& context ){

		context._socket.shutdown( SD_SEND );
		::Sleep(1); // Delay for receive remaining data.

		Text64<> text;
		context._socket.getRemoteAddr().getHost( text, text._maxCount );
		this->sendAllTargetString( _T("onClose: Remote Address = %s."), &text );

		//TODO: FD_CLOSE will not signal when Client Socket is not close.
		return this->release( &context );
	}

	virtual bool selectSocket( Socket& socket ){
		long netEvent = ( FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE );

		//TODO: Move this Function to Other Place.
		socket.setRecvBufferSize( BufferSize_64K );
		socket.setSendBufferSize( BufferSize_64K );
		//socket.setNoDelay( true );

		// Bind the Socket to the HWND with Network Events.
		return( SOCKET_ERROR != ::WSAAsyncSelect( socket, *this, WM_SOCKET, netEvent ) );
	}

	virtual bool ignoreSocket( Socket& socket ){
		// Unbind the Socket to the HWND with All Network Events
		return( SOCKET_ERROR != ::WSAAsyncSelect( socket, *this, 0, 0 ) );
	}

public:
	bool create( unsigned int bufferSizePerCtx ){

		_bufferSizePerCtx = bufferSizePerCtx;

		for( int i = 0; i < _maxCount; i++ )
			_bufferSet[i].createBuffer( _bufferSizePerCtx );

		return true;
	}

	bool pushTask( unsigned int id ){
		IoTask ioTask = {0};
		ioTask._id = id;
		ioTask._ioType = IoType_Task;
		return _recvTaskQueue.push( ioTask );
	}

	bool pushRecvTaskTcp( unsigned int id, Socket* socket, void* data, unsigned int len ){
		Buffer& buffer = this->getBuffer();
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
		Buffer& buffer = this->getBuffer();
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

	Buffer& getBuffer(){
		Buffer& buffer = _bufferSet[ _bufferIdx ];
		++_bufferIdx;
		return buffer;
	}
};

}}}
// ===================================Namespace Tail==========================================

#endif

#endif
