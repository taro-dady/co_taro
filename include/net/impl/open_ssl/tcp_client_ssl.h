
#pragma once

#include "net/tcp_client.h"
#include "net/impl/open_ssl/ssl_conn.h"

#if defined(_WIN32) || defined(_WIN64)
    #include "net/impl/windows/tcp_client_raw.h"
    typedef SOCKET socket_t;
#else
    #include "net/impl/linux/tcp_client_raw.h"
    typedef int32_t socket_t;
#endif

NAMESPACE_TARO_NET_BEGIN

// 原始tcp客户端
class TcpClientSSL : PUBLIC TcpClientRaw
{
PUBLIC:

    /**
    * @brief 构造函数
    */
    TcpClientSSL( SSLContext const& ctx );

    /**
    * @brief 构造函数
    */
    TcpClientSSL( socket_t socket, SSL_CTX* ctx );

    /**
    * @brief 析构函数
    */
    ~TcpClientSSL();

    /**
    * @brief 处理握手( 服务端调用 )
    */
    bool do_accept();

    /**
    * @brief 连接服务器
    * 
    * @param[in] ip   服务器ip
    * @param[in] port 服务端口
    */
    virtual bool connect( std::string const& ip, uint16_t port ) override final;

    /**
    * @brief 发送数据
    * 
    * @param[in] buffer 数据缓冲
    * @param[in] bytes  数据字节
    */
    virtual int32_t send( char* buffer, size_t bytes ) override final;

    /**
    * @brief 接收数据
    * 
    * @param[in] buffer 数据缓冲
    * @param[in] bytes  缓冲大小
    * @param[in] ms     等待超时时间(毫秒) <= 0 表示一直阻塞等待
    */
    virtual int32_t recv( char* buffer, size_t bytes, int32_t ms ) override final;

PRIVATE:

    /**
    * @brief 加密完成后输出数据
    * 
    * @param[in] data  数据指针
    * @param[in] bytes 数据长度
    */
    int32_t write_data( char* data, uint32_t bytes );

PRIVATE:
    
    int32_t ssl_write_ret_;
    SSLConn ssl_conn_;
};

NAMESPACE_TARO_NET_END
