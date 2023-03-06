
#pragma once

#include "net/impl/linux/event_handler.h"
#include <memory>

NAMESPACE_TARO_NET_BEGIN

// 多路复用器
class Multiplexer
{
PUBLIC: // function

    /**
    * @brief 虚析构函数
    */
    virtual ~Multiplexer() = default;

    /**
    * @brief 添加事件
    *
    * @param[in] fd      文件描述符
    * @param[in] handler 事件处理器
    * @param[in] evt     事件类型
    */
    virtual bool add_event( int32_t fd, NetEventHandler* handler, int32_t evt ) = 0;

    /**
    * @brief 修改事件类型
    *
    * @param[in] fd      文件描述符
    * @param[in] handler 事件处理器
    * @param[in] evt     事件类型
    */
    virtual bool modify_event( int32_t fd, NetEventHandler* handler, int32_t evt ) = 0;

    /**
    * @brief 删除事件
    *
    * @param[in] fd      文件描述符
    */
    virtual bool remove_event( int32_t fd ) = 0;

    /**
    * @brief 轮询事件
    *
    * @param[in] ms      超时时间
    */
    virtual void poll( int32_t ms ) = 0;

    /**
    * @brief 唤醒
    */
    virtual void wakeup() = 0;
};

using MultiplexerSPtr = std::shared_ptr<Multiplexer>;

NAMESPACE_TARO_NET_END

