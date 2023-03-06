
#pragma once

#include <functional>
#include "base/function/function.h"
#include "base/serialize/type_traits.h"

NAMESPACE_TARO_BEGIN

template <typename T>
struct FunctionTraits;

template <typename R, typename... Args>
struct FunctionTraits<R( Args... )>
{
    using ResultType   = R;
    using FunctionType = R( Args... );
    using TupleType    = std::tuple<DecayType<Args>...>;

    enum { ArgNum = sizeof...( Args ) };

    template <size_t I>
    struct Arg
    {
        typedef typename std::tuple_element< I, std::tuple<Args...> >::type type;
    };
};

template <typename T>
struct FunctionTraits : public FunctionTraits<decltype( &T::operator() )> {};

template <typename R, typename... Args>
struct FunctionTraits<R(*)( Args... )> : public FunctionTraits<R( Args... )>{};

template <typename F>
struct FunctionTraits< std::function<F> > : public FunctionTraits<F>{};

template <typename F>
struct FunctionTraits< taro::Function<F> > : public FunctionTraits<F>{};

//member function
#define FUNCTION_TRAITS(...) \
    template <typename R, typename C, typename... Args>\
    struct FunctionTraits<R(C::*)(Args...) __VA_ARGS__> : FunctionTraits<R(Args...)>{}; \

FUNCTION_TRAITS()
FUNCTION_TRAITS( const )
FUNCTION_TRAITS( volatile )
FUNCTION_TRAITS( const volatile )

template<typename T>
struct IsClassFunction
{
    static constexpr bool value = false;
};

#define IS_MEMBER_TRAITS(...) \
    template <typename R, typename C, typename... Args>\
    struct IsClassFunction<R(C::*)(Args...) __VA_ARGS__>{ static constexpr bool value = true; }; \

IS_MEMBER_TRAITS()
IS_MEMBER_TRAITS( const )
IS_MEMBER_TRAITS( volatile )
IS_MEMBER_TRAITS( const volatile )

template<typename T>
struct IsObjectFunction
{
    static constexpr bool value = true;
};

template <typename R, typename... Args>
struct IsObjectFunction<R( * )( Args... )>
{
    static constexpr bool value = false;
};

template <typename R, typename... Args>
struct IsObjectFunction<R( Args... )>
{
    static constexpr bool value = false;
};

#define IS_OBJ_TRAITS(...) \
    template <typename R, typename C, typename... Args>\
    struct IsObjectFunction<R(C::*)(Args...) __VA_ARGS__>{ static constexpr bool value = false; }; \

IS_OBJ_TRAITS()
IS_OBJ_TRAITS( const )
IS_OBJ_TRAITS( volatile )
IS_OBJ_TRAITS( const volatile )

NAMESPACE_TARO_END
