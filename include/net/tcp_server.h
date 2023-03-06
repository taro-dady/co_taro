
#pragma once

#include "net/tcp_client.h"

NAMESPACE_TARO_NET_BEGIN

// tcp服务端
TARO_INTERFACE TcpServer
{
PUBLIC:

    /**
    * @brief 析构函数
    */
    virtual ~TcpServer() = default;
    
    /**
    * @brief 监听客户端
    * 
    * @param[in] ip    监听ip
    * @param[in] port  监听端口
    * @param[in] reuse 端口是否重用
    */
    virtual bool listen( const char* ip, uint16_t port, bool reuse = true ) = 0;
    
    /*
    * @brief 接受对端连接
    */
    virtual TcpClientSPtr accept() = 0;
};

using TcpServerSPtr = std::shared_ptr<TcpServer>;

/**
* @brief 创建tcp服务端
*
* @param[in] ctx  加密传输的参数 nullptr 表示不使用加密传输
* @return tcp服务端
*/
TARO_DLL_EXPORT TcpServerSPtr create_tcp_svr( SSLContext* ctx = nullptr );

NAMESPACE_TARO_NET_END
