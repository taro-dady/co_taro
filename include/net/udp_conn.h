
#pragma once

#include "net/defs.h"
#include <memory>

NAMESPACE_TARO_NET_BEGIN

// udp连接
TARO_INTERFACE UdpConn
{
PUBLIC:
    
    /**
    * @brief 析构函数
    */
    virtual ~UdpConn() = default;

    /**
    * @brief 绑定地址
    * 
    * @param[in] addr  绑定地址
    * @param[in] reuse 端口是否重用
    */
    virtual bool bind( IpAddr const& addr, bool reuse = false ) = 0;

    /**
    * @brief 发送数据
    * 
    * @param[in] remote 发送地址
    * @param[in] buffer 数据缓冲
    * @param[in] bytes  数据字节
    */
    virtual int32_t sendto( IpAddr const& remote, char* buffer, size_t bytes ) = 0;

    /**
    * @brief 接收数据
    * 
    * @param[in]  buffer 数据缓冲
    * @param[in]  cap    缓冲大小
    * @param[out] remote 发送端地址
    * @param[in]  ms     等待超时时间(毫秒) <= 0 表示一直阻塞等待
    */
    virtual int32_t recvfrom( char* buffer, size_t cap, IpAddr& remote, int32_t ms = TARO_FOREVER ) = 0;

    /**
    * @brief 设置组播环回使能
    * 
    * @param[in] loop  true 开启 false 关闭
    */
    virtual bool set_group_loop( bool loop = true ) = 0;

    /**
    * @brief 加入组播
    * 
    * @param[in] local_port  组播端口
    * @param[in] group       组播地址
    */
    virtual bool join_group( uint16_t local_port, std::string const& group ) = 0;

    /**
    * @brief 退出组播
    */
    virtual bool leave_group() = 0;

    /**
    * @brief 拷贝socket
    */
    virtual std::shared_ptr<UdpConn> copy() = 0;
};

using UdpConnSPtr = std::shared_ptr<UdpConn>;

/**
* @brief 创建UDP
*/
extern TARO_DLL_EXPORT UdpConnSPtr create_udp_conn();

NAMESPACE_TARO_NET_END
