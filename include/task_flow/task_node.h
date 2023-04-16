
#pragma once

#include "base/memory/graph.h"
#include "task_flow/task_message.h"

NAMESPACE_TARO_TASKFLOW_BEGIN

struct TaskNodeImpl;

// 任务节点
class TARO_DLL_EXPORT TaskNode : PUBLIC GraphyNode
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] name 节点名称
    */
    TaskNode( const char* name );

    /**
    * @brief 析构函数
    */
    virtual ~TaskNode();

    /**
    * @brief 投递数据
    * 
    * @param[in] msg 投递的消息，由数据源调用
    */
    void feed( TaskMessageSPtr const& msg );

PROTECTED: // 保护函数

    /**
    * @brief 处理消息
    * 
    * @param[in] msg 输入消息
    * @return 输出消息
    */
    virtual TaskMessageSPtr on_message( TaskMessageSPtr const& msg );

    /**
    * @brief 初始化
    */
    virtual int32_t init();

    /**
    * @brief 节点退出
    */
    virtual int32_t finish();

PRIVATE: // 私有类型

    friend class TaskScheduler;

PRIVATE: // 私有函数

    TARO_NO_COPY( TaskNode );

PRIVATE: // 私有变量

    TaskNodeImpl* impl_;
};

using TaskNodeSPtr = std::shared_ptr<TaskNode>;

NAMESPACE_TARO_TASKFLOW_END
