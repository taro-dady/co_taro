
#include "task_flow/impl/task_flow_impl.h"

NAMESPACE_TARO_TASKFLOW_BEGIN

TaskNode::TaskNode( const char* name )
    : GraphyNode( name )
    , impl_( new TaskNodeImpl )
{

}

TaskNode::~TaskNode()
{
    delete impl_;
}

int32_t TaskNode::init()
{
    return TARO_OK;
}

int32_t TaskNode::finish()
{
    return TARO_OK;
}

TaskMessageSPtr TaskNode::on_message( TaskMessageSPtr const& )
{
    return nullptr;
}

void TaskNode::feed( TaskMessageSPtr const& msg )
{
    TARO_ASSERT( impl_->feed_func_ );
    impl_->feed_func_( this, msg );
}

NAMESPACE_TARO_TASKFLOW_END
