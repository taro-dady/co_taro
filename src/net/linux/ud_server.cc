
#include "net/ud_server.h"
#include "net/impl/linux/ud_client_impl.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>

NAMESPACE_TARO_NET_BEGIN

struct UDServer::Impl
{
    Impl()
        : socket_( TARO_INVALID_SOCKET )
    {}

    int32_t        socket_;
    CoNetEventSPtr co_evt_;
};

UDServer::UDServer()
    : impl_( new Impl )
{

}

UDServer::~UDServer()
{
    if( impl_->socket_ != TARO_INVALID_SOCKET )
    {
        ::close( impl_->socket_ );
        impl_->socket_ = TARO_INVALID_SOCKET;
    }
}

bool UDServer::listen( std::string const& local )
{
    if ( local.empty() )
    {
        NET_ERROR << "invalid parameter";
        set_errno( TARO_ERR_INVALID_ARG );
        return false;
    }

    auto socket = ( int32_t )::socket( AF_UNIX, SOCK_STREAM, 0 );
    if( socket < 0 )
    {
        NET_ERROR << "create uds sock failed. path:" << local;
        return false;
    }

    struct sockaddr_un serun;
    memset( &serun, 0, sizeof( serun ) );
    serun.sun_family = AF_UNIX;
    strcpy( serun.sun_path, local.c_str() );
    auto size = offsetof( struct sockaddr_un, sun_path ) + strlen( serun.sun_path );
    unlink( local.c_str() );

    if( ::bind( socket, ( struct sockaddr * )&serun, size ) < 0 )
    {
        ::close( socket );
        set_errno( TARO_ERR_FAILED );
        NET_ERROR << "bind uds sock failed. error:" << errno << " path:" << local;
        return false;
    }

    if( ::listen( socket, SOMAXCONN ) < 0 )
    {
        ::close( socket );
        set_errno( TARO_ERR_FAILED );
        NET_ERROR << "listen tcp sock failed.";
        return false;
    }

    impl_->socket_ = socket;
    impl_->co_evt_ = std::make_shared<CoNetEvent>( socket, eNetEvtRead );
    return true;
}

UDClientSPtr UDServer::accept()
{
    if ( impl_->socket_ == TARO_INVALID_SOCKET )
    {
        set_errno( TARO_ERR_INVALID_ARG );
        NET_ERROR << "listen first.";
        return nullptr;
    }
    impl_->co_evt_->wait();

    struct sockaddr_un clientun;
    int32_t client_addr_len = sizeof( struct sockaddr_un );
    int32_t clientfd = ( int32_t )::accept( impl_->socket_, ( struct sockaddr* )&clientun, ( socklen_t* )&client_addr_len );
    if( clientfd < 0 )
    {
        NET_ERROR << "accept failed.";
        return nullptr;
    }
    return UDClientImpl::create_cli( clientfd );
}

NAMESPACE_TARO_NET_END
