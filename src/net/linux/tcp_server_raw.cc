
#include "base/utils/arg_expander.h"
#include "net/impl/linux/tcp_client_raw.h"
#include "net/impl/linux/tcp_server_raw.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <string.h>

NAMESPACE_TARO_NET_BEGIN

TcpServerRaw::TcpServerRaw()
    : socket_( TARO_INVALID_SOCKET )
{

}

bool TcpServerRaw::listen( const char* ip, uint16_t port, bool reuse )
{
    if ( !STRING_CHECK( ip ) )
    {
        set_errno( TARO_ERR_INVALID_ARG );
        NET_ERROR << "parameter invalid";
        return false;
    }

    if ( socket_ >= 0 )
    {
        set_errno( TARO_ERR_MULTI_OP );
        NET_ERROR << "tcp server started";
        return false;
    }

    auto fd = ::socket( AF_INET, SOCK_STREAM, 0 );

    struct sockaddr_in svr_addr;
    memset( &svr_addr, 0, sizeof( svr_addr ) );
    svr_addr.sin_family      = PF_INET;
    svr_addr.sin_port        = htons( port );
    svr_addr.sin_addr.s_addr = ::inet_addr( ip );

    if ( reuse )
    {
        int32_t optval = 1;
        if( 0 != ::setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, ( char* )&optval, sizeof( optval ) ) )
        {
            set_errno( TARO_ERR_FAILED );
            NET_ERROR << "set socket option failed. error:" << errno;
            return false;
        }
    }

    if( ::bind( fd, ( struct sockaddr* )&svr_addr, sizeof( struct sockaddr ) ) < 0 )
    {
        set_errno( TARO_ERR_FAILED );
        NET_ERROR << "bind tcp sock failed.";
        return false;
    }

    if( ::listen( fd, SOMAXCONN ) < 0 )
    {
        set_errno( TARO_ERR_FAILED );
        NET_ERROR << "listen tcp sock failed.";
        return false;
    }

    socket_ = fd;
    co_evt_ = std::make_shared<CoNetEvent>( socket_, eNetEvtRead );
    return true;
}

TcpClientSPtr TcpServerRaw::accept()
{
    int32_t sock;
    if( !accept_socket( sock ) )
    {
        return nullptr;
    }
    return std::make_shared<TcpClientRaw>( sock );
}

bool TcpServerRaw::accept_socket( int32_t& socket )
{
    if ( socket_ == TARO_INVALID_SOCKET )
    {
        set_errno( TARO_ERR_INVALID_RES );
        NET_ERROR << "listen first.";
        return false;
    }
    
    co_evt_->wait();
    
    struct sockaddr_in clientun;
    int32_t client_addr_len = sizeof( struct sockaddr_in );
    int32_t clientfd = ( int32_t )::accept( socket_, ( struct sockaddr* )&clientun, ( socklen_t* )&client_addr_len );
    if( clientfd < 0 )
    {
        set_errno( TARO_ERR_FAILED );
        NET_ERROR << "accept failed.";
        return false;
    }
    socket = clientfd;
    return true;
}

NAMESPACE_TARO_NET_END
