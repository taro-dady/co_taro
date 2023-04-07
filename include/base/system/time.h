
#pragma once

#include "base/system/file_sys.h"

NAMESPACE_TARO_BEGIN

// 单调时钟
class TARO_DLL_EXPORT SteadyTime
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    SteadyTime();

    /**
    * @brief 析构函数
    */
    ~SteadyTime();

    /**
    * @brief 获取时间差
    * 
    * @return 与构造时的时间差(ms)
    */
    int64_t cost();

    /**
    * @brief 获取系统启动开始经过的ms
    */
    static int64_t current_ms();

PRIVATE: // 私有函数

    TARO_NO_COPY( SteadyTime );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

// 系统时钟
class TARO_DLL_EXPORT SystemTime
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    SystemTime();

    /**
    * @brief 构造函数
    * 
    * @param[in] 时间戳
    */
    SystemTime( int64_t const& t );

    /**
    * @brief 析构函数
    */
    ~SystemTime();

    /**
    * @brief 获取当前时间与系统启动时刻的时间差（毫秒）
    */
    static int64_t current_ms();

    /**
    * @brief 获取当前时间与系统启动时刻的时间差（秒）
    */
    static int64_t current_sec();

    /**
    * @brief 将时间转换为字符串(秒)
    * 
    * @param[in] fmt 时间格式 
    */
    std::string to_sec_str( const char* fmt = "%04d-%02d-%02d %02d:%02d:%02d" );

    /**
    * @brief 将时间转换为字符串(毫秒) 如: 2022-01-01 02:12:54 023
    * 
    * @param[in] fmt 时间格式 
    */
    std::string to_ms_str( const char* fmt = "%04d-%02d-%02d %02d:%02d:%02d %03lld" );

PRIVATE: // 私有函数

    TARO_NO_COPY( SystemTime );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

NAMESPACE_TARO_END
