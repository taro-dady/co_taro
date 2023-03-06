
#include "base/error_no.h"
#include "base/utils/assert.h"
#include "base/utils/arg_expander.h"
#include "base/system/file_locker.h"
#include <windows.h>

NAMESPACE_TARO_BEGIN

struct FileLocker::Impl
{
    HANDLE fd_;
};

FileLocker::FileLocker()
    : impl_( new Impl )
{
    impl_->fd_ = INVALID_HANDLE_VALUE;
}

FileLocker::FileLocker( FileLocker&& other )
    : impl_( other.impl_ )
{
    other.impl_ = nullptr;
}

FileLocker::~FileLocker()
{
    if ( impl_ != nullptr )
    {
        unlock();
        CloseHandle( impl_->fd_ );
        impl_->fd_ = INVALID_HANDLE_VALUE;
        delete impl_;
        impl_ = nullptr;
    }
}

int32_t FileLocker::init( const char* path )
{
    if ( !STRING_CHECK( path ) )
    {
        return TARO_ERR_INVALID_ARG;
    }

    if ( INVALID_HANDLE_VALUE != impl_->fd_ )
    {
        return TARO_ERR_MULTI_OP;
    }

    impl_->fd_ = CreateFile(
        path,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if( impl_->fd_ == INVALID_HANDLE_VALUE )
    {
        return TARO_ERR_FAILED;
    }
    return TARO_OK;
}

void FileLocker::write_lock( size_t start, size_t end )
{
    TARO_ASSERT( end >= start, "start", start, "end", end );
    TARO_ASSERT( impl_->fd_ != INVALID_HANDLE_VALUE );

    OVERLAPPED ovlp;
    memset( &ovlp, 0, sizeof( ovlp ) );
    ovlp.Offset = ( DWORD )start;
    LockFileEx( impl_->fd_, LOCKFILE_EXCLUSIVE_LOCK, 0, ( DWORD )( end - start ), 0, &ovlp );
}

bool FileLocker::try_write_lock( size_t start, size_t end )
{
    TARO_ASSERT( end >= start, "start", start, "end", end );
    TARO_ASSERT( impl_->fd_ != INVALID_HANDLE_VALUE );

    OVERLAPPED ovlp;
    memset( &ovlp, 0, sizeof( ovlp ) );
    ovlp.Offset = ( DWORD )start;
    return LockFileEx( impl_->fd_, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, ( DWORD )( end - start ), 0, &ovlp );
}

void FileLocker::read_lock( size_t start, size_t end )
{
    TARO_ASSERT( end >= start, "start", start, "end", end );
    TARO_ASSERT( impl_->fd_ != INVALID_HANDLE_VALUE );

    OVERLAPPED ovlp;
    memset( &ovlp, 0, sizeof( ovlp ) );
    ovlp.Offset = ( DWORD )start;
    LockFileEx( impl_->fd_, 0, 0, ( DWORD )( end - start ), 0, &ovlp );
}

bool FileLocker::try_read_lock( size_t start, size_t end )
{
    TARO_ASSERT( end >= start, "start", start, "end", end );
    TARO_ASSERT( impl_->fd_ != INVALID_HANDLE_VALUE );

    OVERLAPPED ovlp;
    memset( &ovlp, 0, sizeof ovlp );
    ovlp.Offset = ( DWORD )start;
    return LockFileEx( impl_->fd_, LOCKFILE_FAIL_IMMEDIATELY, 0, ( DWORD )( end - start ), 0, &ovlp );
}

void FileLocker::unlock( size_t start, size_t end )
{
    TARO_ASSERT( end >= start, "start", start, "end", end );
    TARO_ASSERT( impl_->fd_ != INVALID_HANDLE_VALUE );

    OVERLAPPED ovlp;
    memset( &ovlp, 0, sizeof( ovlp ) );
    ovlp.Offset = ( DWORD )start;
    UnlockFileEx( impl_->fd_, 0, ( DWORD )( end - start ), 0, &ovlp );
}

NAMESPACE_TARO_END
