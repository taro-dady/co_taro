
#include "log/sink.h"

NAMESPACE_TARO_LOG_BEGIN

struct Sink::Impl
{
    bool enabled_;
    bool async_mode_;
    std::string name_;
};

Sink::Sink( const char* name )
    : impl_( new Impl )
{
    impl_->name_       = name;
    impl_->enabled_    = true;
    impl_->async_mode_ = false;
}

Sink::~Sink()
{
    delete impl_;
}

const char* Sink::name() const
{
    return impl_->name_.c_str();
}

bool Sink::enabled() const
{
    return impl_->enabled_;
}

void Sink::set_enable( bool enable )
{
    impl_->enabled_ = enable;
}

bool Sink::use_async_mode() const
{
    return impl_->async_mode_;
}

void Sink::set_async_mode( bool enable )
{
    impl_->async_mode_ = enable;
}

NAMESPACE_TARO_LOG_END
