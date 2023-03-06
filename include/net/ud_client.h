
#pragma once

#include "net/defs.h"
#include <memory>

NAMESPACE_TARO_NET_BEGIN

struct UDClientImpl;

// unix domain 客户端
class UDClient
{
PUBLIC:

    /**
    * @brief 构造函数
    */
    UDClient();

    /**
    * @brief 析构
    */
    ~UDClient();

    /**
    * @brief 连接服务
    * 
    * @param[in] remote 服务名称
    * @param[in] self   自己的名称(get_peer_info返回该值)
    */
    bool connect( std::string const& remote, std::string const& self );

    /**
    * @brief 发送数据
    * 
    * @param[in] buffer 数据缓冲
    * @param[in] bytes  数据字节
    */
    int32_t send( char* buffer, size_t bytes );

    /**
    * @brief 接收数据
    * 
    * @param[in] buffer 数据缓冲
    * @param[in] bytes  缓冲大小
    * @param[in] ms     等待超时时间(毫秒) <= 0 表示一直阻塞等待
    */
    int32_t recv( char* buffer, size_t bytes, int32_t ms = TARO_FOREVER );

    /**
    * @brief 获取远端的地址
    */
    std::string get_peer_info() const;

PRIVATE:

    friend struct UDClientImpl;
    std::unique_ptr<UDClientImpl> impl_;
};

using UDClientSPtr = std::shared_ptr<UDClient>;

NAMESPACE_TARO_NET_END
