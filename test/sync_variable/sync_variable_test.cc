
#include "base/error_no.h"
#include "base/system/time.h"
#include "base/system/sync_variable.h"
#include <thread>

USING_NAMESPACE_TARO

void test1()
{
    ConditionVariable cond;
    Mutex mutex;

    int count = 30;
    mutex.lock();
    while( count-- > 0 )
    {
        SteadyTime tm;
        auto ret = cond.wait( mutex, 20 );
        if ( ret == TARO_ERR_TIMEOUT )
        {
            printf( "timeout %d\n", ( int )tm.cost() );
        }
    }
}

void test2()
{
    ConditionVariable cond;
    Mutex mutex;

    int count = 0;
    std::thread thrd( [&]()
    {
        std::this_thread::sleep_for( std::chrono::microseconds( 1200 * 1000 ) );
        mutex.lock();
        count = 1;
        mutex.unlock();
        cond.notify_one();
    } );

    mutex.lock();
    while( count == 0 )
    {
        SteadyTime tm;
        auto ret = cond.wait( mutex, 1000 );
        if( ret == TARO_ERR_TIMEOUT )
        {
            printf( "timeout %d\n", ( int )tm.cost() );
        }
        else
        {
            printf( "wakeup %d\n", ( int )tm.cost() );
        }
        mutex.unlock();
    }
    thrd.join();
}

void test3()
{
    ConditionVariable cond;
    Mutex mutex;

    int count = 0;
    std::thread thrd1( [&]()
    {
        mutex.lock();
        while( count == 0 )
        {
            SteadyTime tm;
            auto ret = cond.wait( mutex, 1000 );
            if( ret == TARO_ERR_TIMEOUT )
            {
                printf( "timeout1 %d\n", ( int )tm.cost() );
            }
            else
            {
                printf( "wakeup1 %d\n", ( int )tm.cost() );
            }
            mutex.unlock();
        }
    } );

    std::thread thrd2( [&]()
    {
        mutex.lock();
        while( count == 0 )
        {
            SteadyTime tm;
            auto ret = cond.wait( mutex, 1000 );
            if( ret == TARO_ERR_TIMEOUT )
            {
                printf( "timeout2 %d\n", ( int )tm.cost() );
            }
            else
            {
                printf( "wakeup2 %d\n", ( int )tm.cost() );
            }
            mutex.unlock();
        }
    } );

    std::this_thread::sleep_for( std::chrono::microseconds( 1200 * 1000 ) );
    mutex.lock();
    count = 1;
    mutex.unlock();
    cond.notify_all();

    thrd1.join();
    thrd2.join();
}

int main()
{
    test1();
    return 0;
}
