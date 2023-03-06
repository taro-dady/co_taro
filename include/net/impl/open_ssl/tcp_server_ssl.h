
#pragma once

#include "net/impl/open_ssl/tcp_client_ssl.h"

#if defined(_WIN32) || defined(_WIN64)
    #include "net/impl/windows/tcp_server_raw.h"
#else
    #include "net/impl/linux/tcp_server_raw.h"
#endif

NAMESPACE_TARO_NET_BEGIN

class TcpServerSSL : PUBLIC TcpServerRaw
{
PUBLIC:

    /**
    * @brief 构造函数
    * 
    * @param[in] ctx SSL上下文参数
    */
    TcpServerSSL( SSLContext const& ctx );

    /*
    * @brief 接受对端连接
    */
    virtual TcpClientSPtr accept() override final;

PRIVATE:

    SSL_CTX* ctx_;
};

NAMESPACE_TARO_NET_END
