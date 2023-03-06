
#include "base/error_no.h"
#include "base/utils/assert.h"
#include "base/system/thread.h"
#include <Winsock2.h>
#include <Windows.h>
#include <process.h>

NAMESPACE_TARO_BEGIN

int32_t get_prio( EPriority thrd_prio )
{
    int32_t prior = 0;
    int32_t min_prior = THREAD_PRIORITY_IDLE;
    int32_t max_prior = THREAD_PRIORITY_TIME_CRITICAL;
    constexpr int32_t THRD_PRIOR_TOP = 1;
    constexpr int32_t THRD_PRIOR_MID = 64;
    constexpr int32_t THRD_PRIOR_LOW = 127;

    int32_t real_prior = ( ePriorityMid == thrd_prio ) ? THRD_PRIOR_MID : THRD_PRIOR_LOW;
    prior = max_prior - ( real_prior - THRD_PRIOR_TOP ) * ( max_prior - min_prior ) / ( THRD_PRIOR_LOW - THRD_PRIOR_TOP );
    if( prior <= THREAD_PRIORITY_IDLE )
    {
        prior = THREAD_PRIORITY_IDLE;
    }
    else if( prior <= THREAD_PRIORITY_LOWEST )
    {
        prior = THREAD_PRIORITY_LOWEST;
    }
    else if( prior <= THREAD_PRIORITY_BELOW_NORMAL )
    {
        prior = THREAD_PRIORITY_BELOW_NORMAL;
    }
    else if( prior <= THREAD_PRIORITY_NORMAL )
    {
        prior = THREAD_PRIORITY_NORMAL;
    }
    else if( prior >= THREAD_PRIORITY_TIME_CRITICAL )
    {
        prior = THREAD_PRIORITY_TIME_CRITICAL;
    }
    else if( prior >= THREAD_PRIORITY_HIGHEST )
    {
        prior = THREAD_PRIORITY_HIGHEST;
    }
    else if( prior >= THREAD_PRIORITY_ABOVE_NORMAL )
    {
        prior = THREAD_PRIORITY_ABOVE_NORMAL;
    }
    else if( prior >= THREAD_PRIORITY_NORMAL )
    {
        prior = THREAD_PRIORITY_NORMAL;
    }
    else
    {
        TARO_ASSERT( 0, "prior", prior );
    }
    return prior;
}

struct ThreadImpl
{
    ThreadImpl()
        : handler_( NULL )
    {

    }

    uintptr_t             handler_;
    std::string           name_;
    std::function<void()> func_;
};

static unsigned __stdcall thread_proc( void* arg )
{
    TARO_ASSERT( nullptr != arg );
    reinterpret_cast< ThreadImpl* >( arg )->func_();
    return 0;
}

Thread::Thread( const char* name )
    : impl_( new ThreadImpl )
{
    impl_->name_ = name;
}

Thread::Thread( Thread&& other )
    : impl_( other.impl_ )
{
    other.impl_ = nullptr;
}

Thread::~Thread()
{
    if( impl_ == nullptr )
    {
        return;
    }

    join();
    delete impl_;
    impl_ = nullptr;
}

std::string Thread::name() const
{
    return impl_->name_;
}

int32_t Thread::start( std::function<void()> const& func, ThrdAttr const& attr )
{
    auto stack_sz = attr.stack_size;
    constexpr int32_t MIN_STACK_SZ = 128 * 1024;
    if( stack_sz < MIN_STACK_SZ )
    {
        stack_sz = MIN_STACK_SZ;
    }

    unsigned int id;
    impl_->func_    = func;
    impl_->handler_ = _beginthreadex( NULL, stack_sz, thread_proc, ( void* )impl_, 0, &id );
    if( NULL == impl_->handler_ )
    {
        set_errno( TARO_ERR_FAILED );
        return false;
    }

    CHECK_TRUE_RET( SetThreadPriority( ( HANDLE )impl_->handler_, get_prio( attr.prio ) ), false );
    if( attr.cpu_num > 0 )
    {
        DWORD_PTR mask = 0;
        for( int32_t i = 0; i < attr.cpu_num; ++i )
        {
            mask |= ( ( DWORD_PTR )1 ) << attr.cpus[i];
        }
        CHECK_TRUE_RET( SetThreadAffinityMask( ( HANDLE )impl_->handler_, mask ), false );
    }
    return true;
}

void Thread::join()
{
    if( NULL != impl_->handler_ )
    {
        WaitForSingleObject( ( HANDLE )impl_->handler_, INFINITE );
        impl_->handler_ = NULL;
    }
}

int64_t Thread::current_tid()
{
    return ( int64_t )GetCurrentThreadId();
}

void Thread::sleep( int64_t millisec )
{
    Sleep( ( DWORD )millisec );
}

NAMESPACE_TARO_END
