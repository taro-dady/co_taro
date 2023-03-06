
#include "log/log_sys.h"
#include "log/log_writer.h"
#include "base/system/time.h"
#include "base/utils/assert.h"
#include "base/system/thread.h"
#include "base/utils/arg_expander.h"

NAMESPACE_TARO_LOG_BEGIN

struct LogWriter::Impl
{
    ELogLevel         level_;
    std::string       module_;
    std::stringstream os_;
};

LogWriter::LogWriter( const char* m, ELogLevel const& level, const char* file, const char* function, int32_t line )
    : impl_( new Impl )
{
    TARO_ASSERT( STRING_CHECK( m ) );

    impl_->module_ = m;
    impl_->level_  = level;

    static const char* ls[] = { "DEBUG", "TRACE", "WARNING", "ERROR", "FATAL" };
    impl_->os_  << SystemTime().to_ms_str() 
                << " [" << m << "] " 
                << ls[level] << " " 
                << file << ":" << line << " "
                << function << " " 
                << Thread::current_tid() << " ";
}

LogWriter::~LogWriter()
{
    impl_->os_ << std::endl;
    LogSystem::instance().write( impl_->module_.c_str(), 
                                 impl_->level_, 
                                 impl_->os_.str().c_str() );
}

void* LogWriter::get_stream()
{
    return ( void* )&impl_->os_;
}

NAMESPACE_TARO_LOG_END
