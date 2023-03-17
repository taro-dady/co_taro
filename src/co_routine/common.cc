
#include "co_routine/co_channel.h"
#include "co_routine/co_wrapper.h"
#include "co_routine/impl/common.h"
#include "co_routine/impl/default_scheduler.h"

USING_NAMESPACE_TARO_LOG

NAMESPACE_TARO_RT_BEGIN

struct LogItem
{
    SinkSPtr sink;
    std::shared_ptr<std::string> msg;
    ELogLevel lvl;
};
using LogItemSPtr = std::shared_ptr<LogItem>;

static std::atomic_bool co_async_running( false );
thread_local int64_t current_tid = -1;
thread_local CoSchedulerSPtr co_sched = std::make_shared<DefaultScheduler>();

int64_t get_tid()
{
    if ( current_tid < 0 )
    {
        current_tid = Thread::current_tid();
    }
    return current_tid;
}

CoSchedulerSPtr get_scheduler()
{
    return co_sched;
}

void set_scheduler( CoSchedulerSPtr const& sched )
{
    co_sched = sched;
}

extern TARO_DLL_EXPORT void co_wait( uint32_t ms )
{
    auto co = CoRoutine::current();
    if ( ms == 0 )
    {
        co_sched->push( co );
    }
    else
    {
        // 当前协程被调度出去后，会被调度器清除，故需要保存在function中
        // 等待evtpoller触发
        std::weak_ptr< CoScheduler > sched( co_sched );
        ( void )co_sched->add_event( [sched, co]()
        {
            auto sc_sp = sched.lock();
            if ( sc_sp ) sc_sp->push( co );
        }, ms );
    }
    co->yield();
}

extern TARO_DLL_EXPORT void co_loop()
{
    co_sched->run();
    CoTaskContainer::instance().join();
}

extern TARO_DLL_EXPORT void co_loop_until_no_tasks()
{
    co_sched->run_until_no_task();
    CoTaskContainer::instance().join();
}

void sched_co( CoRoutineSPtr co )
{
    co_sched->push( co );
}

void init_async_log()
{
    auto chn  = std::make_shared< Channel<LogItemSPtr> >( 128 );
    auto schd = get_scheduler(); // 获取初始化线程即主线程的调度器
    auto func = [chn, schd]( SinkSPtr sink, std::shared_ptr<std::string> msg, ELogLevel lvl )
    {
        TARO_ASSERT( sink && msg );

        bool expr = false;
        if ( co_async_running.compare_exchange_strong( expr, true ) )
        {
            // 异步日志协程主线程进行写日志操作
            auto func = [chn]()
            {
                while( 1 )
                {
                    LogItemSPtr item;
                    chn->read( item );
                    if ( nullptr == item )
                        continue;
                    item->sink->write( item->msg->c_str(), item->lvl );
                }
            };
            schd->push( CoRoutine::create( "async_logger", func, false ) );
        }
        
        auto item  = std::make_shared<LogItem>();
        item->sink = sink;
        item->msg  = msg;
        item->lvl  = lvl;
        chn->write( item );
    };
    init_async_logger( func );
}

NAMESPACE_TARO_RT_END
