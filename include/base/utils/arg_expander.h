
#pragma once

#include "base/base.h"
#include <string.h>
#include <sstream>

NAMESPACE_TARO_BEGIN

template<typename C, typename T>
inline bool value_check( C checker, T value )
{
    return checker( value );
}

template<typename C, typename T, typename... Args>
inline bool value_check( C checker, T value, Args... args )
{
    if ( !checker( value ) )
        return false;
    return value_check( checker, args... );
}

inline bool check_str( const char* v )
{
    if ( v == nullptr || strlen( v ) == 0 )
        return false;
    return true;
}
#define STRING_CHECK(...) value_check( check_str, ##__VA_ARGS__ )

template<typename C, typename T>
inline void arg_expand( C op, T value )
{
    op( value );
}

template<typename C, typename T, typename... Args>
inline void arg_expand( C op, T value, Args... args )
{
    op( value );
    arg_expand( op, args... );
}

// 向容器中添加数据
template<typename Container, typename... Args>
inline void append_container( Container& container, Args&&... args )
{
    auto func = [&]( typename Container::value_type& value )
    {
        container.push_back( value );
    };
    arg_expand( func, std::forward<Args>( args )... );
}

template<typename C, typename T, typename... Args>
inline void type_to_str_expand( C func, T&& value, Args&&... args )
{
    std::stringstream ss;
    ss << value;
    func( ss.str().c_str() );
    type_to_str_expand( func, std::forward<Args>( args )... );
}

template<typename C, typename T>
inline void type_to_str_expand( C func, T&& value )
{
    std::stringstream ss;
    ss << value;
    func( ss.str().c_str() );
}

template<typename T, typename... Args>
inline void type_to_os_expand( std::ostream& ss, const char* split, T&& value, Args&&... args )
{
    ss << value;
    if ( nullptr != split )
        ss << split;
    type_to_os_expand( ss, split, std::forward<Args>( args )... );
}

template<typename T>
inline void type_to_os_expand( std::ostream& ss, const char*, T&& value )
{
    ss << value;
}

NAMESPACE_TARO_END
