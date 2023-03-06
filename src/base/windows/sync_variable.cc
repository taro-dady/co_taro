
#include "base/error_no.h"
#include "base/system/spin_locker.h"
#include "base/system/sync_variable.h"
#include <set>
#include <mutex>
#include <memory>
#include <windows.h>
#include <Mmsystem.h>

#pragma comment(lib, "Winmm.lib")

NAMESPACE_TARO_BEGIN

struct Mutex::Impl
{
    HANDLE handle_;
};

struct CondVariableImpl
{
    SpinLocker locker_;
    std::set<HANDLE> events_;
};

struct TimeoutWakeup
{
    HANDLE handle;
    bool*  tm;
};

static void on_time_out( UINT, UINT, DWORD_PTR dwUser, DWORD_PTR, DWORD_PTR )
{
    TimeoutWakeup* wk = ( TimeoutWakeup* )dwUser;
    *wk->tm = true;
    SetEvent( wk->handle );
}

Mutex::Mutex()
    : impl_( new Impl )
{
    impl_->handle_ = CreateMutex( NULL, FALSE, NULL );
}

Mutex::~Mutex()
{
    CloseHandle( impl_->handle_ );
    delete impl_;
}

void Mutex::lock()
{
    WaitForSingleObject( impl_->handle_, INFINITE );
}

void Mutex::unlock()
{
    ReleaseMutex( impl_->handle_ );
}

ConditionVariable::ConditionVariable()
    : impl_( new CondVariableImpl )
{

}

ConditionVariable::~ConditionVariable()
{
    {
        std::lock_guard<SpinLocker> g( impl_->locker_ );
        for( auto& one : impl_->events_ )
        {
            CloseHandle( one );
        }
        impl_->events_.clear();
    }
    delete impl_;
}

void ConditionVariable::wait( Mutex& mutex )
{
    auto handle = CreateEvent( NULL, FALSE, FALSE , NULL );
    if ( INVALID_HANDLE_VALUE == handle )
    {
        printf( "create event failed.\n" );
        return;
    }

    {
        std::lock_guard<SpinLocker> g( impl_->locker_ );
        impl_->events_.insert( handle );
    }
    
    mutex.unlock();
    WaitForSingleObject( handle, INFINITE );
    mutex.lock();

    {
        std::lock_guard<SpinLocker> g( impl_->locker_ );
        impl_->events_.erase( handle );
    }
    ( void )CloseHandle( handle );
}

int32_t ConditionVariable::wait( Mutex& mutex, uint32_t ms )
{
    if ( ms == 0 )
    {
        wait( mutex );
        return TARO_OK;
    }

    auto handle = CreateEvent( NULL, FALSE, FALSE , NULL );
    if ( INVALID_HANDLE_VALUE == handle )
    {
        printf( "create event failed.\n" );
        return TARO_ERR_FAILED;
    }

    {
        std::lock_guard<SpinLocker> g( impl_->locker_ );
        impl_->events_.insert( handle );
    }

    bool timeout = false;
    TimeoutWakeup wk;
    wk.handle   = handle;
    wk.tm       = &timeout;
    uint32_t id = ::timeSetEvent( ms, 0, ( LPTIMECALLBACK )on_time_out, ( DWORD_PTR )&wk, TIME_ONESHOT );

    mutex.unlock();
    WaitForSingleObject( handle, INFINITE );
    mutex.lock();

    {
        std::lock_guard<SpinLocker> g( impl_->locker_ );
        impl_->events_.erase( handle );
    }

    ( void )timeKillEvent( id );
    ( void )CloseHandle( handle );
    return timeout ? TARO_ERR_TIMEOUT : TARO_OK;
}

void ConditionVariable::notify_one()
{
    HANDLE front = INVALID_HANDLE_VALUE;
    {
        std::lock_guard<SpinLocker> g( impl_->locker_ );
        if( !impl_->events_.empty() )
        {
            front = ( *impl_->events_.begin() );
        }
    }

    if ( front != INVALID_HANDLE_VALUE )
    {
        SetEvent( front );
    }
}

void ConditionVariable::notify_all()
{
    std::set<HANDLE> bk;
    {
        std::lock_guard<SpinLocker> g( impl_->locker_ );
        bk = impl_->events_;
        impl_->events_.clear();
    }

    for ( auto& one : bk )
    {
        SetEvent( one );
    }
}

NAMESPACE_TARO_END
