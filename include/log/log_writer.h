
#pragma once

#include "log/defs.h"
#include <iostream>

NAMESPACE_TARO_LOG_BEGIN

// 日志
class TARO_DLL_EXPORT LogWriter
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] module    模块
    * @param[in] level     日志级别
    * @param[in] file      文件名称
    * @param[in] function  函数名称
    * @param[in] line      文件行号
    */
    LogWriter( const char* module, ELogLevel const& level, const char* file, const char* function, int32_t line );

    /**
    * @brief 析构函数
    */
    ~LogWriter();

    /**
    * @brief 获取输出字符流对象
    */
    template<typename OutStream>
    OutStream& stream()
    {
        return *( ( OutStream* )get_stream() );
    }

PRIVATE: // 私有函数

    TARO_NO_COPY( LogWriter );
    
    /**
    * @brief 获取输出字符流对象
    */
    void* get_stream();

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

NAMESPACE_TARO_LOG_END
