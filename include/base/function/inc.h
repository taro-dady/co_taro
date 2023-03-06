
#pragma once

#include "base/function/function.h"
#include "base/function/function_traits.h"
#include "base/function/tuple_function.h"
#include "base/function/observers.h"

NAMESPACE_TARO_BEGIN

template<typename F>
typename std::enable_if< !IsClassFunction< F >::value && !IsObjectFunction<F>::value, Function< typename FunctionTraits<F>::FunctionType > >::type
make_func( F const& f )
{
    return Function< typename FunctionTraits<F>::FunctionType >( f );
}

template<typename F, typename Class>
typename std::enable_if< std::is_member_function_pointer< F >::value, Function< typename FunctionTraits<F>::FunctionType > >::type
make_func( F const& f, Class const& obj )
{
    return Function< typename FunctionTraits<F>::FunctionType>( f, obj );
}

template<typename F>
typename std::enable_if< IsObjectFunction<F>::value, Function< typename FunctionTraits<F>::FunctionType > >::type
make_func( F const& f )
{
    return Function< typename FunctionTraits<F>::FunctionType >( f );
}

template<class T>
class FunctionWrapper {};

template<typename Ret, typename... Args>
struct FunctionWrapper<Ret(Args...)>
{
PUBLIC: // function

    FunctionWrapper( std::function<Ret( Args... )> const& func )
        : func_( func )
    {

    }

    Ret handle( Args... args )
    {
        return func_( args... );
    }

PRIVATE: // variable

    std::function<Ret( Args... )> func_;
};

template<typename F>
inline Function< typename FunctionTraits<F>::FunctionType > transfer_func( F const& func )
{
    auto ptr = std::make_shared< FunctionWrapper<typename FunctionTraits<F>::FunctionType> >( func );
    return make_func( &FunctionWrapper<typename FunctionTraits<F>::FunctionType>::handle, ptr );
}

NAMESPACE_TARO_END
