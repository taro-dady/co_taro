
#pragma once

#include "log/defs.h"
#include <memory>

NAMESPACE_TARO_LOG_BEGIN

// 输出对象
class TARO_DLL_EXPORT Sink
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] name 名称
    */
    Sink( const char* name );

    /**
    * @brief 析构函数
    */
    virtual ~Sink();

    /**
    * @brief 输出日志
    * 
    * @param[in] log_msg 日志数据
    * @param[in] level   日志等级
    */
    virtual void write( const char* log_msg, ELogLevel const& level ) = 0;

    /**
    * @brief 获取输出类型
    */
    virtual const char* type() const = 0;

    /**
    * @brief 获取对象名称
    */
    const char* name() const;

    /**
    * @brief 获取使能开关
    */
    bool enabled() const;

    /**
    * @brief 设置使能开关
    * 
    * @param[in] enable 使能标识
    */
    void set_enable( bool enable );

    /**
    * @brief 是否异步输出
    */
    bool use_async_mode() const;

    /**
    * @brief 设置异步输出开关
    * 
    * @param[in] enable 使能标识
    */
    void set_async_mode( bool enable );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

using SinkSPtr = std::shared_ptr< Sink >;

NAMESPACE_TARO_LOG_END
