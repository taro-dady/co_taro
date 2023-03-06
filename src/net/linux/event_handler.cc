#include "co_routine/impl/common.h"
#include "co_routine/impl/co_scheduler.h"
#include "net/impl/linux/epoller.h"

NAMESPACE_TARO_NET_BEGIN

thread_local MultiplexerSPtr thrd_poller;

// epoller 协程事件轮询器
class EpollerEventPoller : PUBLIC rt::EventPoller
{
PUBLIC:

    /**
    * @brief 构造函数
    * 
    * @param[in] poller 事件轮询器
    */
    EpollerEventPoller( MultiplexerSPtr poller )
        : poller_( poller )
    {
        
    }

    /**
    * @brief 等待指定时间
    * 
    * @param[in] ms 等待时间(毫秒)
    */
    virtual void wait( int32_t ms ) override
    {
        poller_->poll( ms );
    }

    /**
    * @brief 唤醒
    */
    virtual void wakeup() override
    {
        poller_->wakeup();
    }

PRIVATE:

    MultiplexerSPtr poller_;
};

MultiplexerSPtr get_poller()
{
    if ( nullptr == thrd_poller )
    {
        thrd_poller = std::make_shared<Epoller>();
        rt::get_scheduler()->set_evt_poller( std::make_shared<EpollerEventPoller>( thrd_poller ) );
    }
    return thrd_poller;
}

CoNetEvent::CoNetEvent( int32_t fd, ENetEventType type )
    : fd_( fd )
    , evt_id_( -1 )
    , type_( eNetEvtInvalid )
{
    get_poller()->add_event( fd, this, ( int32_t )type );
}

CoNetEvent::~CoNetEvent()
{
    get_poller()->remove_event( fd_ );
}

void CoNetEvent::wait()
{
    co_ = rt::CoRoutine::current();
    co_->yield();
}

void CoNetEvent::wait( ENetEventType type, uint32_t ms )
{
    co_ = rt::CoRoutine::current();
    get_poller()->modify_event( fd_, this, type );
    if ( ms > 0 )
    {
        auto timeout_func = [this]()
        {
            // 事件等待超时处理
            evt_id_ = -1;
            type_   = eNetEvtTimeout;
            rt::get_scheduler()->push( co_ );
            co_ = nullptr;
        };
        evt_id_ = rt::get_scheduler()->add_event( timeout_func, ms );
    }
    co_->yield();
    
    get_poller()->modify_event( fd_, 0, 0 );
}

ENetEventType CoNetEvent::evt_type() const
{
    return type_;
}

void CoNetEvent::on_event( ENetEventType type )
{
    if ( evt_id_ >= 0 )
    {
        // 事件在超时之前发生则删除超时处理函数
        rt::get_scheduler()->remove_event( evt_id_ );
        evt_id_ = -1;
    }
    
    type_ = type;
    if ( co_ )
    {
        rt::get_scheduler()->push( co_ );
        co_ = nullptr;
    }
}

NAMESPACE_TARO_NET_END
