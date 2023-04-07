
#include "base/system/time.h"
#include "base/utils/assert.h"
#include <string>
#include <chrono>
#include <ctime>

NAMESPACE_TARO_BEGIN

struct SteadyTime::Impl
{
    std::chrono::steady_clock::time_point timepoint_;
};

struct SystemTime::Impl
{
    std::chrono::system_clock::time_point timepoint_;
};

SteadyTime::SteadyTime()
    : impl_( new Impl )
{
    impl_->timepoint_ = std::chrono::steady_clock::now();
}

SteadyTime::~SteadyTime()
{
    delete impl_;
}

int64_t SteadyTime::cost()
{
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast< std::chrono::milliseconds >( now - impl_->timepoint_ );
    return ( int64_t )duration.count();
}

int64_t SteadyTime::current_ms()
{
    auto duration = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now().time_since_epoch() );
    return ( int64_t )duration.count();
}

SystemTime::SystemTime()
    : impl_( new Impl )
{
    impl_->timepoint_ = std::chrono::system_clock::now();
}

SystemTime::SystemTime( int64_t const& t )
    : impl_( new Impl )
{
    impl_->timepoint_ = std::chrono::system_clock::from_time_t( t );
}

SystemTime::~SystemTime()
{
    if ( nullptr != impl_ )
    {
        delete impl_;
        impl_ = nullptr;
    } 
}

int64_t SystemTime::current_ms()
{
    auto duration = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::system_clock::now().time_since_epoch() );
    return ( time_t )duration.count();
}

int64_t SystemTime::current_sec()
{
    auto duration = std::chrono::duration_cast< std::chrono::seconds >( std::chrono::system_clock::now().time_since_epoch() );
    return ( time_t )duration.count();
}

std::string SystemTime::to_sec_str( const char* fmt )
{
    TARO_ASSERT( STRING_CHECK( fmt ), fmt );

    auto as_time_t = std::chrono::system_clock::to_time_t( impl_->timepoint_ );
    struct tm tm;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s( &tm, &as_time_t );  //win api，线程安全，而std::localtime线程不安全
#else
    localtime_r( &as_time_t, &tm );//linux api，线程安全
#endif
    char buf[128] = { 0 };
    snprintf( buf, sizeof( buf ), fmt, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec );
    return buf;
}

std::string SystemTime::to_ms_str( const char* fmt )
{
    TARO_ASSERT( STRING_CHECK( fmt ), fmt );

    auto as_time_t = std::chrono::system_clock::to_time_t( impl_->timepoint_ );
    struct tm tm;

#if defined(_WIN32) || defined(_WIN64)
    localtime_s( &tm, &as_time_t );
#else
    localtime_r( &as_time_t, &tm );
#endif
    std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >( impl_->timepoint_.time_since_epoch() );
    char buf[128] = { 0 };
    snprintf( buf, sizeof( buf ), fmt, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, ms.count() % 1000 );
    return buf;
}

NAMESPACE_TARO_END
