
#include "base/error_no.h"
#include "base/utils/defer.h"
#include "base/utils/arg_expander.h"
#include "base/system/share_memory.h"
#include <windows.h>

NAMESPACE_TARO_BEGIN

struct ShmMemory::Impl
{
	Impl()
		: handle_( INVALID_HANDLE_VALUE )
		, base_( NULL )
	{

	}

	HANDLE handle_;
	LPVOID base_;
};

ShmMemory::ShmMemory()
	: impl_( new Impl )
{ 
	
}

ShmMemory::ShmMemory( ShmMemory&& other )
	: impl_( std::move( other.impl_ ) )
{
	other.impl_ = nullptr;
}

ShmMemory::~ShmMemory()
{
	if ( impl_ == nullptr )
	{
		return;
	}

	if ( impl_->base_ != NULL )
	{
		UnmapViewOfFile( impl_->base_ );
		impl_->base_ = NULL;
	}

	if ( impl_->handle_ != INVALID_HANDLE_VALUE )
	{
		CloseHandle( impl_->handle_ );
		impl_->handle_ = INVALID_HANDLE_VALUE;
	}

	delete impl_;
	impl_ = nullptr;
}

int32_t ShmMemory::create( const char* name, uint64_t bytes )
{
	if ( !STRING_CHECK( name ) || bytes == 0 )
	{
		return TARO_ERR_INVALID_ARG;
	}

	// 创建共享文件句柄 
	HANDLE hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,   // 物理文件句柄
		NULL,                   // 默认安全级别
		PAGE_READWRITE,         // 可读可写
		0,                      // 高位文件大小
		( DWORD )bytes,         // 低位文件大小
		name                    // 共享内存名称
	);

	if ( hMapFile == INVALID_HANDLE_VALUE )
	{
		return TARO_ERR_FILE_OP;
	}

	// 映射缓存区视图 , 得到指向文件映射视图的指针
	LPVOID lpBase = MapViewOfFile(
		hMapFile,            // 共享内存的句柄
		FILE_MAP_ALL_ACCESS, // 可读写许可
		0,
		0,
		bytes
	);

	if ( lpBase == NULL )
	{
		CloseHandle( hMapFile );
		return TARO_ERR_FAILED;
	}

	impl_->handle_ = hMapFile;
	impl_->base_   = lpBase;
	return TARO_OK;
}

int32_t ShmMemory::open( const char* name )
{
	if ( !STRING_CHECK( name ) )
	{
		return TARO_ERR_INVALID_ARG;
	}

	HANDLE hMapFile = OpenFileMapping( FILE_MAP_ALL_ACCESS, NULL, name );
	if ( hMapFile == INVALID_HANDLE_VALUE )
	{
		return TARO_ERR_FILE_OP;
	}

	LPVOID lpBase = MapViewOfFile( hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
	if ( lpBase == NULL )
	{
		CloseHandle( hMapFile );
		return TARO_ERR_FAILED;
	}

	impl_->handle_ = hMapFile;
	impl_->base_   = lpBase;
	return TARO_OK;
}

void* ShmMemory::get() const
{
	return impl_->base_;
}

NAMESPACE_TARO_END
