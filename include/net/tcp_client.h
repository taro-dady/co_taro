
#pragma once

#include "net/defs.h"
#include <memory>

NAMESPACE_TARO_NET_BEGIN

// tcp客户端
TARO_INTERFACE TcpClient
{
PUBLIC:

    /**
    * @brief 虚析构
    */
    virtual ~TcpClient() = default;

    /**
    * @brief 连接服务器
    * 
    * @param[in] ip   服务器ip
    * @param[in] port 服务端口
    */
    virtual bool connect( std::string const& ip, uint16_t port ) = 0;

    /**
    * @brief 发送数据
    * 
    * @param[in] buffer 数据缓冲
    * @param[in] bytes  数据字节
    */
    virtual int32_t send( char* buffer, size_t bytes ) = 0;

    /**
    * @brief 接收数据
    * 
    * @param[in] buffer 数据缓冲
    * @param[in] bytes  缓冲大小
    * @param[in] ms     等待超时时间(毫秒) <= 0 表示一直阻塞等待
    */
    virtual int32_t recv( char* buffer, size_t bytes, int32_t ms = TARO_FOREVER ) = 0;

    /**
    * @brief 获取远端的地址
    */
    virtual IpAddr get_peer_info() const = 0;
};

using TcpClientSPtr = std::shared_ptr<TcpClient>;

/**
* @brief 创建tcp客户端
*
* @param[in] ctx  加密传输的参数 nullptr 表示不使用加密传输
* @return tcp客户端
*/
TARO_DLL_EXPORT TcpClientSPtr create_tcp_cli( SSLContext* ctx = nullptr );

NAMESPACE_TARO_NET_END
