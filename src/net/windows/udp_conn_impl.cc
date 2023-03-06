
#include "base/utils/defer.h"
#include "net/impl/windows/udp_conn_impl.h"
#include <io.h>

#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
#endif

NAMESPACE_TARO_NET_BEGIN

UdpConnImpl::UdpConnImpl()
    : socket_( ::socket( AF_INET, SOCK_DGRAM, 0 ) )
    , group_setting_( nullptr )
    , co_evt_( nullptr )
{

}

UdpConnImpl::~UdpConnImpl()
{
    if ( nullptr != co_evt_ )
    {
        ::free( co_evt_ );
        co_evt_ = nullptr;
    }

    if ( socket_ != INVALID_SOCKET )
    {
        ::closesocket( socket_ );
        socket_ = INVALID_SOCKET;
    }

    if ( nullptr != group_setting_ )
    {
        delete group_setting_;
        group_setting_ = nullptr;
    }
}

bool UdpConnImpl::bind( IpAddr const& server, bool reuse )
{
    if ( !server.valid() )
    {
        NET_ERROR << "invalid address";
        return false;
    }

    int32_t optval = reuse ? 1 : 0;
    if ( 0 != setsockopt( socket_, SOL_SOCKET, SO_REUSEADDR, ( char* )&optval, sizeof( optval ) ) )
    {
        NET_ERROR << "set socket option failed. reuse:" << reuse;
        return false;
    }

    struct sockaddr_in server_addr;
    memset( &server_addr, 0, sizeof( server_addr ) );
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons( server.port );
    server_addr.sin_addr.s_addr = ::inet_addr( server.ip.c_str() );
    if ( ::bind( socket_, ( struct sockaddr* )&server_addr, sizeof( struct sockaddr ) ) == -1 )
    {
        NET_ERROR << "bind error!";
        return false;
    }
    return true;
}

int32_t UdpConnImpl::sendto( IpAddr const& remote, char* buffer, size_t bytes )
{
    if( nullptr == buffer || 0 == bytes || !remote.valid() )
    {
        NET_ERROR << "parameter invalid";
        return TARO_ERR_INVALID_ARG;
    }

    init_iocp();
    new ( co_evt_ ) IocpUdpWriteEvent( buffer, bytes, remote );
    IocpUdpWriteEvent* evt = ( IocpUdpWriteEvent* )co_evt_;
    if ( !post_write_evt( evt ) )
    {
        NET_ERROR << "post write event failed";
        return TARO_ERR_FAILED;
    }
    evt->wait();

    Defer defer([evt](){ evt->~IocpUdpWriteEvent(); });
    if ( eNetEvtWrite != evt->type )
    {
        NET_ERROR << "send error";
        return TARO_ERR_FAILED;
    }
    return ( int32_t )evt->bytes;
}

int32_t UdpConnImpl::recvfrom( char* buffer, size_t bytes, IpAddr& remote, int32_t ms )
{
    if ( nullptr == buffer || 0 == bytes )
    {
        NET_ERROR << "parameter invalid";
        return TARO_ERR_INVALID_ARG;
    }

    init_iocp();

    Defer defer;
    uint32_t wait_time = 0;
    IocpUdpReadEevent* evt;
    if( ms > 0 )
    {
        // 等待延时为了解决超时问题
        wait_time = ms;
        evt = new IocpUdpReadEevent( buffer, bytes );
        defer.set( [evt](){ delete evt; } );
    }
    else
    {
        // 无限等待复用event对象
        new ( co_evt_ ) IocpUdpReadEevent( buffer, bytes );
        evt = ( IocpUdpReadEevent* )co_evt_;
        defer.set( [evt](){ evt->~IocpUdpReadEevent(); } );
    }

    if ( !post_read_evt( evt ) )
    {
        NET_ERROR << "post read event failed";
        return TARO_ERR_FAILED;
    }
    evt->wait( wait_time );

    if ( eNetEvtTimeout == evt->type )
    {
        defer = false;
        CancelIoEx( ( HANDLE )socket_, evt );
        return TARO_ERR_TIMEOUT;
    }

    if ( eNetEvtRead != evt->type )
    {
        NET_ERROR << "read error";
        return TARO_ERR_FAILED;
    }

    remote.ip   = inet_ntoa( evt->remote_addr.sin_addr );
    remote.port = ntohs( evt->remote_addr.sin_port );
    return ( int32_t )evt->bytes;
}

bool UdpConnImpl::set_group_loop( bool loop )
{
    int32_t opt = loop ? 1 : 0;
    if ( 0 != setsockopt( socket_, IPPROTO_IP, IP_MULTICAST_LOOP, ( char* )&opt, sizeof( opt ) ) )
    {
        NET_ERROR << "set socket loop option failed. loop:" << ( loop ? "true" : "false" );
        return false;
    }
    return true;
}

bool UdpConnImpl::join_group( uint16_t local_port, std::string const& group )
{
    if ( !UdpConnImpl::bind( IpAddr( "0.0.0.0", local_port ), true ) )
    {
        NET_ERROR << "bind error, port:" << local_port;
        return false;
    }
    
    if ( group_setting_ != nullptr )
    {
        delete group_setting_;
    }
    group_setting_ = new struct ip_mreq;

    memset( ( void* )group_setting_, 0, sizeof( struct ip_mreq ) );
    group_setting_->imr_interface.s_addr = inet_addr( "0.0.0.0" );
    group_setting_->imr_multiaddr.s_addr = inet_addr( group.c_str() );
    if ( 0 != setsockopt( socket_, IPPROTO_IP, IP_ADD_MEMBERSHIP, ( char* )group_setting_, sizeof( struct ip_mreq ) ) )
    {
        delete group_setting_;
        group_setting_ = nullptr;
        NET_ERROR << "set socket add member option failed. group:" << group;
        return false;
    }
    return true;
}

bool UdpConnImpl::leave_group()
{
    if ( group_setting_ == nullptr )
    {
        NET_ERROR << "no group joined";
        return false;
    }

    if ( 0 != setsockopt( socket_, IPPROTO_IP, IP_DROP_MEMBERSHIP, ( char* )group_setting_, sizeof( struct ip_mreq ) ) )
    {
        NET_ERROR << "set socket option failed.";
        return false;
    }

    delete group_setting_;
    group_setting_ = nullptr;
    return true;
}

std::shared_ptr<UdpConn> UdpConnImpl::copy()
{
    auto conn = std::make_shared<UdpConnImpl>();
    conn->socket_ = _get_osfhandle( _dup( _open_osfhandle( socket_, 0 ) ) );
    return conn;
}

void UdpConnImpl::init_iocp()
{
    if ( nullptr != co_evt_ )
    {
        return;
    }

    co_evt_ = ::malloc( sizeof( IocpUdpReadEevent ) );
    if( NULL == CreateIoCompletionPort( ( HANDLE )socket_, get_poller(), ( ULONG_PTR )socket_, 0 ) )
    {
        NET_ERROR << "bind iocp failed";
    }
}

bool UdpConnImpl::post_read_evt( IocpUdpReadEevent* data )
{
    if ( nullptr == data || nullptr == data->wrap.buf || 0 == data->wrap.len )
    {
        NET_ERROR << "data is invalid";
        return false;
    }

    DWORD nBytes = ( DWORD )data->wrap.len;
    DWORD dwFlags = 0;
    int nRet = WSARecvFrom( 
                            socket_,
                            &data->wrap,
                            1,
                            &nBytes,
                            &dwFlags,
                            ( SOCKADDR* )&( data->remote_addr ),
                            &( data->remote_len ),
                            ( OVERLAPPED* )data,
                            NULL 
                            );
    if ( nRet == SOCKET_ERROR && ( ERROR_IO_PENDING != WSAGetLastError() ) )
    {
        NET_ERROR << "post udp receive event failed. err:" << GetLastError();
        return false;
    }
    return true;
}   

bool UdpConnImpl::post_write_evt( IocpUdpWriteEvent* data )
{
    if ( nullptr == data
        || nullptr == data->wrap.buf
        || 0 == data->wrap.len )
    {
        NET_ERROR << "write data is invalid.";
        return false;
    }

    DWORD flags = 0;
    DWORD bytes = ( DWORD )data->wrap.len;
    int32_t ret = WSASendTo(
                            socket_,
                            &data->wrap,
                            1,
                            &bytes,
                            flags,
                            ( SOCKADDR* )&data->remote_addr,
                            sizeof( data->remote_addr ),
                            ( OVERLAPPED* )data,
                            NULL 
                            );

    if ( ret == SOCKET_ERROR && ( ERROR_IO_PENDING != WSAGetLastError() ) )
    {
        NET_ERROR << "post udp send event failed. err:" << GetLastError();
        return false;
    }
    return true;
}

extern UdpConnSPtr create_udp_conn()
{
    return std::make_shared<UdpConnImpl>();
}

NAMESPACE_TARO_NET_END
