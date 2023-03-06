
#include "net/impl/linux/epoller.h"
#include <unistd.h>
#include <string.h>
#include <sys/eventfd.h>

NAMESPACE_TARO_NET_BEGIN

Epoller::Epoller()
    : num_( 0 )
    , epoll_fd_( epoll_create1( 0 ) )
{
    TARO_ASSERT( epoll_fd_ >= 0, "epoll create failed" );
    init_event_fd();
}

Epoller::~Epoller()
{
    ::close( epoll_fd_ );
    epoll_fd_ = -1;

    if ( wakeup_fd_ > 0 )
    {
        ::close( wakeup_fd_ );
        wakeup_fd_ = -1;
    }
    num_ = 0;
}

void Epoller::init_event_fd()
{
    wakeup_fd_ = ::eventfd( 0, 0 );
    if( wakeup_fd_ < 0 )
    {
        NET_ERROR << "create event fd failed.";
        return;
    }

    struct epoll_event ev;
    memset( &ev, 0, sizeof( ev ) );
    ev.data.fd = wakeup_fd_;
    ev.events  = EPOLLHUP | EPOLLERR | EPOLLIN;
    if( epoll_ctl( epoll_fd_, EPOLL_CTL_ADD, wakeup_fd_, &ev ) < 0 )
    {
        NET_ERROR << "add wakeup event fd failed.";
        return;
    }
    ++num_;
}

bool Epoller::add_event( int32_t fd, NetEventHandler* handler, int32_t opt )
{
    if( fd < 0 )
    {
        NET_ERROR << "parameter is invalid.";
        return false;
    }

    struct epoll_event ev;
    ::memset( &ev, 0, sizeof( ev ) );
    ev.data.ptr = handler;

    if( opt & eNetEvtRead )      ev.events |= EPOLLIN;
    if( opt & eNetEvtWrite )     ev.events |= EPOLLOUT;
    if( opt & eNetEvtException ) ev.events |= EPOLLERR;

    if( epoll_ctl( epoll_fd_, EPOLL_CTL_ADD, fd, &ev ) < 0 )
    {
        NET_ERROR << "add event failed. fd:" << fd;
        return false;
    }
    ++num_;
    return true;
}

bool Epoller::modify_event( int32_t fd, NetEventHandler* handler, int32_t opt )
{
    if( fd < 0 )
    {
        NET_ERROR << "parameter is invalid.";
        return false;
    }

    struct epoll_event ev;
    ::memset( &ev, 0, sizeof( ev ) );
    ev.data.ptr = handler;

    if( opt & eNetEvtRead )      ev.events |= EPOLLIN;
    if( opt & eNetEvtWrite )     ev.events |= EPOLLOUT;
    if( opt & eNetEvtException ) ev.events |= EPOLLERR;

    if( epoll_ctl( epoll_fd_, EPOLL_CTL_MOD, fd, &ev ) < 0 )
    {
        NET_ERROR << "modify event failed.";
        return false;
    }
    return true;
}

bool Epoller::remove_event( int32_t fd )
{
    if( fd < 0 )
    {
        NET_ERROR << "parameter is invalid.";
        return false;
    }

    if( epoll_ctl( epoll_fd_, EPOLL_CTL_DEL, fd, nullptr ) < 0 )
    {
        NET_TRACE << "delete event failed.";
        return false;
    }
    --num_;
    return true;
}

void Epoller::wakeup()
{
    auto ret = eventfd_write( wakeup_fd_, ( uint64_t )1 );
    if( ret < 0 )
    {
        NET_ERROR << "write event fd fail";
    }
}

void Epoller::poll( int32_t ms )
{
    events_.resize( num_ );
    auto event_ptr = events_.data();
    memset( event_ptr, 0, sizeof( struct epoll_event ) * num_ );

    int32_t res = epoll_wait( epoll_fd_, event_ptr, num_, ms );
    if( res <= 0 )
    {
        return;
    }

    for( int32_t i = 0; i < res; i++ )
    {
        if( event_ptr[i].data.fd == wakeup_fd_ )
        {
            eventfd_t val;
            ( void )eventfd_read( wakeup_fd_, &val );
            continue;
        }

        auto handler = static_cast< NetEventHandler* >( event_ptr[i].data.ptr );
        if( nullptr == handler )
        {
            continue;
        }

        ENetEventType evt = eNetEvtInvalid;
        if( event_ptr[i].events & ( EPOLLERR | EPOLLHUP ) )
        {
            evt = eNetEvtException;
        }
        else if( ( event_ptr[i].events & EPOLLIN ) != 0 )
        {
            evt = eNetEvtRead;
        }
        else if( ( event_ptr[i].events & EPOLLOUT ) != 0 )
        {
            evt = eNetEvtWrite;
        }

        if( evt != eNetEvtInvalid )
        {
            handler->on_event( evt );
        }
    }
}

NAMESPACE_TARO_NET_END
