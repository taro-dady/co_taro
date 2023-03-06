
#include "base/error_no.h"
#include "base/system/rw_locker.h"
#include "base/system/sync_variable.h"
#include <sys/time.h>
#include <unistd.h> 

NAMESPACE_TARO_BEGIN

struct MonotonicTime
{
PUBLIC:

    MonotonicTime()
    {
        clock_gettime( CLOCK_MONOTONIC, &ts_ );
    }

    MonotonicTime& operator+=( int32_t ms )
    {
        // tv_usec < 1000000000 ns
        ts_.tv_sec += ms / 1000;
        uint64_t us = ts_.tv_nsec / 1000 + 1000 * (ms % 1000);
        ts_.tv_sec += us / 1000000;
        us = us % 1000000;
        ts_.tv_nsec = us * 1000;
        return *this;
    }

    struct timespec* get()
    {
        return &ts_;
    }

PRIVATE:

    struct timespec ts_;
};

struct Mutex::Impl
{
    pthread_mutex_t mutex_;
};  

Mutex::Mutex()
    : impl_( new Impl )
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init( &attr );
    pthread_mutex_init( &impl_->mutex_, &attr );
}

Mutex::~Mutex()
{
    delete impl_;
}

void Mutex::lock()
{
    pthread_mutex_lock( &impl_->mutex_ );
}

void Mutex::unlock()
{
    pthread_mutex_unlock( &impl_->mutex_ );
}

struct CondVariableImpl
{
    pthread_cond_t cond_;
};

ConditionVariable::ConditionVariable()
    : impl_( new CondVariableImpl )
{
    pthread_condattr_t attr;
    pthread_condattr_init( &attr );
    pthread_condattr_setclock( &attr, CLOCK_MONOTONIC );
    pthread_cond_init( &impl_->cond_, &attr );
}

ConditionVariable::~ConditionVariable()
{
    delete impl_;
}

void ConditionVariable::wait( Mutex& mutex )
{
    pthread_cond_wait( &impl_->cond_, &mutex.impl_->mutex_ );
}

int32_t ConditionVariable::wait( Mutex& mutex, uint32_t ms )
{
    MonotonicTime t;
    t += ( int32_t )ms;
    auto ret = pthread_cond_timedwait( &impl_->cond_, &mutex.impl_->mutex_, t.get() );
    return 0 == ret ? TARO_OK : TARO_ERR_TIMEOUT;
}

void ConditionVariable::notify_one()
{
    pthread_cond_signal( &impl_->cond_ ); 
}

void ConditionVariable::notify_all()
{
    pthread_cond_broadcast( &impl_->cond_ ); 
}

NAMESPACE_TARO_END
