
#pragma once

#include "log/controller.h"

NAMESPACE_TARO_LOG_BEGIN

struct LogSystemImpl;

// 日志系统
class TARO_DLL_EXPORT LogSystem
{
PUBLIC: // 公共函数

    /**
    * @brief 获取日志系统
    */
    static LogSystem& instance();

    /**
    * @brief 添加控制器
    * 
    * @param[in] ctrl 控制器
    */
    int32_t add_ctrl( ControllerSPtr ctrl );

    /**
    * @brief 删除控制器
    * 
    * @param[in]  name 控制器名称
    * @param[out] ctrl 控制器对象
    */
    int32_t remove_ctrl( const char* name );

    /**
    * @brief 查找控制器
    * 
    * @param[in] name 控制器名称
    */
    ControllerSPtr find_ctrl( const char* name );

PRIVATE: // 私有类型

    friend class LogWriter;
    friend struct LogSystemImpl;

PRIVATE: // 私有函数

    TARO_NO_COPY( LogSystem );

    /**
    * @brief 构造函数
    */
    LogSystem();

    /**
    * @brief 析构函数
    */
    ~LogSystem();

    /**
    * @brief 输出日志
    * 
    * @param[in] module_name 模块名称
    * @param[in] level       日志级别
    * @param[in] message     日志消息
    */
    void write( const char* module_name, ELogLevel level, const char* message );

PRIVATE: // 私有变量

    LogSystemImpl* impl_;
};

NAMESPACE_TARO_LOG_END
