
#include "base/utils/defer.h"
#include "net/impl/windows/tcp_client_raw.h"

NAMESPACE_TARO_NET_BEGIN

TcpClientRaw::TcpClientRaw()
    : socket_( INVALID_SOCKET )
    , support_connect_( true )
    , co_evt_( new IocpTypeEvent( eNetEvtWrite ) )
    , fn_ctx_( nullptr )
{
    
}

TcpClientRaw::TcpClientRaw( SOCKET socket )
    : socket_( socket )
    , support_connect_( false )
    , co_evt_( new IocpTypeEvent( eNetEvtInvalid ) )
    , fn_ctx_( nullptr )
{
    if( NULL == CreateIoCompletionPort( ( HANDLE )socket, get_poller(), ( ULONG_PTR )socket, 0 ) )
    {
        NET_ERROR << "bind iocp failed";
    }
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

    if ( socket_ != INVALID_SOCKET )
    {
        set_errno( TARO_ERR_MULTI_OP );
        NET_ERROR << "socket connected";
        return false;
    }
    
    sockaddr_in local_addr;
    ZeroMemory( &local_addr, sizeof( sockaddr_in ) );
    local_addr.sin_family = AF_INET;
    
    auto socket = ::WSASocket( AF_INET, SOCK_STREAM , IPPROTO_TCP , NULL , NULL , WSA_FLAG_OVERLAPPED );
    Defer defer( [socket](){ ::closesocket( socket ); } );

    if ( ::bind( socket, (sockaddr*)(&local_addr), sizeof(sockaddr_in) ) == SOCKET_ERROR )
    {
        set_errno( TARO_ERR_FAILED );
        NET_ERROR << "bind address:" << ip << ":" << port << " failed";
        return false;
    }

    if ( NULL == CreateIoCompletionPort( ( HANDLE )socket, get_poller(), ( ULONG_PTR )socket, 0 ) )
    {
        set_errno( TARO_ERR_FAILED );
        NET_ERROR << "bind iocp failed";
        return false;
    }

    auto evt = co_evt_;
    new ( evt ) IocpTypeEvent( eNetEvtWrite, nullptr, 0 );
    if ( !post_connect_evt( socket, ip.c_str(), port, evt ) )
    {
        set_errno( TARO_ERR_FAILED );
        NET_ERROR << "post connect event failed";
        return false;
    }
    evt->wait();

    if ( eNetEvtWrite != evt->type )
    {
        set_errno( TARO_ERR_FAILED );
        NET_ERROR << "connect failed";
        return false;
    }

    defer   = false;
    socket_ = socket;
    return true;
}

int32_t TcpClientRaw::send( char* buffer, size_t bytes )
{
    if( nullptr == buffer || 0 == bytes )
    {
        NET_ERROR << "parameter invalid";
        return TARO_ERR_INVALID_ARG;
    }

    if ( socket_ == INVALID_SOCKET )
    {
        clear();
        NET_ERROR << "socket invalid";
        return TARO_ERR_INVALID_RES;
    }

    new ( co_evt_ ) IocpTypeEvent( eNetEvtWrite, buffer, bytes );
    if ( !post_write_evt( co_evt_ ) )
    {
        NET_ERROR << "post write event failed";
        return TARO_ERR_DISCONNECT;
    }
    co_evt_->wait();

    if ( eNetEvtWrite != co_evt_->type )
    {
        clear();
        NET_ERROR << "disconnect";
        return TARO_ERR_DISCONNECT;
    }
    return ( int32_t )co_evt_->bytes;
}

int32_t TcpClientRaw::recv( char* buffer, size_t bytes, int32_t ms )
{
    if( nullptr == buffer || 0 == bytes )
    {
        NET_ERROR << "parameter invalid";
        return TARO_ERR_INVALID_ARG;
    }

    if ( socket_ == INVALID_SOCKET )
    {
        NET_ERROR << "socket invalid";
        return TARO_ERR_INVALID_RES;
    }

    uint32_t wait_time = 0;
    IocpTypeEvent* evt = nullptr;
    Defer defer( [&]()
    { 
        delete evt; 
    } );
    if( ms > 0 )
    {
        // 等待延时为了解决超时问题
        wait_time = ms;
        evt = new IocpTypeEvent( eNetEvtRead, buffer, bytes );
    }
    else
    {
        // 无限等待复用event对象
        defer = false;
        evt   = co_evt_;
        new ( evt ) IocpTypeEvent( eNetEvtRead, buffer, bytes );
    }

    if ( !post_read_evt( evt ) )
    {
        clear();
        NET_ERROR << "post read event failed";
        return TARO_ERR_DISCONNECT;
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
        clear();
        NET_ERROR << "disconnect";
        return TARO_ERR_DISCONNECT;
    }
    return ( int32_t )evt->bytes;
}

IpAddr TcpClientRaw::get_peer_info() const
{
    if ( socket_ == INVALID_SOCKET )
    {
        set_errno( TARO_ERR_INVALID_ARG );
        return IpAddr();
    }

    struct sockaddr_in addr_in;
    int32_t addrlen;
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

int32_t TcpClientRaw::recv( int32_t ms, DynPacketSPtr& packet )
{
    auto tmp_pack = create_default_packet( 512 );
    auto ret = TcpClientRaw::recv( ( char* )tmp_pack->buffer(), ( size_t )tmp_pack->capcity(), ms );
    if( ret > 0 )
    {
        packet = tmp_pack;
        packet->resize( ret );
    }
    return ret;
}

void TcpClientRaw::clear()
{
    if ( socket_ == INVALID_SOCKET )
    {
        return;
    }

    delete co_evt_;
    ::closesocket( socket_ );
    
    socket_ = INVALID_SOCKET;
    co_evt_ = nullptr;
    fn_ctx_ = NULL;
}

bool TcpClientRaw::post_connect_evt( SOCKET socket, const char* ip, uint16_t port, IocpEvent* evt )
{
    if ( nullptr == fn_ctx_ )
    {
        DWORD dwBytes = 0;
        GUID GuidConnectEx = WSAID_CONNECTEX;

        if ( 
            SOCKET_ERROR == WSAIoctl
                (
                    socket,
                    SIO_GET_EXTENSION_FUNCTION_POINTER,
                    &GuidConnectEx,
                    sizeof( GuidConnectEx ),
                    &fn_ctx_,
                    sizeof( fn_ctx_ ),
                    &dwBytes,
                    0,
                    0
                )
            )
        {
            NET_ERROR << "WSAIoctl is failed. Error code = " << WSAGetLastError();
            return false;
        }
    }

    sockaddr_in addrPeer;
    ZeroMemory( &addrPeer , sizeof (sockaddr_in ) );
    addrPeer .sin_family       = AF_INET ;
    addrPeer .sin_addr .s_addr = inet_addr ( ip );
    addrPeer .sin_port         = htons ( port );

    int nLen = sizeof (addrPeer );
    PVOID lpSendBuffer = NULL;
    DWORD dwSendDataLength = 0;
    DWORD dwBytesSent = 0;
    BOOL bResult = fn_ctx_
    ( 
        socket,
        (sockaddr *)&addrPeer ,  // [in] 对方地址
        nLen ,                   // [in] 对方地址长度
        lpSendBuffer ,           // [in] 连接后要发送的内容，这里不用
        dwSendDataLength ,       // [in] 发送内容的字节数 ，这里不用
        &dwBytesSent ,           // [out] 发送了多少个字节，这里不用
        (OVERLAPPED *)evt
    ); 
    
    if ( !bResult )
    {
        if ( WSAGetLastError () != ERROR_IO_PENDING )   // 调用失败
        {
            NET_ERROR << "ConnextEx error:" << WSAGetLastError ();
            return false;
        }
        else
        {
            NET_ERROR << "socket connect pending";
        }
    }
    return true;
}

bool TcpClientRaw::post_read_evt( IocpTypeEvent* data )
{
    if ( nullptr == data
      || nullptr == data->wrap.buf
      || 0 == data->wrap.len )
    {
        NET_ERROR << "read data is invalid.";
        return false;
    }

    DWORD nBytes = data->wrap.len, dwFlags = 0;
    int ret = WSARecv( 
                        socket_,
                        &data->wrap,
                        1,
                        &nBytes,
                        &dwFlags,
                        ( OVERLAPPED* )data,
                        NULL 
                    );
    if ( ret == SOCKET_ERROR && ( ERROR_IO_PENDING != WSAGetLastError() ) )
    {
        NET_ERROR << "error:" << WSAGetLastError();
        return false;
    }
    return true;
}

bool TcpClientRaw::post_write_evt( IocpTypeEvent* data )
{
    if ( nullptr == data || nullptr == data->wrap.buf || 0 == data->wrap.len )
    {
        NET_ERROR << "write data is invalid.";
        return false;
    }

    DWORD flags = 0;
    DWORD bytes = data->bytes;
    int32_t ret = WSASend( 
                            socket_,
                            &data->wrap,
                            1,
                            &bytes,
                            flags,
                            ( OVERLAPPED* )data,
                            NULL 
                        );
    if ( ret == SOCKET_ERROR && ( ERROR_IO_PENDING != WSAGetLastError() ) )
    {
        NET_ERROR << "error:" << WSAGetLastError();
        return false;
    }
    return true;
}

NAMESPACE_TARO_NET_END
