
#pragma once

#include "base/base.h"
#include <atomic>

NAMESPACE_TARO_BEGIN

#if defined(_WIN32) || defined(_WIN64)

class SpinLocker
{
PUBLIC:

    void lock()
    {
        static volatile char dummy;
        while ( locked_.test_and_set(std::memory_order_acquire) )
        {
            ( void )dummy;
        }
    }

    void unlock()
    {
        locked_.clear(std::memory_order_release);
    }

PRIVATE:

    std::atomic_flag locked_ = ATOMIC_FLAG_INIT;
};

#else

class SpinLocker 
{
PUBLIC:

    SpinLocker()
        : locked_( ATOMIC_FLAG_INIT )
    {

    }

    void lock()
    {
        while( locked_.test_and_set( std::memory_order_acquire ) )
        {
            asm volatile("rep; nop");
        }
    }

    void unlock()
    {
        locked_.clear( std::memory_order_release );
    }

PRIVATE:

    std::atomic_flag locked_;
};

#endif

NAMESPACE_TARO_END
