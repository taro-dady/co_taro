
#pragma once

#include "log/log_sys.h"
#include "log/log_writer.h"
#include "log/sink_rotate_file.h"
#include <functional>

NAMESPACE_TARO_LOG_BEGIN

/**
* @brief 获取系统使能开关
*/
TARO_DLL_EXPORT bool get_sys_enable();

/**
* @brief 设置系统使能开关
*/
TARO_DLL_EXPORT void set_sys_enable( bool enable );

/**
* @brief 获取系统日志级别
*/
TARO_DLL_EXPORT ELogLevel get_sys_level();

/**
* @brief 设置系统日志级别
* 
* @param[in] level 日志级别
*/
TARO_DLL_EXPORT void set_sys_level( ELogLevel level );

/**
* @brief 设置日志文件策略参数
* 
* @param[in] out_dir      输出目录
* @param[in] max_file_num 最大文件数量
* @param[in] file_size    单个文件的大小
* @param[in] async        异步输出
*/
TARO_DLL_EXPORT void set_sys_rotate_cfg( const char* out_dir, uint32_t max_file_num = 3, uint32_t file_size = 1024 * 1024 * 3, bool async = true );

/**
* @brief 初始化异步日志
*/
TARO_DLL_EXPORT void init_async_logger( std::function<void( SinkSPtr, std::shared_ptr<std::string>, ELogLevel)> const& func );

NAMESPACE_TARO_LOG_END
