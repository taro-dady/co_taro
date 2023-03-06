
#include "base/error_no.h"
#include "base/system/thread.h"
#include "co_routine/impl/common.h"
#include "co_routine/impl/co_task.h"
#include "co_routine/impl/co_scheduler.h"
#include "co_routine/impl/co_routine_impl.h"
#include <list>
#include <mutex>
#include <atomic>

NAMESPACE_TARO_RT_BEGIN

extern void co_loop();

struct CoTask::Impl
{
    Impl( const char* name )
        : thrd( name )
        , start( false )
        , tid( -1 )
    {
        
    }

    Thread                   thrd;
    std::mutex               co_mtx;
    CoSchedulerSPtr          sched;
    std::atomic_bool         start;
    std::atomic<int64_t>     tid;
    std::list<CoRoutineSPtr> co_que;
};

CoTask::CoTask( const char* name )
    : impl_( new Impl( name ) )
{

}

CoTask::~CoTask()
{
    join();
}

bool CoTask::append( CoRoutineSPtr const& co )
{
    if ( nullptr == co )
    {
        RT_ERROR << "routine is invalid";
        return false;
    }

    std::lock_guard<std::mutex> g( impl_->co_mtx );
    if ( impl_->sched == nullptr )
    {
        impl_->co_que.push_back( co );
        return true;
    }
    impl_->sched->push( co );
    return true;
}

bool CoTask::start( ThrdAttr const& attr )
{
    bool expr = false;
    if ( !impl_->start.compare_exchange_strong( expr, true ) )
    {
        return true;
    }

    auto ret = impl_->thrd.start( [this]()
    {
        impl_->tid = Thread::current_tid();
        {
            std::lock_guard<std::mutex> g( impl_->co_mtx );
            impl_->sched = get_scheduler();
            for ( auto& one : impl_->co_que )
            {
                impl_->sched->push( one );
            }
            impl_->co_que.clear();
        }
        impl_->sched->run();
    }, attr );

    return ret;
}

void CoTask::join()
{
    impl_->thrd.join();
}

int64_t CoTask::tid()
{
    return impl_->tid;
}

std::string CoTask::name() const
{
    return impl_->thrd.name();
}

NAMESPACE_TARO_RT_END
