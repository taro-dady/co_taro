

#include "net/tcp_server.h"
#include "base/utils/defer.h"
#include "net/impl/windows/tcp_server_raw.h"

NAMESPACE_TARO_NET_BEGIN

TcpServerRaw::TcpServerRaw()
    : socket_( INVALID_SOCKET )
    , fn_ctx_( nullptr )
    , co_evt_( new IocpAcceptEvent )
{

}

TcpServerRaw::~TcpServerRaw()
{
    if ( socket_ == INVALID_SOCKET )
    {
        return;
    }

    delete co_evt_;
    ::closesocket( socket_ );
    fn_ctx_ = nullptr;
    co_evt_ = nullptr;
}

bool TcpServerRaw::init( SOCKET socket )
{
    DWORD dwBytes;
    GUID guidAcceptEx = WSAID_ACCEPTEX;
    if (WSAIoctl( socket,
                  SIO_GET_EXTENSION_FUNCTION_POINTER,
                  &guidAcceptEx,
                  sizeof(guidAcceptEx),
                  &fn_ctx_,
                  sizeof( fn_ctx_ ),
                  &dwBytes,
                  NULL,
                  NULL ) != 0)
    {
        NET_ERROR << "ioctrl failed. error:" << WSAGetLastError();
        return false;
    }

    if ( NULL == CreateIoCompletionPort( ( HANDLE )socket, get_poller(), 0, 0 ) )
    {
        NET_ERROR << "bind iocp failed. error:" << WSAGetLastError();
        return false;
    }
    return true;
}

void TcpServerRaw::post_accept_evt( IocpAcceptEvent* io_data )
{
    io_data->client = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED );

    DWORD bytes;
    BOOL ret = fn_ctx_( socket_,
                        io_data->client,
                        io_data->buf,
                        0,
                        sizeof( SOCKADDR_IN ) + 16,
                        sizeof( SOCKADDR_IN ) + 16,
                        &bytes,
                        ( OVERLAPPED* )io_data );
    if ( !ret && ( ERROR_IO_PENDING != WSAGetLastError() ) )
    {
        NET_ERROR << "post accept event failed. err:" << GetLastError();
    }
}

bool TcpServerRaw::listen( const char* ip, uint16_t port, bool reuse )
{
    if ( nullptr == ip || strlen( ip ) == 0 )
    {
        set_errno( TARO_ERR_INVALID_ARG );
        NET_ERROR << "parameter invalid";
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

    if ( !init( fd ) )
    {
        set_errno( TARO_ERR_FAILED );
        NET_ERROR << "init failed.";
        return false;
    }

    socket_ = fd;
    return true;
}

bool TcpServerRaw::accept_socket( SOCKET& socket )
{
    new ( co_evt_ ) IocpAcceptEvent();
    post_accept_evt( co_evt_ );
    co_evt_->wait();
    
    setsockopt( co_evt_->client, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, ( char* )&socket_, sizeof( socket_ ) );
    socket = co_evt_->client;
    return true;
}

TcpClientSPtr TcpServerRaw::accept()
{
    SOCKET sock;
    if( !accept_socket( sock ) )
    {
        return nullptr;
    }
    return std::make_shared<TcpClientRaw>( sock );
}

NAMESPACE_TARO_NET_END

