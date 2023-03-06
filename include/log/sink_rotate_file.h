
#pragma once

#include "log/sink.h"

NAMESPACE_TARO_LOG_BEGIN

// 循环写文件输出
class TARO_DLL_EXPORT RotateFileSink : PUBLIC Sink
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] name   名称
    */
    RotateFileSink( const char* name );

    /**
    * @brief 析构函数
    */
    ~RotateFileSink();

    /**
    * @brief 设置参数
    * 
    * @param[in] out_dir      输出目录
    * @param[in] max_file_num 最大文件数量
    * @param[in] file_size    单个文件的大小
    */
    int32_t set_param( const char* out_dir, uint32_t max_file_num = 3, uint32_t file_size = 1024 * 1024 * 3 );

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

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

NAMESPACE_TARO_LOG_END
