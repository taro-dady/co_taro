
#pragma once

#include "co_routine/co_routine.h"
#include "co_routine/impl/co_context.h"
#include <list>
#include <stdlib.h>

NAMESPACE_TARO_RT_BEGIN

// 协程的内部对象
struct CoRoutineImpl
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    CoRoutineImpl();

    /**
    * @brief 恢复协程
    */
    static void resume( CoRoutineSPtr dst );
    
    /**
    * @brief 销毁协程
    */
    static void destroy( CoRoutine* co );

    /**
    * @brief 释放当前协程
    */
    static void reset_curr_co();

PUBLIC: // 公共类型

    using Caller = std::function<void()>;

PUBLIC: // 公共变量

    int32_t           cid_;
    CoStack           st_;
    CoContext*        ctx_;
    EPriority         prio_;
    std::string       name_;
    ECoRoutineState   state_;
    Caller            func_;
    std::list<Caller> exit_func_;
};

NAMESPACE_TARO_RT_END
