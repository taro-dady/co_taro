
#pragma once

#include "co_routine/defs.h"
#include "base/system/thread.h"
#include <functional>

NAMESPACE_TARO_RT_BEGIN

// 协程状态
enum ECoRoutineState
{
    eCoRoutineStateReady,
    eCoRoutineStateRunning,
    eCoRoutineStateSuspend,
    eCoRoutineStateFinish,
};

// 协程属性
struct CoRoutineAttr
{
    CoRoutineAttr()
        : max_co_routine( 1000 )
        , stack_min_size( 1024 * 1024 )
        , stack_max_size( 2 * 1024 * 1024 )
    {}

    int32_t  max_co_routine; // 协程的最大数量
    uint32_t stack_min_size; // 栈的最小字节
    uint32_t stack_max_size; // 栈的最大字节
};

class CoRoutine;
struct CoRoutineImpl;
using CoRoutineSPtr = std::shared_ptr< CoRoutine >;

// 协程对象
class TARO_DLL_EXPORT CoRoutine
{
PUBLIC: // 公共函数

    /**
    * @brief 创建协程
    * 
    * @param[in] name       协程名称
    * @param[in] func       处理函数
    * @param[in] sched_auto 创建即被当前线程的调度, 创建和调度分开的时候需要将该值设置为false
    * @param[in] prio       优先级
    * @param[in] stack_size 栈大小(取值范围参考CoRoutineAttr)
    * 
    * @return 协程对象指针
    */
    static CoRoutineSPtr create( const char* name,
                                 std::function<void()> const& func, 
                                 bool auto_sched = true,
                                 EPriority prio = ePriorityMid,
                                 uint32_t stack_size = 0 );

    /**
    * @brief 获取当前协程对象
    */
    static CoRoutineSPtr current();

    /**
    * @brief 初始化协程库
    * 
    * @return 协程属性
    */
    static void init( CoRoutineAttr const& attr = CoRoutineAttr() );

    /**
    * @brief 协程退出时的清理程序
    * 
    * @param[in] func 清理程序
    */
    void on_exit( std::function<void()> const& func );

    /**
    * @brief 获取协程名称
    */
    const char* name() const;

    /**
    * @brief 获取协程号
    */
    int32_t cid() const;

    /**
    * @brief 获取协程状态
    */
    ECoRoutineState state() const;

    /**
    * @brief 获取协程优先级
    */
    EPriority priority() const;

    /**
    * @brief 让渡协程
    */
    void yield();

PRIVATE: // 私有类型

    friend struct CoRoutineImpl;

PRIVATE: // 私有函数

    TARO_NO_COPY( CoRoutine );

    /**
    * @brief 构造函数
    */
    CoRoutine();

    /**
    * @brief 析构函数
    */
    ~CoRoutine();

    /**
    * @brief 恢复协程
    */
    void resume();

PRIVATE: // 私有变量

    CoRoutineImpl* impl_;
};

NAMESPACE_TARO_RT_END
