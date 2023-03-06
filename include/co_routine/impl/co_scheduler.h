
#pragma once

#include "co_routine/co_routine.h"

NAMESPACE_TARO_RT_BEGIN

// 事件轮询对象
class EventPoller
{
PUBLIC: // 公共函数

    /**
    * @brief 虚析构
    */
    virtual ~EventPoller() = default;

    /**
    * @brief 等待指定时间
    * 
    * @param[in] ms 等待时间(毫秒)
    */
    virtual void wait( int32_t ms ) = 0;

    /**
    * @brief 唤醒
    */
    virtual void wakeup() = 0; 
};

using EventPollerSPtr = std::shared_ptr<EventPoller>;

// 协程调度器
class CoScheduler
{
PUBLIC: // 公共函数 

    /**
    * @brief 虚析构
    */
    virtual ~CoScheduler() = default;

    /**
    * @brief 获取调度器类型
    */
    virtual const char* type() const = 0;

    /**
    * @brief 增加要被调度的协程
    */
    virtual void push( CoRoutineSPtr const& co ) = 0;

    /**
    * @brief 增加延时触发事件
    * 
    * @param[in] evt_cb 事件回调
    * @param[in] ms     延时时间(ms)
    * @return 事件编号 < 0 无效
    */
    virtual int32_t add_event( std::function<void()> const& evt_cb, int32_t ms ) = 0;

    /**
    * @brief 增加延时事件
    * 
    * @param[in] eid 事件编号 
    */
    virtual bool remove_event( int32_t eid ) = 0;

    /**
    * @brief 运行调度器
    */
    virtual void run() = 0;

    /**
    * @brief 设置事件轮询器
    */
    virtual void set_evt_poller( EventPollerSPtr const& poller ) = 0;

    /*
    * @brief 退出轮询
    */
    virtual void exit() = 0;

    /**
    * @brief 运行调度器
    */
    virtual void run_until_no_task() = 0;
};

using CoSchedulerSPtr = std::shared_ptr<CoScheduler>;

NAMESPACE_TARO_RT_END
