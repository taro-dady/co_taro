
#pragma once

#include "co_routine/co_routine.h"

NAMESPACE_TARO_RT_BEGIN

// 定时器
class TARO_DLL_EXPORT CoTimer
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    CoTimer();

    /**
    * @brief 析构函数
    */
    ~CoTimer();
    
    /**
    * @brief 启动定时器
    * 
    * @param[in] delay    延时时间
    * @param[in] period   定时器周期
    * @param[in] callback 定时器回调
    */
    bool start( int32_t delay, int32_t period, std::function<void()> const& callback );

    /**
    * @brief 只调用一次
    * 
    * @param[in] delay    延时时间
    * @param[in] callback 定时器回调
    */
    bool once( int32_t delay, std::function<void()> const& callback );

    /*
    * @brief 停止定时器
    */
    void stop();

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

NAMESPACE_TARO_RT_END
