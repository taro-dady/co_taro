
#include "co_routine/inc.h"
#include "base/system/time.h"
#include "base/function/inc.h"
#include <type_traits>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <ctime>
#include <iostream>
#include <cstdlib>
#include <atomic>
#include <future>       // std::packaged_task, std::future

USING_NAMESPACE_TARO
USING_NAMESPACE_TARO_RT

void test1()
{
    printf("test1\n");  
}

void test_default()
{
    co_run []()
    {
        printf( "r1 begin %s\n", SystemTime().to_ms_str().c_str() );
        co_wait();
        printf( "r1 back\n" );
        co_wait();
        printf( "r1 finish\n" );
    }, opt_stack( 1024 * 1024 * 1 );

    co_run []() 
    {
        printf( "r2 begin\n" );
        co_wait();
        printf( "r2 back\n" );
        co_wait();
        printf( "r2 finish\n" );
    };

    co_run test1, opt_prior( ePriorityHigh );

    rt::co_loop_until_no_tasks();
    std::cout << "test finished" << std::endl;
}

void test_sleep()
{
    co_run []()
    { 
        printf( "r1 begin\n" );

        SteadyTime tm;
        co_wait( 2000 );
        printf( "r1 back %d\n", ( int )tm.cost() );
        co_wait();
        printf( "r1 finish\n" );
    };

    co_run []()
    {
        printf( "r2 begin\n" );
        co_wait();
        printf( "r2 back\n" );
        co_wait();
        printf( "r2 finish\n" );
    };

    rt::co_loop_until_no_tasks();
    std::cout << "test finished" << std::endl;
}

void test_condition()
{
    Condition cond;
    std::mutex lock;
    std::list< int > dq;

    co_run [&]()
    {
        int v = 1;
        while( 1 )
        {
            std::unique_lock<std::mutex> g( lock );
            dq.push_back( v++ );
            cond.notify_all();
            g.unlock();

            SteadyTime tm;
            co_wait( 1000 );
            printf( "producer %d\n", ( int )tm.cost() );
        }
    };
 
    co_run [&]()
    {
        while( 1 )
        {
            SteadyTime tm;
            std::unique_lock<std::mutex> g( lock );
            cond.wait( g, [&]()
            {
                return !dq.empty();
            } );
            auto i = dq.front();
            dq.pop_front();
            g.unlock();
            printf( "consumer1 %d %d\n", i, ( int )tm.cost() );
        } 
    };

    co_run [&]()
    {
        while( 1 )
        {
            std::unique_lock<std::mutex> g( lock );
            cond.wait( g, [&]()
            {
                // consumer1会被先调度到，dq被清空，故此处判断始终失败
                printf( "c2 dq empty %s\n", dq.empty() ? "true" :"false");
                return !dq.empty();
            } );
            auto i = dq.front();
            dq.pop_front();
            g.unlock();
            printf( "consumer2 %d\n", i );
        }
    };

    rt::co_loop_until_no_tasks();
    std::cout << "test finished" << std::endl;
}

void test_channel()
{
    Channel<int> chn;

    std::atomic_int v( 0 );
    co_run [&]()
    { 
        while( 1 )
        {
            auto t = v++;
            printf( "write1 %d\n", t );
            chn.write( t );
            co_wait( 1000 );
        }
    };  

    co_run [&]()
    { 
        v = 1;
        while( 1 )
        {
            auto t = v++;
            printf( "write2 %d\n", t );
            t >> chn;
            co_wait( 1000 );
        }
    };  

    co_run [&]()
    {
        int data = -1;
        while( 1 )
        {
            data << chn;
            printf( "consumer1 %d\n", data );
        }
    };

    co_run [&]()
    {
        int data = -1;
        while( 1 )
        {
            data << chn;
            printf( "consumer2 %d\n", data );
        }
    };

    rt::co_loop();
}

void test_channel_old()
{
    Channel<int> chn( 8 );

    std::atomic_int v( 0 );
    co_run[&]()
    {
        int count = 0;
        while( 1 )
        {
            auto t = v++;
            printf( "write1 %d\n", t );
            chn.write( t );
            co_wait( 1000 );

            ++count;
            if( count == 8 )
            {
                co_run[&]()
                {
                    int data = -1;
                    while( 1 )
                    {
                        data << chn;
                        printf( "consumer1 %d\n", data );
                    }
                };
            }
        }
    };

    rt::co_loop();
}

void test_channel_new()
{
    Channel<int> chn( 8, true );

    std::atomic_int v( 0 );
    co_run[&]()
    {
        int count = 0;
        while( 1 )
        {
            auto t = v++;
            printf( "write1 %d\n", t );
            chn.write( t );
            co_wait( 1000 );

            ++count;
            if( count == 10 )
            {
                co_run[&]()
                {
                    int data = -1;
                    while( 1 )
                    {
                        data << chn;
                        printf( "consumer1 %d\n", data );
                    }
                };
            }
        }
    };

    rt::co_loop();
}

void test_channel_tm()
{
    Channel<int> chn;

    co_run[&]()
    {
        int data = -1;
        while( 1 )
        {
            SteadyTime tm;
            if( !chn.read( data, 500 ) ){
                printf( "consumer1 timeout %d\n", (int)tm.cost() );
            }else{
                printf( "consumer1 %d %d\n", data, ( int )tm.cost() );
            }
        }
    };

    std::atomic_int v( 0 );
    co_run[&]()
    {
        co_wait( 1000 );
        while( 1 )
        {
            auto t = v++;
            printf( "write1 %d\n", t );
            chn.write( t );
            co_wait( 1000 );
        }
    };

    rt::co_loop();
}

void test_co_task()
{
    Channel<int> chn;

    co_run [&]()
    {
        int v = 1;
        while( 1 )
        {
            auto t = v++;
            chn.write( t );
            printf("channel write %d\n", t);
            co_wait( 10 );
        }
    };

    co_run [&]()
    {
        while( 1 )
        {
            int data = -1;
            if( chn.read( data ) )
                printf( "consumer1 %d %d\n", data, rt::CoRoutine::current()->cid() );
            else
                printf( "consumer1 read failed\n" );
        }
    };

    co_run [&]()
    {
        while( 1 )
        {
            int data = -1;
            if( chn.read( data ) )
                printf( "consumer2 %d %d %d\n", data, (int)Thread::current_tid(), rt::CoRoutine::current()->cid() );
            else
                printf( "consumer2 read failed\n" );
        }
    };

    co_task
        .attr( opt_affinity( 1, 2, 3 ) )
        .append(
            [&]() 
            {
                while( 1 )
                {
                    int data = -1;
                    if( chn.read( data ) )
                        printf( "consumer3 %d %d %d\n", data, (int)Thread::current_tid(), rt::CoRoutine::current()->cid() );
                    else
                        printf( "consumer3 read failed\n" );
                }
            }, TRACE_NAME )
        .co_arg( opt_stack(1024) );
    rt::co_loop();
} 

void test_co_timer()
{
    co_task.append (
        [&](){
            CoTimer timer;
            auto last = SystemTime::current_ms();
            timer.start( 1000, 2000, [&](){
                printf( "task cost %d\n",  (int)( SystemTime::current_ms() - last ) );
                last = SystemTime::current_ms();
            } );

            while( 1 ) co_wait( 1000000 );
        }, TRACE_NAME 
    ).co_arg( opt_name("task") );

    co_run [&]()
    {
        CoTimer timer, timer1;
        auto last = SystemTime::current_ms();
        timer.start( 1000, 2000, [&]()
        {
            printf( "main timer cost %d\n", (int)( SystemTime::current_ms() - last ) );
            last = SystemTime::current_ms();
        } );

        timer1.once( 2000,  [&](){
            printf( "main timer1 once cost %d\n", (int)( SystemTime::current_ms() - last ) );
        } );
        
        while( 1 ) co_wait( 1000000 );
    };

    rt::co_loop();
}

int add( int a, int b )
{
    return a + b;
}

struct AsyncTest
{
    AsyncTest( int a, int b )
        : a_( a ), b_(b)
    {}

    int handle( int c )
    {
        return a_ + b_ + c;
    }

    int a_;
    int b_;
};

void async_await_test()
{
    co_run []()
    {
        auto f2 = co_async std::bind(add, 19, 23);
        printf( "ret2:%d\n", f2 );

        auto f1 = co_async [](){return add(12,2);};
        printf( "ret1:%d\n", f1 );
    };

    co_run []()
    {
        AsyncTest test(2, 4); 
        auto f2 = co_async std::bind(&AsyncTest::handle, &test, 3);
        printf( "ret3:%d\n", f2 );
    }; 
    rt::co_loop_until_no_tasks();  
    printf("process exit\n");
} 

int main( int argc, char** argv )
{ 
    if ( argc < 2 )
    {
        perror( "parameter error\n" );
        exit( 0 );
    }

    log::set_sys_rotate_cfg( "log" );
    rt::CoRoutine::init();

    RT_ERROR << "test log";
    switch ( atoi( argv[1] ) )
    {
    case 0:
        test_default();
        break;
    case 1: 
        test_sleep();
        break;
    case 2:
        test_condition();
        break;
    case 3:
        test_channel();
        break;
    case 4:
        test_channel_old();
        break;
    case 5:
        test_channel_new();
        break;
    case 6:
        test_channel_tm();
        break;
    case 7:
        test_co_task();
        break;
    case 8:
        test_co_timer();
        break;
    case 9:
        async_await_test();
        break;
    default:
        break;
    }
    return 0;
}
