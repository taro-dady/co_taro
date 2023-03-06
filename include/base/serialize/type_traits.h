
#pragma once

#include "base/memory/optional.h"
#include <list>
#include <vector>
#include <tuple>
#include <type_traits>

NAMESPACE_TARO_BEGIN

template<typename T>
using DecayType = typename std::decay<T>::type;

template< typename T >
struct UnWrapperParam
{
    using type = typename std::remove_cv<typename std::remove_reference< T >::type>::type;
};

#define IS_CLASS_MEMBER( MEMBER )\
template<typename U>\
struct IsClsMember##MEMBER\
{\
    template<typename T>\
    static void check( decltype( &T::MEMBER ) );\
    template<typename T>\
    static int check(...);\
    enum{ value = std::is_void< decltype( check<U>(0) ) >::value};\
};

/* array */
template <class T> 
struct IsArray : std::false_type {};

template <class T>
struct IsArray< std::vector<T> > : std::true_type {};

template <class T>
struct IsArray< std::list<T> > : std::true_type {};

/* vector */
template <class T> 
struct IsVector : std::false_type {};

template <class T> 
struct IsVector< std::vector<T> > : std::true_type {};

/* list */
template <class T> 
struct IsList : std::false_type {};

template <class T> 
struct IsList< std::list<T> > : std::true_type {};

/* optional */
template <class T> 
struct IsOptional : std::false_type {};

template <class T> 
struct IsOptional< Optional<T> > : std::true_type {};

/* tuple */
template<typename T>
struct IsTuple : std::false_type {};

template<typename... Ts>
struct IsTuple< std::tuple<Ts...> > : std::true_type {};

/* pair */
template <typename T>
struct IsPair : std::false_type {};

template <typename T, typename U>
struct IsPair<std::pair<T, U>> : std::true_type {};

/* map */
template <typename, typename = void>
struct IsMap : std::false_type {};

template <typename Container>
struct IsMap<
Container, 
typename std::enable_if<
    IsPair<typename std::iterator_traits<typename Container::iterator>::value_type>::value
>::type
> : std::true_type {};

NAMESPACE_TARO_END
