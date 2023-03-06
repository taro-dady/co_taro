
#pragma once

#include "base/base.h"
#include "base/utils/utils.h"
#include "base/memory/details/destroyer.h"
#include <string.h>
#include <functional>

NAMESPACE_TARO_BEGIN

// 线程调度策略
enum EThrdPolicy
{
    eThrdPolicyDefault,
    eThrdPolicyRR,
};

// 优先级
enum EPriority
{
    ePriorityLow  = 0,
    ePriorityMid  = 1,
    ePriorityHigh = 2,
    ePriorityNum  = 3,
};

// 线程属性
struct ThrdAttr
{
    int32_t     stack_size; // 线程栈大小
    EPriority   prio;       // 线程优先级
    EThrdPolicy policy;     // 线程调度策略
    int32_t     cpus[512];  // 线程绑定的cpu序号
    int32_t     cpu_num;    // 绑定cpu数量
};

/**
 * @brief 初始化默认参数
*/
inline void init_attr( ThrdAttr& attr )
{
    attr.stack_size = 1024 * 128;
    attr.prio       = ePriorityMid;
    attr.policy     = eThrdPolicyDefault;
    attr.cpu_num    = 0;
    memset( attr.cpus, 0, sizeof( attr.cpus ) );
}

// 线程内部变量
struct ThreadImpl;

// 线程对象
class TARO_DLL_EXPORT Thread 
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] name
    */
    Thread( const char* name = "default" );

    /**
    * @brief 构造函数
    * 
    * @param[in] other
    */
    Thread( Thread&& other );

    /**
    * @brief 析构函数
    */
    ~Thread();

    /**
    * @brief 获取线程名称
    */
    std::string name() const;

    /**
    * @brief 启动线程
    * 
    * @param[in] func 线程处理函数
    * @param[in] attr 线程属性
    */
    int32_t start( std::function<void()> const& func, ThrdAttr const& attr );

    /**
    * @brief 等待线程结束
    */
    void join();

    /**
    * @brief 获取当前的线程号
    */
    static int64_t current_tid();

    /**
    * @brief 线程等待指定时间
    * 
    * @param[in] millisec 毫秒
    */
    static void sleep( int64_t millisec );

PRIVATE: // 私有函数

    TARO_NO_COPY( Thread );

PRIVATE: // 私有变量

    ThreadImpl* impl_;
};

NAMESPACE_TARO_END
