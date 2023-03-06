
#pragma once

#include "co_routine/co_routine.h"
#include <list>
#include <mutex>

NAMESPACE_TARO_RT_BEGIN

// 协程的条件变量
class TARO_DLL_EXPORT Condition
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    Condition();

    /**
    * @brief 析构函数
    */
    ~Condition();

    /**
    * @brief 唤醒等待的协程
    */
    void notify_one();

    /**
    * @brief 唤醒所有等待的协程
    */
    void notify_all();

    /**
    * @brief 等待条件变量唤醒
    */
    template<typename Lock>
    void wait( std::unique_lock<Lock>& locker )
    {
        locker.unlock();
        cond_wait();
        locker.lock();
    }

    /**
    * @brief 等待条件变量唤醒
    */
    template<typename Lock>
    bool wait( std::unique_lock<Lock>& locker, uint32_t ms )
    {
        locker.unlock();
        if ( !cond_wait( ms ) )
        {
            return false;
        }
        locker.lock();
    }

    /**
    * @brief 等待条件变量唤醒
    */
    template<typename Lock, typename Pred>
    void wait( std::unique_lock<Lock>& locker, Pred&& pred )
    {
        while( !pred() )
        {
            locker.unlock();
            cond_wait();
            locker.lock();
        }
    }

    /**
    * @brief 协程等待
    * 
    * @param[in] ms 等待时间
    */
    bool cond_wait( uint32_t ms = 0 );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

NAMESPACE_TARO_RT_END
