
#include "task_flow/impl/task_flow_impl.h"

NAMESPACE_TARO_TASKFLOW_BEGIN

TaskMessage::TaskMessage( const char* src, const char* type, uint32_t sequence, int64_t ts )
    : impl_( new TaskMessageImpl )
{
    impl_->src_        = src;
    impl_->type_       = type;
    impl_->sequence_   = sequence;
    impl_->time_stamp_ = ts;
}

TaskMessage::~TaskMessage()
{
    delete impl_;
}

const char* TaskMessage::source() const
{
    return impl_->src_.empty() ? nullptr : impl_->src_.c_str();
}

void TaskMessage::set_source( const char* src )
{
    impl_->src_ = src;
}

const char* TaskMessage::type() const
{
    return impl_->type_.empty() ? nullptr : impl_->type_.c_str();
}

void TaskMessage::set_type( const char* type )
{
    impl_->type_ = type;
}

uint32_t TaskMessage::sequence() const
{
    return impl_->sequence_;
}

void TaskMessage::set_sequence( uint32_t sequence )
{
    impl_->sequence_ = sequence;
}

int64_t TaskMessage::time_stamp() const
{
    return impl_->time_stamp_;
}

void TaskMessage::set_time_stamp( int64_t ts )
{
    impl_->time_stamp_ = ts;
}

NAMESPACE_TARO_TASKFLOW_END
