
#pragma once

#include "base/base.h"
#include <tuple>

NAMESPACE_TARO_BEGIN

template<class T>
struct RetType {};

template<class T, int32_t N = std::tuple_size<T>::value >
struct TupleFunctor
{
    template<typename R, class F, class ...Args>
    static R call( RetType<R>, F&& f, T&& t, Args&&... args )
    {
        return TupleFunctor<T, N - 1>::call( RetType<R>(), 
                                             std::forward<F>( f ), 
                                             std::forward<T>( t ),
                                             std::forward<typename std::tuple_element<N - 1, T>::type>( std::get<N - 1>( t ) ),
                                             std::forward<Args>(args)... );
    }

    template<class F, class ...Args>
    static void call( RetType<void>, F&& f, T&& t, Args&&... args )
    {
        TupleFunctor<T, N - 1>::call( RetType<void>(),
                                      std::forward<F>( f ),
                                      std::forward<T>( t ),
                                      std::forward<typename std::tuple_element<N - 1, T>::type>( std::get<N - 1>( t ) ),
                                      std::forward<Args>( args )... );
    }
};

template<class T >
struct TupleFunctor<T, 0>
{
    template<typename R, class F, class ...Args>
    static R call( RetType<R>, F&& f, T&&, Args&&... args )
    {
        return f( args... );
    }

    template<class F, class ...Args>
    static void call( RetType<void>, F&& f, T&&, Args&&... args )
    {  
        f( args... );
    }
};

NAMESPACE_TARO_END
