
#pragma once

#include "base/base.h"
#include <string>
#include <sstream>
#include <type_traits>

NAMESPACE_TARO_BEGIN

// const char*/char*/std::string 均为字符型
template<class T> 
struct IsString
    : std::integral_constant< 
        bool,
        std::is_same<char*, typename std::remove_reference<typename std::remove_cv<T>::type>::type>::value ||
        std::is_same<char const*, typename std::remove_reference<typename std::remove_cv<T>::type>::type>::value || 
        std::is_same<std::string, typename std::decay<T>::type>::value
    > {};

// 非字符型数据转换为字符
template<typename T>
inline typename std::enable_if< !IsString<T>::value, std::string >::type
value_to_str( T const& val )
{
    std::stringstream ss;
    ss << val;
    return ss.str();
}

// 字符型数据转换为字符
template<typename T>
inline typename std::enable_if< IsString<T>::value, std::string >::type
value_to_str( T const& val )
{
    return val;
}

template<typename T>
T str_to_value( std::string const& str )
{
    std::stringstream ss;
    ss << str;
    T value;
    ss >> value;
    return value;
}

NAMESPACE_TARO_END
