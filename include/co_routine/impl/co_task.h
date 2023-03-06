
#pragma once

#include "base/system/thread.h"
#include "co_routine/co_routine.h"

NAMESPACE_TARO_RT_BEGIN

// 协程对应的线程封装
class CoTask
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    CoTask( const char* name = "default" );

    /**
    * @brief 析构函数
    */
    ~CoTask();

    /**
    * @brief 添加该线程运行的协程 CoRoutine::create sched_auto = false
    * 
    * @param[in] co 协程
    */
    bool append( CoRoutineSPtr const& co );

    /**
    * @brief 启动线程
    * 
    * @param[in] attr 线程属性
    */
    bool start( ThrdAttr const& attr = ThrdAttr() );

    /**
    * @brief 等待线程结束
    */
    void join();

    /**
    * @brief 获取线程号
    */
    int64_t tid();

    /**
    * @brief 获取名称
    */
    std::string name() const;

PRIVATE: // 私有变量

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

using CoTaskSPtr = std::shared_ptr<CoTask>;

NAMESPACE_TARO_RT_END
