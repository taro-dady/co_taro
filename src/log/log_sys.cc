
#include "log/log_sys.h"
#include "base/error_no.h"
#include "log/sink_print.h"
#include "base/system/spin_locker.h"
#include "base/utils/arg_expander.h"
#include "base/memory/details/destroyer.h"
#include <map>
#include <mutex>

NAMESPACE_TARO_LOG_BEGIN

struct LogSystemImpl
{
    static Controller& get_global()
    {
        return *LogSystem::instance().impl_->global_;
    }

    SpinLocker mtx_;
    ControllerSPtr global_;
    std::map<std::string, ControllerSPtr> ctrl_;
};

extern Controller& get_global()
{
    return LogSystemImpl::get_global();
}

LogSystem& LogSystem::instance()
{
    static LogSystem inst;
    return inst;
}

LogSystem::LogSystem()
    : impl_( new LogSystemImpl )
{
    impl_->global_ = std::make_shared< Controller >( "Global" );
    ( void )impl_->global_->add_sink( std::make_shared<PrintSink>( "PrintSinkGlobal" ) );
}

LogSystem::~LogSystem()
{
    delete impl_;
}

int32_t LogSystem::add_ctrl( ControllerSPtr ctrl )
{
    if ( nullptr == ctrl || !STRING_CHECK( ctrl->name() ) )
    {
        return TARO_ERR_INVALID_ARG;
    }

    std::lock_guard<SpinLocker> g( impl_->mtx_ );
    return impl_->ctrl_.insert( std::make_pair( ctrl->name(), ctrl ) ).second 
            ? TARO_OK : TARO_ERR_FAILED;
}

int32_t LogSystem::remove_ctrl( const char* name )
{
    if ( !STRING_CHECK( name ) )
    {
        return TARO_ERR_INVALID_ARG;
    }

    std::lock_guard<SpinLocker> g( impl_->mtx_ );
    auto it = impl_->ctrl_.find( name );
    if ( it == impl_->ctrl_.end() )
    {
        return TARO_ERR_FAILED;
    }
    impl_->ctrl_.erase( it );
    return TARO_OK;
}

ControllerSPtr LogSystem::find_ctrl( const char* name )
{
    if ( !STRING_CHECK( name ) )
    {
        return nullptr;
    }

    std::lock_guard<SpinLocker> g( impl_->mtx_ );
    auto it = impl_->ctrl_.find( name );
    if ( it != impl_->ctrl_.end() )
    {
        return it->second;
    }
    return nullptr;
}

void LogSystem::write( const char* module_name, ELogLevel level, const char* message )
{
    if ( !STRING_CHECK( module_name ) )
    {
        return;
    }

    if( !impl_->global_->enabled() || impl_->global_->level() > level )
    {
        return;
    }

    ControllerSPtr controller = impl_->global_;
    {
        std::lock_guard<SpinLocker> g( impl_->mtx_ );
        auto it = impl_->ctrl_.find( module_name );
        if ( it != impl_->ctrl_.end() )
        {
            controller = it->second;
        }
    }
    
    if ( ( controller != impl_->global_ ) && ( !controller->enabled() || ( controller->level() > level ) ) )
    {
        return;
    }
    controller->write( message, level );
}

NAMESPACE_TARO_LOG_END
