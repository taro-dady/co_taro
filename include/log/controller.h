
#pragma once

#include "log/sink.h"

NAMESPACE_TARO_LOG_BEGIN

class LogSystem;

// 日志模块控制器
class TARO_DLL_EXPORT Controller
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] name 名称
    */
    Controller( const char* name = "" );

    /**
    * @brief 析构函数
    */
    ~Controller();

    /**
    * @brief 获取对象名称
    */
    const char* name() const;

    /**
    * @brief 获取使能开关
    */
    bool enabled();

    /**
    * @brief 设置使能开关
    * 
    * @param[in] enable 使能开关
    */
    void set_enable( bool enable );

    /**
    * @brief 获取日志级别
    */
    ELogLevel level();

    /**
    * @brief 设置日志级别
    * 
    * @param[in] level 日志级别
    */
    void set_level( ELogLevel level );

    /**
    * @brief 添加输出模块
    * 
    * @param[in] sink 输出模块对象
    */
    bool add_sink( SinkSPtr const& sink );

    /**
    * @brief 删除输出模块
    * 
    * @param[in]  name 输出模块名称
    */
    bool remove_sink( const char* name );

PRIVATE: // 私有类型

    friend class LogSystem;

PRIVATE: // 私有函数

    TARO_NO_COPY( Controller );

    /**
    * @brief 输出日志
    * 
    * @param[in] param 参数
    */
    void write( const char* message, ELogLevel const& lvl );

PRIVATE: // 私有变量
    
    struct Impl;
    Impl* impl_;
};

using ControllerSPtr = std::shared_ptr<Controller>;

NAMESPACE_TARO_LOG_END
