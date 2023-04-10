
#pragma once

#include "defs.h"
#include "base/system/time.h"
#include <memory>

NAMESPACE_TARO_TASKFLOW_BEGIN

struct TaskMessageImpl;

// 任务消息
class TARO_DLL_EXPORT TaskMessage
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] src      数据源
    * @param[in] type     消息类型
    * @param[in] sequence 序列号
    * @param[in] ts       时间戳
    */
    TaskMessage( const char* src, const char* type, uint32_t sequence, int64_t ts = SystemTime::current_ms() );

    /**
    * @brief 析构函数
    */
    virtual ~TaskMessage();

    /**
    * @brief 获取源节点信息
    */
    const char* source() const;

    /**
    * @brief 设置源节点信息
    */
    void set_source( const char* src );
    
    /**
    * @brief 获取消息类型
    */
    const char* type() const;

    /**
    * @brief 设置消息类型
    */
    void set_type( const char* type );
    
    /**
    * @brief 获取序列号
    */
    uint32_t sequence() const;

    /**
    * @brief 设置消息序列号
    */
    void set_sequence( uint32_t sequence );
    
    /**
    * @brief 获取时间戳
    */
    int64_t time_stamp() const;
    
    /**
    * @brief 设置时间戳
    */
    void set_time_stamp( int64_t ts );

PRIVATE: // 私有类型 

    friend struct TaskMessageImpl;

PRIVATE: // 私有函数

    TARO_NO_COPY( TaskMessage );

PRIVATE: // 私有变量

    TaskMessageImpl* impl_;
};

using TaskMessageSPtr = std::shared_ptr<TaskMessage>;

NAMESPACE_TARO_TASKFLOW_END
