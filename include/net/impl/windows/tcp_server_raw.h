

#pragma once

#include "net/tcp_server.h"
#include "net/impl/windows/tcp_client_raw.h"

NAMESPACE_TARO_NET_BEGIN

// tcp服务端
class TcpServerRaw : PUBLIC TcpServer
{
PUBLIC:

    /**
    * @brief 构造函数
    */
    TcpServerRaw();

    /**
    * @brief 析构函数
    */
    ~TcpServerRaw();
    
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
    bool accept_socket( SOCKET& socket );

    /*
    * @brief 初始化服务
    */
    bool init( SOCKET socket );

    /*
    * @brief 初始化服务
    */
    void post_accept_evt( IocpAcceptEvent* io_data );

PROTECTED:
    
    SOCKET           socket_;
    LPFN_ACCEPTEX    fn_ctx_;
    IocpAcceptEvent* co_evt_;
};

NAMESPACE_TARO_NET_END

