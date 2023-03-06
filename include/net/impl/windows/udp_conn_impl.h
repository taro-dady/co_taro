
#pragma once

#include "net/udp_conn.h"
#include "net/impl/windows/io_event.h"
#include <winsock2.h>
#include <tchar.h>
#include <windows.h>
#include <ws2ipdef.h>

NAMESPACE_TARO_NET_BEGIN

// udp内部实现
class UdpConnImpl : PUBLIC UdpConn
{
PUBLIC:

    /**
    * @brief 构造函数
    */
    UdpConnImpl();

    /**
    * @brief 析构函数
    */
    ~UdpConnImpl();

PRIVATE:

    /**
    * @brief 绑定地址
    * 
    * @param[in] addr  绑定地址
    * @param[in] reuse 端口是否重用
    */
    virtual bool bind( IpAddr const& addr, bool reuse = false ) override;

    /**
    * @brief 发送数据
    * 
    * @param[in] remote 发送地址
    * @param[in] buffer 数据缓冲
    * @param[in] bytes  数据字节
    */
    virtual int32_t sendto( IpAddr const& remote, char* buffer, size_t bytes ) override;

    /**
    * @brief 接收数据
    * 
    * @param[in]  buffer 数据缓冲
    * @param[in]  cap    缓冲大小
    * @param[out] remote 发送端地址
    * @param[in]  ms     等待超时时间(毫秒) <= 0 表示一直阻塞等待
    */
    virtual int32_t recvfrom( char* buffer, size_t cap, IpAddr& remote, int32_t ms ) override;

    /**
    * @brief 设置组播环回使能
    * 
    * @param[in] loop  true 开启 false 关闭
    */
    virtual bool set_group_loop( bool loop = true ) override;

    /**
    * @brief 加入组播
    * 
    * @param[in] local_port  组播端口
    * @param[in] group       组播地址
    */
    virtual bool join_group( uint16_t local_port, std::string const& group ) override;

    /**
    * @brief 退出组播
    */
    virtual bool leave_group() override;

    /**
    * @brief 拷贝socket
    */
    virtual std::shared_ptr<UdpConn> copy() override;

    /**
    * @brief 投递读事件
    */
    bool post_read_evt( IocpUdpReadEevent* data );

    /**
    * @brief 投递写事件
    */
    bool post_write_evt( IocpUdpWriteEvent* data );

    /**
    * @brief 初始化IOCP
    */
    void init_iocp();

PRIVATE:

    SOCKET          socket_;
    struct ip_mreq* group_setting_;
    void*           co_evt_;
};

NAMESPACE_TARO_NET_END
