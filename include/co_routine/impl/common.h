
#pragma once

#include "co_routine/impl/co_task.h"
#include "co_routine/impl/co_scheduler.h"
#include <list>

NAMESPACE_TARO_RT_BEGIN

/**
* @brief 获取当前调度策略
*/
CoSchedulerSPtr get_scheduler();

/**
* @brief 获取线程号
*/
extern int64_t get_tid();

/**
* @brief 调度协程
*/
extern void sched_co( CoRoutineSPtr co );

/**
* @brief 创建延迟调度对象
*/
extern std::function<void()> delay_sched_func( CoRoutineSPtr& co );

/**
* @brief 进程中是否还有协程
*/
extern bool has_co_routine();

/**
* @brief 线程容器
*/
struct CoTaskContainer
{   
PUBLIC:

    /**
    * @brief 获取单例
    */
    static CoTaskContainer& instance()
    {
        static CoTaskContainer inst;
        return inst;
    }

    /**
    * @brief 添加任务
    */
    void push( CoTaskSPtr const& task )
    {
        tasks_.push_back( task );
    }

    /**
    * @brief 等待所有任务结束
    */
    void join()
    {
        for( auto& one : tasks_ )
        {
            one->join();
        }
    }
    
PRIVATE:

    std::list<taro::rt::CoTaskSPtr> tasks_;
};

NAMESPACE_TARO_RT_END
