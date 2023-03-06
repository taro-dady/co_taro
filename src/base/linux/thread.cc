
#include "base/error_no.h"
#include "base/utils/defer.h"
#include "base/utils/assert.h"
#include "base/system/thread.h"
#include "base/utils/arg_expander.h"
#include <thread>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/resource.h>

NAMESPACE_TARO_BEGIN

static bool init_attr( ThrdAttr const& thrd_attr, pthread_attr_t& attr )
{
    auto stack_sz = thrd_attr.stack_size;
    constexpr int32_t MIN_STACK_SZ = 128 * 1024;
    if ( stack_sz < MIN_STACK_SZ )
    {
        stack_sz = MIN_STACK_SZ;
    }
    CHECK_ZERO_RET( pthread_attr_setstacksize( &attr, stack_sz ), false );
    
    auto policy   = ( thrd_attr.policy == eThrdPolicyDefault ) ? SCHED_OTHER : SCHED_RR;
    auto max_prio = sched_get_priority_max( policy );
    auto min_prio = sched_get_priority_min( policy );
    int32_t prio  = min_prio;
    if ( thrd_attr.prio == ePriorityMid )
    {
        prio = ( max_prio + min_prio ) / 2;
    }
    else if ( thrd_attr.prio == ePriorityHigh )
    {
        prio = max_prio;
    }

    struct sched_param sp;
    sp.sched_priority = prio;
    CHECK_ZERO_RET( pthread_attr_setschedparam( &attr, &sp ), false );
    if ( SCHED_OTHER != policy )
    {
        CHECK_ZERO_RET( pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED ), false );
    }
    CHECK_ZERO_RET( pthread_attr_setschedpolicy( &attr, prio ), false );

    if ( thrd_attr.cpu_num > 0 )
    {
        cpu_set_t set;
        CPU_ZERO( &set );
        for ( int32_t i = 0; i < thrd_attr.cpu_num; ++i )
        {
            CPU_SET( thrd_attr.cpus[i], &set );
        }
        CHECK_ZERO_RET( pthread_attr_setaffinity_np( &attr, sizeof( cpu_set_t ), &set ), false );
    }
    return true;
}

struct ThreadImpl
{
    ThreadImpl()
        : handler( 0 )
    {}

    pthread_t             handler;
    ThrdAttr              attr;
    std::string           name;
    std::function<void()> func;
};

static void* thread_proc( void *arg ) 
{
    TARO_ASSERT( nullptr != arg );
    reinterpret_cast< ThreadImpl* >( arg )->func();
    return nullptr;
}

Thread::Thread( const char* name )
    : impl_( new ThreadImpl )
{
    impl_->name = name;
}

Thread::Thread( Thread&& other )
    : impl_( other.impl_ )
{
    other.impl_ = nullptr;
}

Thread::~Thread()
{
    if ( impl_ == nullptr )
    {
        return;
    }
    
    join();
    delete impl_;
    impl_ = nullptr;
}

std::string Thread::name() const
{
    return impl_->name;
}

int32_t Thread::start( std::function<void()> const& func, ThrdAttr const& thrd_attr )
{
    pthread_attr_t attr;
    ( void )pthread_attr_init( &attr );
    Defer defer( [&](){ pthread_attr_destroy( &attr ); } );

    if ( !init_attr( thrd_attr, attr ) )
    {
        return TARO_ERR_INVALID_ARG;
    }

    impl_->func = func;
    impl_->attr = thrd_attr;
    return pthread_create( &impl_->handler, &attr, thread_proc, ( void* )impl_ ) != -1;
}

void Thread::join()
{
    if ( 0 != impl_->handler ) 
    {
        pthread_join( impl_->handler, 0 );
        impl_->handler = 0;
    }
}

int64_t Thread::current_tid()
{
    return ( int64_t )syscall( SYS_gettid );
}

void Thread::sleep( int64_t millisec )
{
    if ( 0 == millisec )
    {
        std::this_thread::yield();
        return;
    }

    struct timespec t;
    t.tv_sec  = millisec / 1000;
    t.tv_nsec = ( ( millisec % 1000 ) * 1000000 ); // 0 to 999 999 999
    while ( nanosleep( &t, &t ) != 0 );
}

NAMESPACE_TARO_END
