
#pragma once

#include "task_flow/task_node.h"

NAMESPACE_TARO_TASKFLOW_BEGIN

// 任务调度器
class TARO_DLL_EXPORT TaskScheduler
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    TaskScheduler();

    /**
    * @brief 析构函数
    */
    ~TaskScheduler();   

    /**
    * @brief 启动异步任务
    * 
    * @param[in] thrd_num 线程数量
    */
    int32_t start( int32_t thrd_num = 4 );

    /**
    * @brief 停止异步任务
    */
    int32_t stop();

    /**
    * @brief 驱动异步任务
    * 
    * @param[in] msg 入口消息
    */
    int32_t feed( TaskMessageSPtr const& msg );
    
    /**
    * @brief 添加节点
    * 
    * @param[in] node 节点
    */
    int32_t add_node( TaskNodeSPtr const& node );

    /**
    * @brief 删除节点
    * 
    * @param[in] name 节点名称
    */
    int32_t remove_node( const char* name );

    /**
    * @brief 查询节点
    * 
    * @param[in]  name 节点名称
    * @param[out] node 查询到的节点
    */
    int32_t find_node( const char* name, TaskNodeSPtr& node );

    /**
    * @brief 连接节点
    * 
    * @param[in] 源节点
    * @param[in] 目的节点
    */
    int32_t add_edge( const char* src, const char* dst );

    /**
    * @brief 断开节点
    * 
    * @param[in] 源节点
    * @param[in] 目的节点
    */
    int32_t remove_edge( const char* src, const char* dst );

    /**
    * @brief 判断是否成环
    * 
    * @param[out] node 成环的节点
    */
    bool has_circle( std::string& node );

PRIVATE: // 私有函数

    TARO_NO_COPY( TaskScheduler );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

NAMESPACE_TARO_TASKFLOW_END
