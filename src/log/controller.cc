
#include "log/controller.h"
#include "base/utils/assert.h"
#include "base/system/spin_locker.h"
#include "base/utils/arg_expander.h"
#include <map>
#include <mutex>
#include <functional>

NAMESPACE_TARO_LOG_BEGIN

extern std::function<void( SinkSPtr, std::shared_ptr<std::string>, ELogLevel )> async_output;

struct Controller::Impl
{
    bool        enable_;
    SpinLocker  mtx_;
    ELogLevel   lvl_;
    std::string name_;
    std::map<std::string, SinkSPtr> sinks_;
};

Controller::Controller( const char* name )
    : impl_( new Impl )
{
    impl_->name_   = name;
    impl_->enable_ = true;
    impl_->lvl_    = eLogLevelWarn;
}

Controller::~Controller()
{
    delete impl_;
}

const char* Controller::name() const
{
    std::lock_guard<SpinLocker> g( impl_->mtx_ );
    return impl_->name_.c_str();
}

bool Controller::enabled()
{
    std::lock_guard<SpinLocker> g( impl_->mtx_ );
    return impl_->enable_;
}

void Controller::set_enable( bool enable )
{
    std::lock_guard<SpinLocker> g( impl_->mtx_ );
    impl_->enable_ = enable;
}

ELogLevel Controller::level()
{
    std::lock_guard<SpinLocker> g( impl_->mtx_ );
    return impl_->lvl_;
}

void Controller::set_level( ELogLevel level )
{
    std::lock_guard<SpinLocker> g( impl_->mtx_ );
    impl_->lvl_ = level;
}

void Controller::write( const char* message, ELogLevel const& lvl )
{
    TARO_ASSERT( STRING_CHECK( message ) );
    auto msg = std::make_shared<std::string>( message );

    std::lock_guard<SpinLocker> g( impl_->mtx_ );
    for ( auto& one : impl_->sinks_ )
    {
        auto sink_ptr = one.second;
        if ( !sink_ptr->enabled() ) 
        {
            continue;
        }
        
        if ( !sink_ptr->use_async_mode() )
        {
            sink_ptr->write( message, lvl );
        }
        else
        {
            TARO_ASSERT( async_output );
            async_output( sink_ptr, msg, lvl );
        }
    }
}

bool Controller::add_sink( SinkSPtr const& sink )
{
    if ( sink == nullptr || !STRING_CHECK( sink->name() ) )
    {
        return false;
    }
    std::lock_guard<SpinLocker> g( impl_->mtx_ );
    return impl_->sinks_.insert( std::make_pair( sink->name(), sink ) ).second;
}

bool Controller::remove_sink( const char* name )
{
    if ( !STRING_CHECK( name ) )
    {
        return false;
    }

    std::lock_guard<SpinLocker> g( impl_->mtx_ );
    auto it = impl_->sinks_.find( name );
    if ( it == impl_->sinks_.end() )
    {
        return false;
    }
    impl_->sinks_.erase( it );
    return true;
}

NAMESPACE_TARO_LOG_END
