
#include "base/utils/defer.h"
#include "net/impl/linux/common.h"
#include "net/impl/linux/tcp_client_raw.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>

NAMESPACE_TARO_NET_BEGIN

TcpClientRaw::TcpClientRaw()
    : socket_( TARO_INVALID_SOCKET )
    , support_connect_( true )
{
    
}

TcpClientRaw::TcpClientRaw( int32_t socket )
    : socket_( socket )
    , support_connect_( false )
    , co_evt_( std::make_shared<CoNetEvent>( socket ) )
{

}

TcpClientRaw::~TcpClientRaw()
{
    printf("TcpClientRaw::~TcpClientRaw\n");
    clear();
}

bool TcpClientRaw::connect( std::string const& ip, uint16_t port )
{
    if ( ip == "" )
    {
        set_errno( TARO_ERR_INVALID_ARG );
        NET_ERROR << "ip invalid";
        return false;
    }

    if ( !support_connect_ )
    {
        set_errno( TARO_ERR_NOT_SUPPORT );
        NET_ERROR << "not support";
        return false;
    }

    if ( socket_ != TARO_INVALID_SOCKET )
    {
        set_errno( TARO_ERR_MULTI_OP );
        NET_ERROR << "connected";
        return false;
    }

    auto sock = ::socket( AF_INET, SOCK_STREAM, 0 );
    ( void )set_fdnoblock( sock );
    
    struct sockaddr_in cli;
    cli.sin_family      = AF_INET;
    cli.sin_port        = htons( port );
    cli.sin_addr.s_addr = ::inet_addr( ip.c_str() );

    int32_t res = ::connect( sock, ( struct sockaddr* )&cli, sizeof( cli ) );
    if( 0 == res )
    {
        socket_ = sock;
        return true;
    }

    if( errno != EINPROGRESS )
    {
        set_errno( TARO_ERR_FAILED );
        NET_ERROR << "connect " << ip << ":" << port << " failed";
        return false;
    }

    co_evt_ = std::make_shared<CoNetEvent>( sock );
    co_evt_->wait( eNetEvtWrite );

    if ( eNetEvtWrite != co_evt_->evt_type() )
    {
        close( sock );
        co_evt_.reset();
        set_errno( TARO_ERR_FAILED );
        NET_ERROR << "connect " << ip << ":" << port << " failed";
        return false;
    }

    int32_t   err_num    = -1;
    socklen_t retval_len = sizeof( errno );
    if( getsockopt( sock, SOL_SOCKET, SO_ERROR, ( char* )&err_num, &retval_len ) < 0 || err_num != 0 )
    {
        close( sock );
        co_evt_.reset();
        set_errno( TARO_ERR_FAILED );
        NET_ERROR << "connect " << ip << ":" << port << " failed";
        return false;
    }

    socket_ = sock;
    return true;
}

int32_t TcpClientRaw::send( char* buffer, size_t bytes )
{
    if( nullptr == buffer || 0 == bytes )
    {
        NET_ERROR << "parameter invalid";
        return TARO_ERR_INVALID_ARG;
    }

    if ( socket_ == TARO_INVALID_SOCKET )
    {
        NET_ERROR << "socket invalid";
        return TARO_ERR_INVALID_RES;
    }

    co_evt_->wait( eNetEvtWrite );

    Defer defer([&](){ clear(); });
    if ( eNetEvtWrite != co_evt_->evt_type() )
    {
        NET_ERROR << "exception happen type:" << co_evt_->evt_type();
        return TARO_ERR_DISCONNECT;
    }

    auto ret = ::send( socket_, buffer, bytes, 0 );
    if ( ret <= 0 )
    {
        NET_ERROR << "send error";
        return is_disconnect( ret ) ? TARO_ERR_DISCONNECT : TARO_ERR_FAILED;
    }
    defer = false;
    return ret;
}

int32_t TcpClientRaw::recv( int32_t ms, DynPacketSPtr& packet )
{
    if ( socket_ == TARO_INVALID_SOCKET )
    {
        NET_ERROR << "socket invalid";
        return TARO_ERR_INVALID_RES;
    }

    co_evt_->wait( eNetEvtRead, ms > 0 ? ms : 0 );
    if ( eNetEvtTimeout == co_evt_->evt_type() )
    {
        NET_DEBUG << "receive time out";
        return TARO_ERR_TIMEOUT;
    }
    else if( eNetEvtRead != co_evt_->evt_type() )
    {
        clear();
        NET_TRACE << "socket disconnect";
        return TARO_ERR_DISCONNECT;
    }

    // get readable bytes
    unsigned long bytes = 0;
    ::ioctl( socket_, FIONREAD, &bytes );
    if( bytes == 0 )
    {
        clear();
        NET_TRACE << "socket disconnect";
        return TARO_ERR_DISCONNECT;
    }

    packet = create_default_packet( bytes );
    auto ret = ::recv( socket_, ( char* )packet->buffer(), bytes, 0 );
    if ( ret <= 0 )
    {
        clear();
        return is_disconnect( ret ) ? TARO_ERR_DISCONNECT : TARO_ERR_FAILED;
    }
    packet->resize( bytes );
    return bytes;
}

int32_t TcpClientRaw::recv( char* buffer, size_t bytes, int32_t ms )
{
    if( nullptr == buffer || 0 == bytes )
    {
        NET_ERROR << "parameter invalid";
        return TARO_ERR_INVALID_ARG;
    }

    if ( socket_ == TARO_INVALID_SOCKET )
    {
        NET_ERROR << "socket invalid";
        return TARO_ERR_INVALID_RES;
    }

    co_evt_->wait( eNetEvtRead, ms > 0 ? ms : 0 );
    if ( eNetEvtTimeout == co_evt_->evt_type() )
    {
        NET_DEBUG << "receive time out";
        return TARO_ERR_TIMEOUT;
    }
    else if( eNetEvtRead != co_evt_->evt_type() )
    {
        clear();
        NET_TRACE << "socket disconnect";
        return TARO_ERR_DISCONNECT;
    }

    auto ret = ::recv( socket_, buffer, bytes, 0 );
    if ( ret <= 0 )
    {
        clear();
        return is_disconnect( ret ) ? TARO_ERR_DISCONNECT : TARO_ERR_FAILED;
    }
    return ret;
}

void TcpClientRaw::clear()
{
    if ( socket_ != TARO_INVALID_SOCKET )
    {
        co_evt_.reset();
        ::close( socket_ );
        socket_ = TARO_INVALID_SOCKET;
    }
}

IpAddr TcpClientRaw::get_peer_info() const
{
    if ( socket_ != TARO_INVALID_SOCKET )
    {
        set_errno( TARO_ERR_INVALID_ARG );
        return IpAddr();
    }

    struct sockaddr_in addr_in;
    socklen_t addrlen;
    addrlen = sizeof( addr_in );
    if( ::getpeername( socket_, ( struct sockaddr* )&addr_in, &addrlen ) < 0 )
    {
        set_errno( TARO_ERR_FAILED );
        NET_ERROR << "get remote peer name failed";
        return IpAddr();
    }

    return IpAddr( 
                   inet_ntoa( ( ( struct sockaddr_in* )&addr_in )->sin_addr ), 
                   ntohs( ( ( struct sockaddr_in* )&addr_in )->sin_port ) 
                 );
}

NAMESPACE_TARO_NET_END
