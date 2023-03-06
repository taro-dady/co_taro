
#include "base/error_no.h"
#include "co_routine/co_timer.h"
#include "co_routine/impl/common.h"
#include "co_routine/impl/co_scheduler.h"

NAMESPACE_TARO_RT_BEGIN

struct CoTimer::Impl
{
    Impl()
        : once( false )
        , stop( false )
        , period( -1 )
        , delay( -1 )
    {

    }

    void invoke()
    {
        callback();
        if ( !once && !stop )
        {
            get_scheduler()->add_event( std::bind( &CoTimer::Impl::invoke, this ), period );
        }
    }

    bool                  once;
    bool                  stop;
    int32_t               period;
    int32_t               delay;
    std::function<void()> callback;
};

CoTimer::CoTimer()
    : impl_( new Impl )
{

}

CoTimer::~CoTimer()
{
    delete impl_;
}

bool CoTimer::start( int32_t delay, int32_t period, std::function<void()> const& callback )
{
    if ( !callback || delay < 0 || period <= 0 )
    {
        set_errno( TARO_ERR_INVALID_ARG );
        return false;
    }

    impl_->once     = false;
    impl_->delay    = delay;
    impl_->period   = period;
    impl_->callback = callback;

    if ( impl_->delay > 0 )
    {
        get_scheduler()->add_event( std::bind( &CoTimer::Impl::invoke, impl_ ), impl_->delay );
    }
    else
    {
        get_scheduler()->add_event( std::bind( &CoTimer::Impl::invoke, impl_ ), impl_->period );
    }
    return true;
}

bool CoTimer::once( int32_t delay, std::function<void()> const& callback )
{
    if ( !callback || delay <= 0 )
    {
        set_errno( TARO_ERR_INVALID_ARG );
        return false;
    }

    impl_->once     = true;
    impl_->delay    = delay;
    impl_->period   = -1;
    impl_->callback = callback;

    get_scheduler()->add_event( std::bind( &CoTimer::Impl::invoke, impl_ ), impl_->delay );
    return true;
}

void CoTimer::stop()
{
    impl_->stop = true;
}

NAMESPACE_TARO_RT_END
