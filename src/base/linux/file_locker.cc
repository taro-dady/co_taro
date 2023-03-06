
#include "base/error_no.h"
#include "base/utils/assert.h"
#include "base/system/file_locker.h"
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>

NAMESPACE_TARO_BEGIN

struct FileLocker::Impl
{
    Impl() : fd_( -1 ) {}

    int32_t fd_;
};

FileLocker::FileLocker()
    : impl_( new Impl )
{

}

FileLocker::FileLocker( FileLocker&& other )
    : impl_( other.impl_ )
{
    other.impl_ = nullptr;
}

FileLocker::~FileLocker()
{
    if ( nullptr == impl_ )
    {
        return;
    }

    if ( impl_->fd_ >= 0 )
    {
        unlock();
        ::close( impl_->fd_ );
    }
    delete impl_;
}

int32_t FileLocker::init( const char* path )
{
    if ( !STRING_CHECK( path ) )
    {
        return TARO_ERR_INVALID_ARG;
    }

    if ( impl_->fd_ > 0 )
    {
        return TARO_ERR_MULTI_OP;
    }

    impl_->fd_ = ::open( path, O_RDWR | O_CREAT, 0644);
    if ( impl_->fd_ < 0 )
    {
        return TARO_ERR_FILE_OP;
    }

    int32_t flags = ::fcntl( impl_->fd_, F_GETFD );
    if ( flags == -1 )
    {
        return TARO_ERR_FILE_OP;
    }

    flags |= FD_CLOEXEC;
    int32_t ret = ::fcntl( impl_->fd_, F_SETFD, flags );
    return ( ret < 0 ) ? TARO_ERR_FAILED : TARO_OK;
}

void FileLocker::write_lock( size_t start, size_t end )
{
    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_start  = start;
    fl.l_len    = end - start;
    fl.l_type   = F_WRLCK;
    fl.l_pid    = -1;

    ( void )fcntl( impl_->fd_, F_SETLKW, &fl );
}

bool FileLocker::try_write_lock( size_t start, size_t end )
{
    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_start  = start;
    fl.l_len    = end - start;
    fl.l_type   = F_WRLCK;
    fl.l_pid    = -1;

    return 0 == fcntl( impl_->fd_, F_SETLK, &fl );
}

void FileLocker::read_lock( size_t start, size_t end )
{
    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_start  = start;
    fl.l_len    = end - start;
    fl.l_type   = F_RDLCK;
    fl.l_pid    = -1;
    ( void )fcntl( impl_->fd_, F_SETLKW, &fl );
}

bool FileLocker::try_read_lock( size_t start, size_t end )
{
    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_start  = start;
    fl.l_len    = end - start;
    fl.l_type   = F_RDLCK;
    fl.l_pid    = -1;
    
    return 0 == fcntl( impl_->fd_, F_SETLK, &fl );
}

void FileLocker::unlock( size_t start, size_t end )
{
    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_start  = start;
    fl.l_len    = end - start;
    fl.l_type   = F_UNLCK;
    fl.l_pid    = -1;

    ( void )fcntl( impl_->fd_, F_SETLKW, &fl );
}

NAMESPACE_TARO_END
