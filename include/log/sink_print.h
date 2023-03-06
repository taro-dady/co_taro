
#pragma once

#include "log/sink.h"

NAMESPACE_TARO_LOG_BEGIN

// 打印输出
class TARO_DLL_EXPORT PrintSink : PUBLIC Sink
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] name 名称
    */
    PrintSink( const char* name );

PRIVATE: // 私有函数

    /**
    * @brief 获取输出类型
    */
    virtual const char* type() const override;

    /**
    * @brief 输出日志
    * 
    * @param[in] log_msg 日志数据
    * @param[in] level   日志等级
    */
    virtual void write( const char* log_msg, ELogLevel const& level ) override;
};

NAMESPACE_TARO_LOG_END
