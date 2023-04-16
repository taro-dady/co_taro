
#include "base/error_no.h"
#include "base/system/dyn_lib.h"
#include "base/utils/arg_expander.h"
#include "base/memory/details/destroyer.h"
#include <Winsock2.h>
#include <windows.h>

NAMESPACE_TARO_BEGIN

struct DynLib::Impl
{
    Impl()
        : handler_( nullptr )
    {}

    HMODULE handler_;
};

DynLib::DynLib()
    : impl_( new Impl )
{
    
}

DynLib::DynLib( DynLib&& other )
    : impl_( other.impl_ )
{
    other.impl_ = nullptr;
}

DynLib::~DynLib()
{
    if ( impl_ != 0 )
    {
        unload();
        delete impl_;
        impl_ = nullptr;
    }
}

int32_t DynLib::load( const char* path )
{
    if ( !STRING_CHECK( path ) )
    {
        return TARO_ERR_INVALID_ARG;
    }

    if ( impl_->handler_ != nullptr )
    {
        return TARO_ERR_MULTI_OP;
    }
    
    impl_->handler_ = LoadLibrary( path );;
    if( impl_->handler_ == nullptr )
    {
        printf( "error %d\n", GetLastError() );
        return TARO_ERR_FAILED;
    }
    return TARO_OK;
}

int32_t DynLib::unload()
{
    if ( nullptr == impl_->handler_ )
    {
        return TARO_ERR_MULTI_OP;
    }

    if ( FreeLibrary( impl_->handler_ ) )
    {
        impl_->handler_  = nullptr;
        return TARO_OK;
    }
    return TARO_ERR_FAILED;
}

void* DynLib::func_addr( const char* func_name )
{
    if ( !STRING_CHECK( func_name ) )
    {
        set_errno( TARO_ERR_INVALID_ARG );
        return nullptr;
    }

    if( nullptr == impl_->handler_ )
    {
        set_errno( TARO_ERR_INVALID_RES );
        return nullptr;
    }
    return ( void * )GetProcAddress( impl_->handler_, func_name );
}

NAMESPACE_TARO_END
