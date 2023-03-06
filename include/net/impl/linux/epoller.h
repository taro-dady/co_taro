
#pragma once

#include "multiplexer.h"
#include <vector>
#include <sys/epoll.h> 

NAMESPACE_TARO_NET_BEGIN

// epoll实现的多路复用器
class Epoller : PUBLIC Multiplexer
{
PUBLIC:

    /*
    * @brief 构造函数
    */
    Epoller();

    /*
    * @brief 析构函数
    */
    ~Epoller();
    
    /**
    * @brief 添加事件
    *
    * @param[in] fd      文件描述符
    * @param[in] handler 事件处理器
    * @param[in] evt     事件类型
    */
    virtual bool add_event( int32_t fd, NetEventHandler* handler, int32_t evt ) override final;

    /**
    * @brief 修改事件类型
    *
    * @param[in] fd      文件描述符
    * @param[in] handler 事件处理器
    * @param[in] evt     事件类型
    */
    virtual bool modify_event( int32_t fd, NetEventHandler* handler, int32_t evt ) override final;

    /**
    * @brief 删除事件
    *
    * @param[in] fd      文件描述符
    */
    virtual bool remove_event( int32_t fd ) override final;

    /**
    * @brief 轮询事件
    *
    * @param[in] ms      超时时间
    */
    virtual void poll( int32_t ms ) override final;

    /**
    * @brief 唤醒
    */
    virtual void wakeup() override final;

PRIVATE: // type

    using EventArray = std::vector<struct epoll_event>;

PRIVATE: // function

    /*
    * @brief 初始化事件唤醒描述符
    */
    void init_event_fd();

PRIVATE:

    int32_t    num_;
    int32_t    epoll_fd_;
    int32_t    wakeup_fd_;
    EventArray events_;
};

NAMESPACE_TARO_NET_END
