
#include "co_routine/co_wrapper.h"
#include "co_routine/impl/common.h"
#include "co_routine/impl/co_task.h"
#include <list>
#include <algorithm>

NAMESPACE_TARO_RT_BEGIN

// 协程参数，创建协程时使用
struct CoRoutineParam
{
    CoRoutineParam()
        : stack_sz( 0 )
        , prio( ePriorityMid )
    {}

    size_t                stack_sz;
    EPriority             prio;
    std::string           name;
    std::function<void()> func;
};

struct CoRunner::Impl
{
    CoRoutineParam param_;
};

CoRunner::CoRunner( std::string const& name )
    : impl_( new Impl )
{
    impl_->param_.name = name;
}

CoRunner::~CoRunner()
{
    CoRoutine::create
    ( 
        impl_->param_.name.c_str(),
        impl_->param_.func,
        true,
        impl_->param_.prio,
        ( uint32_t )impl_->param_.stack_sz
    );
    delete impl_;
}

CoRunner& CoRunner::operator,( CoAttrOpt<eCoAttrTypeName> const& opt )
{
    impl_->param_.name = opt.arg;
    return *this;
}

CoRunner& CoRunner::operator,( CoAttrOpt<eCoAttrTypeStackSize> const& opt )
{
    impl_->param_.stack_sz = opt.arg;
    return *this;
}

CoRunner& CoRunner::operator,( CoAttrOpt<eCoAttrTypePriority> const& opt )
{
    impl_->param_.prio = opt.arg;
    return *this;
}

CoRunner& CoRunner::operator<<( std::function<void()> const& f )
{
    impl_->param_.func = f;
    return *this;
}

struct CoThread::Impl
{
    ThrdAttr                           thrd_attr_;
    std::string                        name_;
    std::unique_ptr<CoRoutineParam>    co_param_;
    std::list<taro::rt::CoRoutineSPtr> co_list_;
};

CoThread::CoThread( std::string const& name )
    : impl_( new Impl )
{
    impl_->name_ = name;
}

CoThread::~CoThread()
{
    auto co = CoRoutine::create
            ( 
                impl_->co_param_->name.c_str(), 
                impl_->co_param_->func,
                false,
                impl_->co_param_->prio,
                ( uint32_t )impl_->co_param_->stack_sz
            );
    impl_->co_list_.push_back( co );

    auto task = std::make_shared<CoTask>( impl_->name_.c_str() );
    for( auto const& one : impl_->co_list_ )
    {
        task->append( one );
    }
    task->start( impl_->thrd_attr_ );
    CoTaskContainer::instance().push( task );
    delete impl_;
}

CoThread& CoThread::attr( CoAttrOpt<eCoAttrTypeStackSize> const& opt )
{
    impl_->thrd_attr_.stack_size = ( int32_t )opt.arg;
    return *this;
}

CoThread& CoThread::attr( CoAttrOpt<eCoAttrTypeName> const& opt )
{
    impl_->name_ = opt.arg;
    return *this;
}

CoThread& CoThread::attr( CoAttrOpt<eCoAttrTypePriority> const& opt )
{
    impl_->thrd_attr_.prio = opt.arg;
    return *this;
}

CoThread& CoThread::attr( CoAttrOpt<eCoAttrTypeCpuAffinity> const& opt )
{
    impl_->thrd_attr_.cpu_num = ( int32_t )opt.cpus.size();
    for ( int32_t i = 0; i < impl_->thrd_attr_.cpu_num; ++i )
    {
        impl_->thrd_attr_.cpus[i] = opt.cpus[i];
    }
    return *this;
}

CoThread& CoThread::append( std::function<void()> const& co_func, std::string const& co_name )
{
    if ( impl_->co_param_ == nullptr )
    {
        impl_->co_param_.reset( new CoRoutineParam );
    }
    else
    {
        auto co = CoRoutine::create( 
            impl_->co_param_->name.c_str(), 
            impl_->co_param_->func,
            false,
            impl_->co_param_->prio,
            ( uint32_t )impl_->co_param_->stack_sz
        );
        impl_->co_list_.push_back( co );
        new ( impl_->co_param_.get() ) CoRoutineParam();
    }
    impl_->co_param_->func = co_func;
    impl_->co_param_->name = co_name;

    return *this;
}

CoThread& CoThread::co_arg( CoAttrOpt<eCoAttrTypeStackSize> const& opt )
{
    impl_->co_param_->stack_sz = opt.arg;
    return *this;
}

CoThread& CoThread::co_arg( CoAttrOpt<eCoAttrTypeName> const& opt )
{
    impl_->co_param_->name = opt.arg;
    return *this;
}

CoThread& CoThread::co_arg( CoAttrOpt<eCoAttrTypePriority> const& opt )
{
    impl_->co_param_->prio = opt.arg;
    return *this;
}

NAMESPACE_TARO_RT_END
