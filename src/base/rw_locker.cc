
#include "base/system/rw_locker.h"
#include <atomic>
#include <thread>

NAMESPACE_TARO_BEGIN

constexpr int32_t WRITING       = -1;
constexpr int32_t MAX_RETRY_CNT = 5;

struct RWLocker::Impl
{
    Impl()
        : lock_num_( 0 )
        , writing_num_( 0 )
    {}

    std::atomic_int32_t lock_num_;
    std::atomic_int32_t writing_num_;
};

RWLocker::RWLocker()
    : impl_( new Impl )
{
    
}

RWLocker::~RWLocker()
{
    delete impl_;
}

void RWLocker::read_lock()
{
    int32_t max_retry = MAX_RETRY_CNT;
    auto lock_num = impl_->lock_num_.load();
    do
    {
        while ( lock_num == WRITING || impl_->writing_num_.load() > 0 )
        {
            if ( --max_retry == 0 )
            {
                std::this_thread::yield();
                max_retry = MAX_RETRY_CNT;
            }
            lock_num = impl_->lock_num_.load();
        }
    }while( impl_->lock_num_.compare_exchange_weak( lock_num, lock_num + 1, 
                                                    std::memory_order_acq_rel,
                                                    std::memory_order_relaxed ) );
}

void RWLocker::write_lock()
{
    auto expr = 0;
    int32_t max_retry = MAX_RETRY_CNT;

    impl_->writing_num_.fetch_add( 1 );
    while( impl_->lock_num_.compare_exchange_weak( expr, WRITING, 
                                                   std::memory_order_acq_rel,
                                                   std::memory_order_relaxed ) )
    {
        if ( --max_retry == 0 )
        {
            std::this_thread::yield();
            max_retry = MAX_RETRY_CNT;
        }
        expr = 0;
    }
    impl_->writing_num_.fetch_sub( 1 );
}

void RWLocker::read_unlock()
{
    impl_->lock_num_.fetch_sub( 1 );
}

void RWLocker::write_unlock()
{
    impl_->lock_num_.fetch_add( 1 );
}

WriteGuard::WriteGuard( RWLocker& locker )
    : locker_( locker )
{
    locker_.write_lock();
}

WriteGuard::~WriteGuard()
{
    locker_.write_unlock();
}

ReadGuard::ReadGuard( RWLocker& locker )
    : locker_( locker )
{
    locker_.read_lock();
}

ReadGuard::~ReadGuard()
{
    locker_.read_unlock();
}

NAMESPACE_TARO_END
