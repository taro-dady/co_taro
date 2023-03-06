
#include "base/utils/defer.h"
#include "net/impl/linux/common.h"
#include "net/impl/linux/ud_client_impl.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>

NAMESPACE_TARO_NET_BEGIN

UDClientImpl::UDClientImpl()
    : socket_( TARO_INVALID_SOCKET )
    , support_connect_( true )
{

}

UDClientImpl::~UDClientImpl()
{
    clear();
}

void UDClientImpl::clear()
{
    if ( socket_ != TARO_INVALID_SOCKET )
    {
        co_evt_.reset();
        ::close( socket_ );
        socket_ = TARO_INVALID_SOCKET;
    }
}

UDClientSPtr UDClientImpl::create_cli( int32_t socket )
{
    auto cli = std::make_shared< UDClient >();
    cli->impl_->socket_          = socket;
    cli->impl_->support_connect_ = false;
    cli->impl_->co_evt_          = std::make_shared<CoNetEvent>( socket );
    return cli;
}

UDClient::UDClient()
    : impl_( new UDClientImpl )
{

}

UDClient::~UDClient()
{
    
}

bool UDClient::connect( std::string const& remote, std::string const& self )
{
    if ( remote.empty() )
    {
        NET_ERROR << "parameter invalid";
        set_errno( TARO_ERR_INVALID_ARG );
        return false;
    }

    if ( !impl_->support_connect_ )
    {
        set_errno( TARO_ERR_NOT_SUPPORT );
        NET_ERROR << "not support";
        return false;
    }

    auto socket = ::socket( AF_UNIX, SOCK_STREAM, 0 );
    if( !self.empty() )
    {
        struct sockaddr_un cliun;
        memset( &cliun, 0, sizeof( cliun ) );
        cliun.sun_family = AF_UNIX;
        strcpy( cliun.sun_path, self.c_str() );
        int32_t len = offsetof( struct sockaddr_un, sun_path ) + strlen( cliun.sun_path );
        unlink( cliun.sun_path );

        if( ::bind( socket, ( struct sockaddr * )&cliun, len ) < 0 )
        {
            NET_ERROR  << "bind uds client failed. path:" << self;
        }
    }

    struct sockaddr_un serun;
    memset( &serun, 0, sizeof( serun ) );
    serun.sun_family = AF_UNIX;
    strcpy( serun.sun_path, remote.c_str() );
    auto len = offsetof( struct sockaddr_un, sun_path ) + strlen( serun.sun_path );
    auto ret = ::connect( socket, ( struct sockaddr* )&serun, len );
    if ( 0 == ret )
    {
        impl_->socket_ = socket;
        impl_->co_evt_ = std::make_shared<CoNetEvent>( socket );
        return true;
    }
    return false;
}

int32_t UDClient::send( char* buffer, size_t bytes )
{
    if( nullptr == buffer || 0 == bytes )
    {
        NET_ERROR << "parameter invalid";
        return TARO_ERR_INVALID_ARG;
    }

    if ( impl_->socket_ == TARO_INVALID_SOCKET )
    {
        NET_ERROR << "socket invalid";
        return TARO_ERR_INVALID_RES;
    }

    impl_->co_evt_->wait( eNetEvtWrite );

    Defer defer([&](){ impl_->clear(); });
    if ( eNetEvtWrite != impl_->co_evt_->evt_type() )
    {
        NET_ERROR << "exception happen type:" << impl_->co_evt_->evt_type();
        return TARO_ERR_DISCONNECT;
    }

    auto ret = ::send( impl_->socket_, buffer, bytes, 0 );
    if ( ret <= 0 )
    {
        NET_ERROR << "send error";
        return is_disconnect( ret ) ? TARO_ERR_DISCONNECT : TARO_ERR_FAILED;
    }
    defer = false;
    return ret;
}

int32_t UDClient::recv( char* buffer, size_t bytes, int32_t ms )
{
    if( nullptr == buffer || 0 == bytes )
    {
        NET_ERROR << "parameter invalid";
        return TARO_ERR_INVALID_ARG;
    }

    if ( impl_->socket_ == TARO_INVALID_SOCKET )
    {
        NET_ERROR << "socket invalid";
        return TARO_ERR_INVALID_ARG;
    }

    impl_->co_evt_->wait( eNetEvtRead, ms > 0 ? ms : 0 );
    if ( eNetEvtTimeout == impl_->co_evt_->evt_type() )
    {
        NET_DEBUG << "receive time out";
        return TARO_ERR_TIMEOUT;
    }
    else if( eNetEvtRead != impl_->co_evt_->evt_type() )
    {
        impl_->clear();
        NET_TRACE << "socket disconnect";
        return TARO_ERR_DISCONNECT;
    }

    auto ret = ::recv( impl_->socket_, buffer, bytes, 0 );
    if ( ret <= 0 )
    {
        impl_->clear();
        return is_disconnect( ret ) ? TARO_ERR_DISCONNECT : TARO_ERR_FAILED;
    }
    return ret;
}

std::string UDClient::get_peer_info() const
{
    if ( impl_->socket_ == TARO_INVALID_SOCKET )
    {
        set_errno( TARO_ERR_INVALID_ARG );
        NET_ERROR << "socket invalid";
        return "";
    }
    
    struct sockaddr_un addr_un;
    socklen_t addrlen = sizeof( addr_un );
    if( ::getpeername( impl_->socket_, ( struct sockaddr* )&addr_un, &addrlen ) < 0 )
    {
        set_errno( TARO_ERR_FAILED );
        NET_ERROR << "get remote peer name failed";
        return "";
    }
    return addr_un.sun_path;
}

NAMESPACE_TARO_NET_END
