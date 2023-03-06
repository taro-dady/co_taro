
#pragma once

#include "base/utils/arg_expander.h"
#include <assert.h>

NAMESPACE_TARO_BEGIN

template<typename... Args>
inline typename std::enable_if< sizeof...( Args ) != 0, void >::type
assert_fail( const char *str, const char *file, size_t line, Args&&... args )
{
    std::stringstream ss;
    type_to_os_expand( ss, " ", std::forward<Args>( args )... );
    std::cerr << "Assertion failed " << str << " " << file << ":" << line << " " << ss.str() << std::endl;
    abort();
}

template<typename... Args>
inline typename std::enable_if< sizeof...( Args ) == 0, void >::type
assert_fail( const char* str, const char* file, size_t line, Args&&... )
{
    std::cerr << "Assertion failed " << str << " " << file << ":" << line << std::endl;
    abort();
}

NAMESPACE_TARO_END

#if defined( _WIN32 ) || defined( _WIN64 )
#define TARO_ASSERT( x, ... ) if (!(x)) { taro::assert_fail( #x, __FILE__, __LINE__, ##__VA_ARGS__ ); }
#else
#define TARO_ASSERT( x, ... ) if (!(x)) { taro::assert_fail( #x, __FILE__, __LINE__, #__VA_ARGS__ ); }
#endif

