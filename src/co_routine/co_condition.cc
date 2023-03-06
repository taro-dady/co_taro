
#include "base/utils/assert.h"
#include "co_routine/impl/common.h"
#include "base/system/spin_locker.h"
#include "co_routine/co_condition.h"
#include "co_routine/impl/co_scheduler.h"
#include "co_routine/impl/co_routine_impl.h"
#include <map>
#include <atomic>
#include <functional>

NAMESPACE_TARO_RT_BEGIN

using CoWaitQue = std::map< uint32_t, std::function<void() > >;

struct Condition::Impl
{
    Impl() : seq_( 0 ){}

    uint32_t   seq_;
    SpinLocker mutex_;
    CoWaitQue  wait_que_;
};

Condition::Condition()
    : impl_( new Impl )
{

}

Condition::~Condition()
{
    delete impl_;
}

void Condition::notify_one()
{
    std::lock_guard<SpinLocker> g( impl_->mutex_ );
    auto tmp = impl_->wait_que_;
    auto it = impl_->wait_que_.begin();
    if ( it != impl_->wait_que_.end() )
    {
        it->second();
        impl_->wait_que_.erase( it );
    }
}

void Condition::notify_all()
{
    std::lock_guard<SpinLocker> g( impl_->mutex_ );
    while ( !impl_->wait_que_.empty() )
    {
        TARO_ASSERT( impl_->wait_que_.begin()->second );
        impl_->wait_que_.begin()->second();
        impl_->wait_que_.erase( impl_->wait_que_.begin() );
    }
}

bool Condition::cond_wait( uint32_t ms )
{
    CoRoutineSPtr   co    = CoRoutine::current();
    CoSchedulerSPtr sched = get_scheduler();
    if ( 0 == ms )
    {
        {
            std::lock_guard<SpinLocker> g( impl_->mutex_ );
            impl_->wait_que_.insert( std::make_pair( impl_->seq_++, [=]()
            {
                sched->push( co );
            } ) );
        }
        co->yield();
        return true;
    }
    
    uint32_t seq;
    int32_t  evt_id = -1;
    {
        // 由notify触发的调度
        std::lock_guard<SpinLocker> g( impl_->mutex_ );
        seq = impl_->seq_++;
        impl_->wait_que_.insert
        ( 
            std::make_pair
            (   
                seq,  
                [=, &evt_id]()
                {
                    sched->remove_event( evt_id );
                    sched->push( co );
                }
            ) 
        );
    }

    bool ok = true;
    // 定时器超时触发的调度
    evt_id  = sched->add_event( [&, seq, co, sched]()
    {
        ok = false;
        {
            std::lock_guard<SpinLocker> g( impl_->mutex_ );
            impl_->wait_que_.erase( seq );
        }
        sched->push( co );
    }, ms );

    co->yield();
    return ok;
}

NAMESPACE_TARO_RT_END
