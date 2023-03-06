
#pragma once

#include "base/system/time.h"
#include "base/system/spin_locker.h"
#include "base/memory/bounded_queue.h"
#include "co_routine/impl/co_scheduler.h"
#include <map>
#include <list>
#include <array>
#include <atomic>

NAMESPACE_TARO_RT_BEGIN

// 每个线程都有一个协程调度器
// 该调度器使用本地协程队列，协程不可以跨线程调度
class DefaultScheduler : PUBLIC CoScheduler
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    DefaultScheduler();

PRIVATE: // 私有类型

    struct EventItem
    {
        int32_t               evtid;
        std::function<void()> cb;
    };

    using CoQueue = std::array< BoundedQueue< CoRoutineSPtr, SpinLocker >, ePriorityNum >;
    using CoEvent = std::multimap< int64_t, EventItem >;

PRIVATE: // 私有函数

    /**
    * @brief 获取调度器类型
    */
    virtual const char* type() const override final;

    /**
    * @brief 增加要被调度的协程
    */
    virtual void push( CoRoutineSPtr const& co ) override final;

    /**
    * @brief 增加延时触发事件
    * 
    * @param[in] evt_cb 事件回调
    * @param[in] ms     延时时间(ms)
    * @return 事件编号 < 0 无效
    */
    virtual int32_t add_event( std::function<void()> const& evt_cb, int32_t ms ) override final;

    /**
    * @brief 增加延时事件
    * 
    * @param[in] eid 事件编号 
    */
    virtual bool remove_event( int32_t eid ) override final;

    /**
    * @brief 运行调度器
    */
    virtual void run() override final;

    /**
    * @brief 设置事件轮询器
    */
    virtual void set_evt_poller( EventPollerSPtr const& poller ) override final;

    /**
    * @brief 退出轮询
    */
    virtual void exit() override final;

    /**
    * @brief 运行调度器
    */
    virtual void run_until_no_task() override final;

    /**
    * @brief 获取需要等待的时间(ms)
    */
    int32_t wait_time();

    /**
    * @brief 轮询处理超时事件
    *
    * @param[in] now 当前时间
    */
    void poll_timeout_evt( int64_t now = SteadyTime::current_ms() );

PRIVATE: // 私有变量

    bool            looping_;
    int64_t         tid_;
    CoQueue         co_que_;
    CoEvent         co_evt_;
    SpinLocker      evt_mtx_;
    std::atomic_int seq_;
    std::atomic_int cnt_;
    EventPollerSPtr evt_poller_;
};

NAMESPACE_TARO_RT_END
