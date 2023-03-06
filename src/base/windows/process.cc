
#include "base/error_no.h"
#include "base/system/process.h"
#include "base/utils/string_tool.h"
#include <windows.h>
#include <process.h>

NAMESPACE_TARO_BEGIN

struct Process::Impl
{
    PROCESS_INFORMATION info_;
};

Process::Process()
    : impl_( new Impl )
{

}

Process::Process( Process&& other )
    : impl_( std::move( other.impl_ ) )
{
    other.impl_ = nullptr;
}

Process::~Process()
{
    if( nullptr != impl_ )
    {
        delete impl_;
        impl_ = nullptr;
    }
}

int32_t Process::cur_pid()
{
    return _getpid();
}

int32_t Process::pid() const
{
    return ( int32_t )GetProcessId( impl_->info_.hProcess );
}

bool Process::kill()
{
    TerminateProcess( impl_->info_.hProcess, 0 );
    WaitForSingleObject( impl_->info_.hProcess, INFINITE );
    CloseHandle( impl_->info_.hProcess );
    CloseHandle( impl_->info_.hThread );
    return true;
}

bool Process::create( const char* entry, char* const* argv )
{
    std::string cmd;
    size_t i = 0;
    while( argv[i] != nullptr )
    {
        cmd += argv[i];
        cmd += " ";
        ++i;
    }

    if ( !cmd.empty() )
    {
        cmd = string_trim_back( cmd );
    }

    STARTUPINFO info ={ sizeof( info ) };
    if( !CreateProcess( entry, ( cmd.empty() ? NULL : ( char* )cmd.c_str() ), NULL, NULL, TRUE, 0, NULL, NULL, &info, &impl_->info_ ) )
    {
        printf( "create process %s failed.\n", entry );
        return false;
    }
    return true;
}

NAMESPACE_TARO_END
