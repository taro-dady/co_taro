

#pragma once

#include "net/tcp_server.h"
#include "net/impl/linux/event_handler.h"

NAMESPACE_TARO_NET_BEGIN

// tcp服务端
class TcpServerRaw : PUBLIC TcpServer
{
PUBLIC:

    /**
    * @brief 构造
    */
    TcpServerRaw();
    
    /**
    * @brief 监听客户端
    * 
    * @param[in] ip    监听ip
    * @param[in] port  监听端口
    * @param[in] reuse 端口是否重用
    */
    virtual bool listen( const char* ip, uint16_t port, bool reuse ) override;
    
    /*
    * @brief 接受对端连接
    */
    virtual TcpClientSPtr accept() override;

PROTECTED:

    /*
    * @brief 接受对端连接
    */
    bool accept_socket( int32_t& socket );

PROTECTED:

    int32_t        socket_;
    CoNetEventSPtr co_evt_;
};

NAMESPACE_TARO_NET_END

