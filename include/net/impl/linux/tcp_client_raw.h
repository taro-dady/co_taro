
#pragma once

#include "net/tcp_client.h"
#include "base/memory/dyn_packet.h"
#include "net/impl/linux/event_handler.h"

NAMESPACE_TARO_NET_BEGIN

// 原始tcp客户端
class TcpClientRaw : PUBLIC TcpClient
{
PUBLIC:

    /**
    * @brief 构造函数
    */
    TcpClientRaw();

    /**
    * @brief 构造函数
    */
    TcpClientRaw( int32_t socket );

    /**
    * @brief 析构函数
    */
    ~TcpClientRaw();

    /**
    * @brief 连接服务器
    * 
    * @param[in] ip   服务器ip
    * @param[in] port 服务端口
    */
    virtual bool connect( std::string const& ip, uint16_t port ) override;

    /**
    * @brief 发送数据
    * 
    * @param[in] buffer 数据缓冲
    * @param[in] bytes  数据字节
    */
    virtual int32_t send( char* buffer, size_t bytes ) override;

    /**
    * @brief 接收数据
    * 
    * @param[in] buffer 数据缓冲
    * @param[in] bytes  缓冲大小
    * @param[in] ms     等待超时时间(毫秒) <= 0 表示一直阻塞等待
    */
    virtual int32_t recv( char* buffer, size_t bytes, int32_t ms ) override;

    /**
    * @brief 获取远端的地址
    */
    virtual IpAddr get_peer_info() const override;

PROTECTED:

    /**
    * @brief 接收数据
    * 
    * @param[in]  ms     等待超时时间(毫秒) <= 0 表示一直阻塞等待
    * @param[out] packet 接收到的数据
    */
    int32_t recv( int32_t ms, DynPacketSPtr& packet );

    /**
    * @brief 清除资源
    */
    void clear();

PROTECTED:

    int32_t        socket_;
    bool           support_connect_;
    CoNetEventSPtr co_evt_;
};

NAMESPACE_TARO_NET_END
