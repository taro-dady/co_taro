
#include "net/net_work.h"
#include "net/impl/open_ssl/tcp_client_ssl.h"
#include "net/impl/open_ssl/tcp_server_ssl.h"

NAMESPACE_TARO_NET_BEGIN

extern std::function<TcpClientSPtr( SSLContext* )> ssl_create_cli_func;
extern std::function<TcpServerSPtr( SSLContext* )> ssl_create_svr_func;

void start_network_ssl()
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    ssl_create_cli_func = []( SSLContext* path ){ return std::make_shared<TcpClientSSL>( *path ); };
    ssl_create_svr_func = []( SSLContext* path ){ return std::make_shared<TcpServerSSL>( *path ); };
    
    start_network();
}

NAMESPACE_TARO_NET_END
