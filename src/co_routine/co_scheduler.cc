
#include "base/system/time.h"
#include "base/system/thread.h"
#include "co_routine/impl/common.h"
#include "co_routine/impl/co_routine_impl.h"
#include "co_routine/impl/cond_evt_poller.h"
#include "co_routine/impl/default_scheduler.h"

NAMESPACE_TARO_RT_BEGIN

DefaultScheduler::DefaultScheduler()
    : looping_( true )
    , tid_( get_tid() )
    , seq_( 0 )
    , cnt_( 0 )
    , evt_poller_( std::make_shared<CondEvtPoller>() )
{
    for ( size_t i = 0; i < ePriorityNum; ++i )
    {
        co_que_[i].init( 32 );
    }
}

const char* DefaultScheduler::type() const
{
    return "Default";
}

void DefaultScheduler::push( CoRoutineSPtr const& co )
{
    if ( nullptr == co )
    {
        RT_ERROR << "co routine invalid";
        return;
    }

    int32_t index = ( int32_t )co->priority();
    co_que_[index].push_back( co );
    ++cnt_;

    // 不是同一个线程需要通知该线程调度器进行协程调度
    if ( tid_ != get_tid() )
    {
        evt_poller_->wakeup();
    }
}

int32_t DefaultScheduler::add_event( std::function<void()> const& evt_cb, int32_t ms )
{
    if ( !evt_cb || ms <= 0 )
    {
        RT_ERROR << "invalid argument";
        return -1;
    }

    auto eid = ++seq_;
    std::lock_guard<SpinLocker> g( evt_mtx_ );
    co_evt_.insert( std::make_pair( SteadyTime::current_ms() + ms, EventItem{ eid, evt_cb } ) );
    return eid;
}

bool DefaultScheduler::remove_event( int32_t eid )
{
    std::lock_guard<SpinLocker> g( evt_mtx_ );
    auto iter = co_evt_.begin();
    for ( ; iter != co_evt_.end(); ++iter )
    {
        if ( iter->second.evtid == eid )
        {
            co_evt_.erase( iter );
            return true;
        }
    }
    return false;
}

void DefaultScheduler::exit()
{
    looping_ = false;
    evt_poller_->wakeup();
}

void DefaultScheduler::run_until_no_task()
{
    while( has_co_routine() )
    {
        CoRoutineSPtr co;

        // 根据优先级遍历所有协程队列，优先调用高优先级协程
        int32_t index = ( int32_t )ePriorityNum - 1;
        while( index >= ( int32_t )ePriorityLow )
        {
            auto& co_que = co_que_[index];
            if ( co_que.pop_front( co ) )
            {
                break;
            }
            --index;
        }

        if ( co != nullptr )
        {
            --cnt_;
            CoRoutineImpl::resume( co );
            continue;
        }

        // 重置当前协程
        CoRoutineImpl::reset_curr_co();

        // 获取距离当前最近的事件触发时间，作为等待时间
        auto ms = wait_time();
        if( cnt_.load() > 0 )
        {
            // 如果在处理定时器回调时出现协程调度的需求则优先处理
            continue;
        }

        if ( ms == (int32_t)TARO_FOREVER && !has_co_routine() )
        {
            break;
        }

        SteadyTime tm;
        evt_poller_->wait( ms );
        if( ms != ( int32_t )TARO_FOREVER )
        {
            poll_timeout_evt();
        }
    }
}

void DefaultScheduler::run()
{
    while( looping_ )
    {
        CoRoutineSPtr co;

        // 根据优先级遍历所有协程队列，优先调用高优先级协程
        int32_t index = ( int32_t )ePriorityNum - 1;
        while( index >= ( int32_t )ePriorityLow )
        {
            auto& co_que = co_que_[index];
            if ( co_que.pop_front( co ) )
            {
                break;
            }
            --index;
        }

        if ( co != nullptr )
        {
            --cnt_;
            CoRoutineImpl::resume( co );
            continue;
        }

        // 重置当前协程
        CoRoutineImpl::reset_curr_co();

        // 获取距离当前最近的事件触发时间，作为等待时间
        auto ms = wait_time();
        if( cnt_.load() > 0 )
        {
            // 如果在处理定时器回调时出现协程调度的需求则优先处理
            continue;
        }

        SteadyTime tm;
        evt_poller_->wait( ms );
        if( ms != ( int32_t )TARO_FOREVER )
        {
            poll_timeout_evt();
        }
    }
}

void DefaultScheduler::set_evt_poller( EventPollerSPtr const& poller )
{
    evt_poller_ = poller;
}

int32_t DefaultScheduler::wait_time()
{
    if( co_evt_.empty() )
    {
        return TARO_FOREVER;
    }

    auto now = SteadyTime::current_ms();
    poll_timeout_evt( now ); // 处理所有超时的事件，剩下的事件必然没有到触发时间
    if ( !co_evt_.empty() )
    {
        return ( int32_t )( co_evt_.begin()->first - now );
    }
    return TARO_FOREVER;
}

void DefaultScheduler::poll_timeout_evt( int64_t now )
{
    auto iter = co_evt_.begin();
    while( iter != co_evt_.end() && iter->first <= now )
    {
        if ( iter->second.cb )
        {
            iter->second.cb();
        }
        co_evt_.erase( iter );
        iter = co_evt_.begin();
    }
}

NAMESPACE_TARO_RT_END
