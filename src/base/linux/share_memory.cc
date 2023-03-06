
#include "base/error_no.h"
#include "base/utils/defer.h"
#include "base/utils/arg_expander.h"
#include "base/system/share_memory.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

NAMESPACE_TARO_BEGIN

struct ShmMemory::Impl
{
	Impl()
		: base_( NULL )
        , bytes_( 0 )
	{

	}

    void*       base_;
    uint64_t    bytes_;
    std::string name_;
};

ShmMemory::ShmMemory()
    : impl_( new Impl )
{

}

ShmMemory::ShmMemory( ShmMemory&& other )
    : impl_( other.impl_ )
{
    other.impl_ = nullptr;
}

ShmMemory::~ShmMemory()
{
    if( nullptr == impl_ )
    {
        return;
    }

    if ( impl_->base_ )
    {
        munmap( impl_->base_, impl_->bytes_ );
        shm_unlink( impl_->name_.c_str() );
    }
    delete impl_;
}

int32_t ShmMemory::create( const char* name, uint64_t bytes )
{
    if ( !STRING_CHECK( name ) || bytes == 0 )
	{
		return TARO_ERR_INVALID_ARG;
	}

    int32_t fd = -1;
    void* base = MAP_FAILED;

    fd = ::shm_open( name, O_RDWR | O_CREAT, 0644 );
    if ( fd < 0 )
    {
        printf( "open failed. name %s %d\n", name, errno );
        return TARO_ERR_FILE_OP;
    }

    Defer defer( [&]()
    {
        if ( base != MAP_FAILED ) munmap( base, bytes );
        if ( fd >= 0 ) ::close( fd );
        shm_unlink( name );
    } );

    if ( ::ftruncate( fd, bytes ) < 0) 
    {
        printf( "ftruncate failed. name %s %d\n", name, errno );
        return TARO_ERR_FILE_OP;
    }

    base = ::mmap( nullptr, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
    if ( base == MAP_FAILED ) 
    {
        printf( "mmap failed. name %s %d\n", name, errno );
        return TARO_ERR_FAILED;
    }

    ::close( fd );
    defer         = false;
    impl_->base_  = base;
    impl_->name_  = name;
    impl_->bytes_ = bytes;
    return TARO_OK;
}

int32_t ShmMemory::open( const char* name )
{
    if ( !STRING_CHECK( name ) )
	{
		return TARO_ERR_INVALID_ARG;
	}

    int fd = ::shm_open( name, O_RDWR, 0644 );
    if ( fd < 0 ) 
    {
        printf( "shm open failed. name %s %d\n", name, errno );
        return TARO_ERR_FILE_OP;
    }

    struct stat file_attr;
    if ( ::fstat( fd, &file_attr ) < 0 ) 
    {
        printf("shm open get file state failed.\n");
        close(fd);
        return TARO_ERR_FILE_OP;
    }

    auto base = ::mmap( nullptr, file_attr.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
    if ( base == MAP_FAILED ) 
    {
        printf("shm open mmap failed. %s %d %d\n", name, errno, (int32_t)file_attr.st_size );
        close(fd);
        return TARO_ERR_FAILED;
    }

    ::close( fd );
    impl_->base_  = base;
    impl_->name_  = name;
    impl_->bytes_ = file_attr.st_size;
    return TARO_OK;
}

void* ShmMemory::get() const
{
    return impl_->base_;
}

NAMESPACE_TARO_END
