// ============================================================
// @Brief: Classed Winsock API
// @Author: Blaine Luo
// @Date: 2016/08
// ============================================================
#ifndef __WIZ_NETWORK_SOCKET_H__
#define __WIZ_NETWORK_SOCKET_H__

#include "Wiz/Network/WinSockHeader.h"
#include "Wiz/Threading/Forward.h"
#include "Wiz/Windowing/Backward.h"

// ===================================Namespace Head==========================================
namespace Wiz{ namespace Network{ namespace Socket{

using namespace Wiz::Threading;
using namespace Wiz::Windowing;

enum{
	PortNumberMin = 0,
	PortNumberMax = 65535,
	TcpMtu = 1460,
	BufferSize_64K = 64 * 1024
};

// ============================================================
class Startup : public Structure< WSADATA >{
	DEFINE_SINGLE_EX( Startup,
		::WSAStartup( 0x0202, (Entry*)this );
	, ; );
};

// ============================================================
class InetAddr : public Structure< IN_ADDR >{
public:
	inline InetAddr& operator <<( DWORD ip ){
		(*this)->S_un.S_addr = ::htonl( ip );
		return *this;
	}

	template< typename T >
	inline InetAddr& operator <<( T* ip ){
		(*this)->S_un.S_addr = ::inet_addr( Text32< char >() << ip );
		return *this;
	}

	inline operator DWORD(){
		return ::ntohl( (*this)->S_un.S_addr );
	}

	inline operator char*(){
		return ::inet_ntoa( *this );
	}
};

// ============================================================
class SockAddr : public Structure< SOCKADDR_IN >{
public:
	inline operator sockaddr*(){
		return (sockaddr*)this;
	}

	inline SockAddr(){
		(*this)->sin_family = AF_INET;
	}

	inline SockAddr( PTCHAR host, WORD port ){
		this->SockAddr::SockAddr();
		this->setHost( host ).setPort( port );
	}

	inline DWORD getHost(){
		return ::ntohl( (*this)->sin_addr.S_un.S_addr );
	}

	inline bool getHost( PTCHAR host, DWORD strLen ){
		return( SOCKET_ERROR != ::WSAAddressToString( *this, _entrySize, NULL, host, &strLen ) );
	}

	inline WORD getPort(){
		return ::ntohs( (*this)->sin_port );
	}

	inline SockAddr& setHost( DWORD host ){
		(*this)->sin_addr.S_un.S_addr = ::htonl( host );
		return *this;
	}

	inline SockAddr& setHost( PTCHAR host ){
		int addrSize = _entrySize;
		::WSAStringToAddress( host, (*this)->sin_family, NULL, *this, &addrSize );
		return *this;
	}

	inline SockAddr& setPort( WORD port ){
		(*this)->sin_port = ::htons( port );
		return *this;
	}
};

// ============================================================
class MCastAddr : public Structure< ip_mreq >{
public:
	template< typename T >
	inline MCastAddr( T* groupIp, T* localIp = CONST_TEXT( T*, "0.0.0.0" ) ){
		this->setGroup( groupIp );
		this->setLocal( localIp );
	}

	inline InetAddr& getLocal(){
		return *(InetAddr*)&(*this)->imr_interface;
	}

	inline InetAddr& getGroup(){
		return *(InetAddr*)&(*this)->imr_multiaddr;
	}

	template< typename T >
	inline MCastAddr& setLocal( T* ip ){
		this->getLocal() << ip;
		return *this;
	}

	template< typename T >
	inline MCastAddr& setGroup( T* ip ){
		this->getGroup() << ip;
		return *this;
	}
};

// ============================================================
struct Protocol{
	int _addressFamily;
	int _socketType;
	int _protocolType;
	inline Protocol( int addressFamily = AF_INET, int socketType = SOCK_STREAM, int protocolType = 0 ) :
		_addressFamily( addressFamily ),
		_socketType( socketType ),
		_protocolType( protocolType )
	{}
};

// ============================================================
template< typename T >
struct Option{
	typedef T Value;
	int _level;
	int _name;
	Value _value;
	inline Option( int level, int name, Value& value ) :
		_level( level ),
		_name( name ),
		_value( value )
	{}
};

template< typename T >
struct OptionIp : Option< T >{
	inline OptionIp( int name, Value value = Value() ) : Option( IPPROTO_IP, name, value ){}
};

template< typename T >
struct OptionTcp : Option< T >{
	inline OptionTcp( int name, Value value = Value() ) : Option( IPPROTO_TCP, name, value ){}
};

template< typename T >
struct OptionSocket : Option< T >{
	inline OptionSocket( int name, Value value = Value() ) : Option( SOL_SOCKET, name, value ){}
};

// ============================================================
class Socket : public HandleT< SOCKET, INVALID_SOCKET >{

public:
	inline ~Socket(){
		if( this->isCreated() )
			this->close();
	}

	inline bool create( Protocol& protocol ){
		Reconstruct( this );
		this->setHandle( ::socket( protocol._addressFamily, protocol._socketType, protocol._protocolType ) );
		return this->isCreated();
	}

	inline bool close(){
#ifndef _WIN32_WCE
		//Cancel Overlapped Socket
		::CancelIo( (HANDLE)this->getHandle() );
#endif
		return( SOCKET_ERROR != ::closesocket( *this ) );
	}

	inline bool accept( Socket& acceptSocket, SockAddr& remoteSockAddr ){
		int sockAddrLen = sizeof(remoteSockAddr);
		acceptSocket.setHandle( ::accept( *this, remoteSockAddr, &sockAddrLen ) );
		return acceptSocket.isCreated();
	}

	inline bool bind( SockAddr& sockAddr ){
		// If Port not eq zero, then bind.
		if( PortNumberMin == sockAddr.getPort() )
			return true;
		return( SOCKET_ERROR != ::bind( *this, sockAddr, sizeof(sockAddr) ) );
	}

	inline bool connect( SockAddr& sockAddr ){
		return( SOCKET_ERROR != ::connect( *this, sockAddr, sizeof(sockAddr) ) );
	}

	inline bool listen( unsigned int backlog = SOMAXCONN ){
		return( SOCKET_ERROR != ::listen( *this, backlog ) );
	}

	inline int recvTcp( void* buffer, unsigned int len ){
		return ::recv( *this, (char*)buffer, len, 0 );
	}

	inline int recvUdp( void* buffer, unsigned int len, SockAddr& remoteSockAddr ){
		int sockAddrLen = sizeof(remoteSockAddr);
		return ::recvfrom( *this, (char*)buffer, len, 0, remoteSockAddr, &sockAddrLen );
	}

	inline int sendTcp( void* buffer, unsigned int len ){
		return ::send( *this, (char*)buffer, len, 0 );
	}

	inline bool sendTcpEx( void* buffer, unsigned int len ){
		unsigned int offset = 0;
		int ret = 0;
		do{
			ret = this->sendTcp( (char*)buffer + offset, len - offset );
			if( SOCKET_ERROR == ret )
				return false;
			offset += ret;
		}while( offset < len );
		return true;
	}

	// Send Packet without TCP Nagle's Algorithm.
	inline int sendTcpSleep( void* buffer, unsigned int len, unsigned int milliseconds = 1 ){
		::Sleep( milliseconds );
		return this->sendTcp( buffer, len );
	}

	inline int sendUdp( void* buffer, unsigned int len, SockAddr& remoteSockAddr ){
		return ::sendto( *this, (char*)buffer, len, 0, remoteSockAddr, sizeof(remoteSockAddr) );
	}

	inline bool shutdown( int shutdownType = SD_SEND ){
		return( SOCKET_ERROR != ::shutdown( *this, shutdownType ) );
	}

	inline bool getLocalAddr( SockAddr& sockAddr ){
		int size = sizeof(sockAddr);
		return( SOCKET_ERROR != ::getsockname( *this, sockAddr, &size ) );
	}

	inline bool getRemoteAddr( SockAddr& sockAddr ){
		int size = sizeof(sockAddr);
		return( SOCKET_ERROR != ::getpeername( *this, sockAddr, &size ) );
	}

	template< typename T >
	bool getLocalIp( T ip, unsigned int len ){
		SockAddr sockAddr;
		if( !this->getLocalAddr( sockAddr ) )
			return false;
		if( !sockAddr.getHost( ip, len ) )
			return false;
		return true;
	}

	template< typename T >
	bool getRemoteIp( T ip, unsigned int len ){
		SockAddr sockAddr;
		if( !this->getRemoteAddr( sockAddr ) )
			return false;
		if( !sockAddr.getHost( ip, len ) )
			return false;
		return true;
	}

	inline bool setNonBlockingMode( bool enable ){
		ULONG value = enable;
		return( SOCKET_ERROR != ::ioctlsocket( *this, FIONBIO, &value ) );
	}

	template< typename T >
	inline T& getOption( T& opt ){
		int len = sizeof(opt._value);
		::getsockopt( *this, opt._level, opt._name, (char*)&opt._value, &len );
		return opt;
	}

	template< typename T >
	inline bool setOption( T& opt ){
		return( SOCKET_ERROR != ::setsockopt( *this, opt._level, opt._name, (char*)&opt._value, sizeof(opt._value) ) );
	}

	inline bool getBroadcastMode(){
		return( 0 != this->getOption( OptionSocket< BOOL >( SO_BROADCAST ) )._value );
	}

	inline bool setBroadcastMode( bool enable ){
		return this->setOption( OptionSocket< BOOL >( SO_BROADCAST, enable ) );
	}

	inline bool setMulticastLoop( BOOL enable ){
		return this->setOption( OptionIp< BOOL >( IP_MULTICAST_LOOP, enable ) );
	}

	inline bool setMulticastTtl( DWORD ttl = 10 ){
		return this->setOption( OptionIp< DWORD >( IP_MULTICAST_TTL, ttl ) );
	}

	inline bool enterMulticastGroup( TCHAR* groupIp, TCHAR* localIp = _T("0.0.0.0") ){
		return this->setOption( OptionIp< MCastAddr >( IP_ADD_MEMBERSHIP, MCastAddr( groupIp, localIp ) ) );
	}

	inline bool leaveMulticastGroup( TCHAR* groupIp, TCHAR* localIp = _T("0.0.0.0") ){
		return this->setOption( OptionIp< MCastAddr >( IP_DROP_MEMBERSHIP, MCastAddr( groupIp, localIp ) ) );
	}

	inline unsigned int getConnectTime(){
		return this->getOption( OptionSocket< unsigned int >( SO_CONNECT_TIME ) )._value;
	}

	inline unsigned int getLinger(){
		return this->getOption( OptionSocket< linger >( SO_LINGER ) )._value.l_linger;
	}

	inline bool setLinger( bool enable, unsigned int second = 0 ){
		linger value = {0};
		value.l_onoff = enable;
		value.l_linger = second;
		return this->setOption( OptionSocket< linger >( SO_LINGER, value ) );
	}

	inline bool setNoDelay( BOOL enable ){
		return this->setOption( OptionTcp< BOOL >( TCP_NODELAY, enable ) );
	}

	inline unsigned int getRecvBufferSize(){
		return this->getOption( OptionSocket< unsigned int >( SO_RCVBUF ) )._value;
	}

	inline bool setRecvBufferSize( unsigned int size ){
		return this->setOption( OptionSocket< unsigned int >( SO_RCVBUF, size ) );
	}

	inline bool setSendBufferSize( unsigned int size ){
		return this->setOption( OptionSocket< unsigned int >( SO_SNDBUF, size ) );
	}

	inline bool setReuseAddress( BOOL enable ){
		return this->setOption( OptionSocket< BOOL >( SO_REUSEADDR, enable ) );
	}

	inline bool setSendTimeout( unsigned int milliseconds ){
		return this->setOption( OptionSocket< unsigned int >( SO_SNDTIMEO, milliseconds ) );
	}

	inline bool isListening(){
		return( 0 != this->getOption( OptionSocket< BOOL >( SO_ACCEPTCONN ) )._value );
	}

	inline int	getTransportType(){
		return this->getOption( OptionSocket< int >( SO_TYPE ) )._value;
	}

	inline bool isTcp(){
		return( this->getTransportType() == SOCK_STREAM );
	}

	inline bool isUdp(){
		return( this->getTransportType() == SOCK_DGRAM );
	}
};

// ============================================================
class SocketEx : public Socket{

protected:
	SockAddr _addrLocal;
	SockAddr _addrRemote;

public:
	typedef Socket Parent;

	inline SockAddr& getLocalAddr(){
		this->Parent::getLocalAddr( _addrLocal );
		return _addrLocal;
	}

	inline SockAddr& getRemoteAddr(){
		this->Parent::getRemoteAddr( _addrRemote );
		return _addrRemote;
	}

	inline SocketEx& setLocalAddr( SockAddr& sockAddr ){
		_addrLocal = sockAddr;
		return *this;
	}

	inline SocketEx& setRemoteAddr( SockAddr& sockAddr ){
		_addrRemote = sockAddr;
		return *this;
	}

	inline bool bind(){
		return this->Parent::bind( _addrLocal );
	}

	inline bool connect(){
		return this->Parent::connect( _addrRemote );
	}

	// WSA Specific Function.
	int wsaRecvTcp( void* buffer, UINT len, WSAOVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completionRoutine, void* param = NULL ){
		DWORD transferred = 0;
		DWORD flags = 0;
		WSABUF wsaBuf = {0};
		wsaBuf.buf = (char*)buffer;
		wsaBuf.len = len;

		if( overlapped == NULL || completionRoutine == NULL )
			return SOCKET_ERROR;

		overlapped->hEvent = (WSAEVENT)param; //Pass a parameter to the completion routine.

		if( SOCKET_ERROR == ::WSARecv( *this, &wsaBuf, 1, &transferred, &flags, overlapped, completionRoutine ) )
			return SOCKET_ERROR;

		return transferred;
	}

	int wsaSendTcp( void* buffer, UINT len, WSAOVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completionRoutine, void* param = NULL ){
		DWORD transferred = 0;
		DWORD flags = 0;
		WSABUF wsaBuf = {0};
		wsaBuf.buf = (char*)buffer;
		wsaBuf.len = len;

		if( overlapped == NULL || completionRoutine == NULL )
			return SOCKET_ERROR;

		overlapped->hEvent = (WSAEVENT)param; //Pass a parameter to the completion routine.

		if( SOCKET_ERROR == ::WSASend( *this, &wsaBuf, 1, &transferred, flags, overlapped, completionRoutine ) )
			return SOCKET_ERROR;

		return transferred;
	}
};

// ============================================================
struct Context{
	Socket _socket;
	Buffer _recvBuffer;
	Buffer _sendBuffer;
	Counter _aliveTime;
};

}}}
// ===================================Namespace Tail==========================================

#endif
