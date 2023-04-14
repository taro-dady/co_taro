
#pragma once

#include "base/memory/optional.h"
#include <limits>
#include <vector>
#include <sstream>
#include <memory>

NAMESPACE_TARO_BEGIN

// 用于匿名函数名称的组装
inline std::string trace_name( const char* file, int line )
{
    std::stringstream ss;
    ss << file << ":" << line;
    return ss.str();
}

// 序列号比较(处理序列号回绕)
template<typename T>
inline bool is_newer( T value, T pre_val )
{
    // 无符号数据的中值
    constexpr uint32_t point = ( ( std::numeric_limits<T>::max )() >> 1 );
    if ( taro_likely( value > pre_val ) )
        return true;
    
    if ( value == pre_val )
        return false;
    
    // 是否是回绕的判断
    return ( pre_val - value ) > point;
}

// 转换为数字
inline Optional<int32_t> to_number( std::string const &str )
{
    std::stringstream ss;
    ss << str;
    int32_t value;
    ss >> value;

    Optional<int32_t> ret;
    if ( !ss.fail() )
        ret = value;
    return ret;
}

template<typename T>
class Singleton
{
PUBLIC:
    
    static T& instance()
    {
        static T inst;
        return inst;
    }
};

NAMESPACE_TARO_END

#define TRACE_NAME taro::trace_name( __FILE__, __LINE__  )
