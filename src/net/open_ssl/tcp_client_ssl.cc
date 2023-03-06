
#include "net/impl/open_ssl/tcp_client_ssl.h"

NAMESPACE_TARO_NET_BEGIN

TcpClientSSL::TcpClientSSL( SSLContext const& ctx )
    : ssl_write_ret_( 0 )
    , ssl_conn_( ctx )
{
    ssl_conn_.set_write_cb
    ( 
        std::bind( &TcpClientSSL::write_data, 
                   this, 
                   std::placeholders::_1, 
                   std::placeholders::_2 ) 
    );
}

TcpClientSSL::TcpClientSSL( socket_t socket, SSL_CTX* ctx )
    : TcpClientRaw( socket )
    , ssl_write_ret_( 0 )
    , ssl_conn_( ctx )
{
    ssl_conn_.set_write_cb
    ( 
        std::bind( &TcpClientSSL::write_data, 
                   this, 
                   std::placeholders::_1, 
                   std::placeholders::_2 ) 
    );
}

TcpClientSSL::~TcpClientSSL()
{
    
}

bool TcpClientSSL::connect( std::string const& ip, uint16_t port )
{
    if ( !TcpClientRaw::connect( ip, port ) )
    {
        NET_ERROR << "connect failed ip:" << ip << " port:" << port;
        return false;
    }

    while( 1 )
    {
        // 客户首先发送握手包
        auto ret = ssl_conn_.handshake();
        if ( TARO_ERR_FAILED == ret )
        {
            clear();
            set_errno( TARO_ERR_FAILED );
            NET_ERROR << "hand shake failed";
            return false;
        }
        else if( TARO_OK == ret )
        {
            break;
        }

        // 等待对方发送回应包
        DynPacketSPtr packet;
        ret = TcpClientRaw::recv( 2000, packet );
        if ( ret <= 0 )
        {
            NET_ERROR << "receive error:" << errno;
            return false;
        }
        ssl_conn_.handle_read_event( packet );
    };
    return true;
}

bool TcpClientSSL::do_accept()
{
    while( 1 )
    {
        // 服务端等待客户端发送的请求包
        DynPacketSPtr packet;
        auto ret = TcpClientRaw::recv( 2000, packet );
        if ( ret <= 0 )
        {
            set_errno( TARO_ERR_FAILED );
            NET_ERROR << "receive error";
            return ret;
        }
        ssl_conn_.handle_read_event( packet );

        // 收到对端数据后继续发送协商包
        ret = ssl_conn_.handshake();
        if ( TARO_ERR_FAILED == ret )
        {
            clear();
            set_errno( TARO_ERR_STATE );
            NET_ERROR << "hand shake failed";
            return false;
        }
        else if( TARO_OK == ret )
        {
            break;
        }
    }
    return true;
}

int32_t TcpClientSSL::send( char* buffer, size_t bytes )
{
    if( nullptr == buffer || 0 == bytes )
    {
        NET_ERROR << "parameter invalid";
        return TARO_ERR_INVALID_ARG;
    }

    if ( !ssl_conn_.state() )
    {
        if ( support_connect_ )
        {
            NET_ERROR << "connect first";
            return TARO_ERR_FAILED; 
        }

        if ( !do_accept() )
        {
            NET_ERROR << "handshake failed";
            return TARO_ERR_FAILED;
        }
    }

    ssl_write_ret_ = 0;
    ssl_conn_.write( buffer, ( int32_t )bytes );
    return ( ssl_write_ret_ > 0 ) ? ssl_write_ret_ : TARO_ERR_DISCONNECT;
}

int32_t TcpClientSSL::recv( char* buffer, size_t bytes, int32_t ms )
{
    if( nullptr == buffer || 0 == bytes )
    {
        NET_ERROR << "parameter invalid";
        return TARO_ERR_INVALID_ARG;
    }

    if ( !ssl_conn_.state() )
    {
        if ( support_connect_ )
        {
            NET_ERROR << "connect first";
            return TARO_ERR_FAILED; 
        }

        if ( !do_accept() )
        {
            NET_ERROR << "handshake failed";
            return TARO_ERR_FAILED;
        }
    }
    
    DynPacketSPtr packet;
    auto ret = TcpClientRaw::recv( ms, packet );
    if ( ret <= 0 )
    {
        NET_ERROR << "receive error:" << errno;
        return TARO_ERR_DISCONNECT;
    }
    TARO_ASSERT( packet != nullptr );
    ssl_conn_.handle_read_event( packet );
    ret = ssl_conn_.read( buffer, ( int32_t )bytes );
    return ( ret == 0 ) ? TARO_ERR_CONTINUE : ret;
}

int32_t TcpClientSSL::write_data( char* data, uint32_t bytes )
{
    ssl_write_ret_ = TcpClientRaw::send( data, bytes );
    return ( ssl_write_ret_ > 0 ) ? ssl_write_ret_ : 0;
}

NAMESPACE_TARO_NET_END
