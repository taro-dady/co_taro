
#pragma once

#include "base/system/sync_variable.h"
#include "co_routine/impl/co_scheduler.h"
#include <mutex>

NAMESPACE_TARO_RT_BEGIN

// 条件变量时间轮询对象，用于时间处理
class CondEvtPoller : PUBLIC EventPoller
{
PUBLIC: // 公共函数

    /**
    * @brief 等待 
    * 
    * @param[in] ms 等待时间 
    */
    virtual void wait( int32_t ms ) override final
    {
        mutex_.lock();
        if ( ms >= 0 )
            ( void )cond_.wait( mutex_, ms );
        else
            cond_.wait( mutex_ );
        mutex_.unlock();
    }

    /**
    * @brief 唤醒 
    */
    virtual void wakeup() override final
    {
        cond_.notify_one();
    }

PRIVATE: // 私有变量

    ConditionVariable cond_;
    Mutex             mutex_;
};

NAMESPACE_TARO_RT_END
