
#pragma once

#include "net/ud_client.h"
#include "net/impl/linux/event_handler.h"

NAMESPACE_TARO_NET_BEGIN

// unix domain内部实现
struct UDClientImpl
{
    /**
    * @brief 构造函数
    */
    UDClientImpl();

    /**
    * @brief 析构函数
    */
    ~UDClientImpl();

    /**
    * @brief 创建客户端
    */
    static UDClientSPtr create_cli( int32_t socket );

    /**
    * @brief 清除资源
    */
    void clear();

    int32_t        socket_;
    bool           support_connect_;
    CoNetEventSPtr co_evt_;
};

NAMESPACE_TARO_NET_END
