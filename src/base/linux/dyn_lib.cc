
#include "base/error_no.h"
#include "base/system/dyn_lib.h"
#include "base/utils/arg_expander.h"
#include <dlfcn.h>

NAMESPACE_TARO_BEGIN

struct DynLib::Impl
{
    Impl()
        :handler_( nullptr )
    {}

    void* handler_;
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
    if ( nullptr == impl_ )
    {
        return;
    }
    
    unload();
    delete impl_;
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

    impl_->handler_ = dlopen( path, RTLD_NOW | RTLD_GLOBAL );
    if( impl_->handler_ == nullptr )
    {
        printf("open file failed. %s %s\n", dlerror(), path);
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

    if ( 0 == dlclose( impl_->handler_ ) )
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
    
    if ( impl_->handler_ == nullptr )
    {
        set_errno( TARO_ERR_INVALID_RES );
        return nullptr;
    }
    return ( void * )dlsym( impl_->handler_, func_name );
}

NAMESPACE_TARO_END
