
#pragma once

#include "net/tcp_client.h"
#include "base/memory/dyn_packet.h"
#include "net/impl/windows/io_event.h"
#include <mswsock.h>

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
    TcpClientRaw( SOCKET socket );

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

    /**
    * @brief 触发链接事件
    */
    bool post_connect_evt( SOCKET socket, const char* ip, uint16_t port, IocpEvent* evt );

    /**
    * @brief 触发读事件
    */
    bool post_read_evt( IocpTypeEvent* data );

    /**
    * @brief 触发写事件
    */
    bool post_write_evt( IocpTypeEvent* data );

PROTECTED:

    SOCKET         socket_;
    bool           support_connect_;
    IocpTypeEvent* co_evt_;
    LPFN_CONNECTEX fn_ctx_;
};

NAMESPACE_TARO_NET_END
