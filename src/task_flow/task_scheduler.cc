
#include "task_flow/impl/task_flow_impl.h"

NAMESPACE_TARO_TASKFLOW_BEGIN

TaskScheduler::TaskScheduler()
    : impl_( new Impl )
{

}

TaskScheduler::~TaskScheduler()
{
    delete impl_;
}

int32_t TaskScheduler::start( int32_t thrd_num )
{
    if ( thrd_num <= 0 )
    {
        TASKFLOW_ERROR << "parameter invalid";
        return TARO_ERR_INVALID_ARG;
    }

    bool expr = false;
    if ( !impl_->running_.compare_exchange_strong( expr, true ) )
    {
        TASKFLOW_ERROR << "scheduler is running";
        return TARO_ERR_MULTI_OP;
    }

    auto nodes = impl_->graphy_.all_nodes();
    for( auto& one : nodes )
    {
        if ( TARO_OK != std::dynamic_pointer_cast<TaskNode>( one )->init() )
        {
            impl_->running_ = false;
            TASKFLOW_ERROR << "task:" << one->name() << "init failed";
            return TARO_ERR_FAILED;
        }
    }

    for( int32_t i = 0; i < thrd_num; ++i )
    {
        impl_->tasks_.emplace_back( new SchedTask );
        if ( !impl_->tasks_.back()->task_->start() )
        {
            impl_->tasks_.clear();
            impl_->running_ = false;
            TASKFLOW_ERROR << "start co task failed";
            return TARO_ERR_FAILED;
        }
    }
    return TARO_OK;
}

int32_t TaskScheduler::stop()
{
    bool expr = true;
    if ( !impl_->running_.compare_exchange_strong( expr, false ) )
    {
        TASKFLOW_ERROR << "scheduler is stopped";
        return TARO_ERR_MULTI_OP;
    }
    
    impl_->tasks_.clear();
    auto nodes = impl_->graphy_.all_nodes();
    for( auto& one : nodes )
    {
        if ( TARO_OK != std::dynamic_pointer_cast<TaskNode>( one )->finish() )
        {
            TASKFLOW_ERROR << "task:" << one->name() << "finish failed";
            return TARO_ERR_FAILED;
        }
    }
    return TARO_OK;
}

int32_t TaskScheduler::add_node( TaskNodeSPtr const& node )
{
    if ( nullptr == node )
    {
        TASKFLOW_ERROR << "node invalid";
        return TARO_ERR_INVALID_ARG;
    }
    node->impl_->feed_func_ = std::bind( &TaskScheduler::Impl::next, impl_, std::placeholders::_1, std::placeholders::_2 );
    return impl_->graphy_.add_node( node );
}

int32_t TaskScheduler::remove_node( const char* name )
{
    return impl_->graphy_.remove_node( name );
}

int32_t TaskScheduler::find_node( const char* name, TaskNodeSPtr& node )
{
    GraphyNodeSPtr graph_node;
    auto ret = impl_->graphy_.find_node( name, graph_node );
    if ( TARO_OK == ret && graph_node != nullptr )
    {
        node = std::dynamic_pointer_cast<TaskNode>( graph_node );
    }
    return ret;
}

int32_t TaskScheduler::add_edge( const char* src, const char* dst )
{
    return impl_->graphy_.add_edge( src, dst );
}

int32_t TaskScheduler::remove_edge( const char* src, const char* dst )
{
    return impl_->graphy_.remove_edge( src, dst );
}

bool TaskScheduler::has_circle( std::string& node )
{
    return impl_->graphy_.has_circle( node );
}

NAMESPACE_TARO_TASKFLOW_END
