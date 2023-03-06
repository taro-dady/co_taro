
#include "base/utils/assert.h"
#include "net/impl/open_ssl/tcp_server_ssl.h"

NAMESPACE_TARO_NET_BEGIN

TcpServerSSL::TcpServerSSL( SSLContext const& ctx )
    : ctx_( SSLCtxFactory::instance().find_or_create( ctx ) )
{
    TARO_ASSERT( ctx_ != nullptr, "ssl file invalid" );
}

TcpClientSPtr TcpServerSSL::accept()
{
    socket_t socket;
    if ( !TcpServerRaw::accept_socket( socket ) )
    {
        set_errno( TARO_ERR_FAILED );
        NET_ERROR << "accept failed";
        return nullptr;
    }
    return std::make_shared< TcpClientSSL >( socket, ctx_ );
}

NAMESPACE_TARO_NET_END
