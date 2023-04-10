
#pragma once

#include "base/memory/graph.h"
#include "co_routine/impl/co_task.h"
#include "task_flow/task_scheduler.h"
#include <atomic>

NAMESPACE_TARO_TASKFLOW_BEGIN

struct TaskMessageImpl
{
    int64_t     time_stamp_;
    uint32_t    sequence_;
    std::string type_;
    std::string src_;
};

struct TaskNodeImpl
{
    std::function< void( TaskNode*, TaskMessageSPtr const& ) > feed_func_;
};

struct SchedTask
{
    /**
    * @brief 构造函数
    */
    SchedTask() 
        : payloads_( 0 )
        , task_( new rt::CoTask )
    {}

    std::atomic_int32_t payloads_;
    std::unique_ptr<rt::CoTask> task_;
};
using SchedTaskPtr = std::unique_ptr<SchedTask>;

struct TaskScheduler::Impl
{
    /**
    * @brief 构造函数
    */
    Impl() : running_( false ) {}

    /**
    * @brief 启动源对象
    */
    void start_source( TaskNodeSPtr const& node )
    {   
        push_co( rt::CoRoutine::create( node->name(), [node]()
        {
            node->start_source();
        }, false ) );
    }

    /**
    * @brief 运行下一步的节点
    * 
    * @param[in] node 当前节点
    * @param[in] msg  当前节点输出的消息
    */
    void next( TaskNode* node, TaskMessageSPtr const& msg )
    {
        for( auto& one : node->next() )
        {
            auto handler = [this, one, msg]()
            {
                auto task_node = std::dynamic_pointer_cast<TaskNode>( one );
                auto out_msg = task_node->on_message( msg );
                if ( out_msg != nullptr )
                {
                    this->next( task_node.get(), out_msg );
                }
            };
            push_co( rt::CoRoutine::create( one->name(), handler, false ) );
        }
    }

    /**
    * @brief 投递协程
    */
    void push_co( rt::CoRoutineSPtr const& co )
    {
        constexpr int32_t MAX_TASK_NUM = 65535;
        size_t index = ( size_t )MAX_TASK_NUM;
        int32_t min_playloads = MAX_TASK_NUM;
        for ( size_t i = 0; i < tasks_.size(); ++i )
        {
            if ( min_playloads > tasks_[i]->payloads_ )
            {
                min_playloads = tasks_[i]->payloads_;
                index = i;
            }
        }
        TARO_ASSERT( index != MAX_TASK_NUM );
        ++tasks_[index]->payloads_;
        co->on_exit( [this, index](){ --tasks_[index]->payloads_; } );
        tasks_[index]->task_->append( co );
    }

    Graph graphy_;
    std::atomic_bool running_;
    std::vector< SchedTaskPtr > tasks_;
};

NAMESPACE_TARO_TASKFLOW_END
