
#include "base/system/process.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

NAMESPACE_TARO_BEGIN

struct Process::Impl
{
    pid_t pid_;
};

Process::Process()
    : impl_( new Impl )
{
    impl_->pid_ = -1;
} 

Process::Process( Process&& other )
    : impl_( other.impl_ )
{
    other.impl_ = nullptr;
}

Process::~Process()
{
    if( impl_ != nullptr )
        delete impl_;
}

int32_t Process::cur_pid()
{
    return getpid();
}

int32_t Process::pid() const
{
    return impl_->pid_;
}

bool Process::kill()
{
    if ( impl_->pid_ < 0 )
    {
        printf( "pid invalid.\n" );
        return false;
    }

    ::kill( impl_->pid_, SIGKILL );
    int32_t stat;
    ::wait( &stat );
    impl_->pid_ = -1;
    return true;
}

bool Process::create( const char* entry, char* const* cmd )
{
    pid_t p;
    p = fork();
    if ( 0 == p )
    {
        execv( entry, cmd );
    }
    else if( p > 0 )
    {
        impl_->pid_ = p;
    }
    else
    {
        printf( "create process failed.\n" );
        return false;
    }
    return true;
}

NAMESPACE_TARO_END
