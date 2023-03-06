
#pragma once

#include "base/base.h"
#include <string.h>
#include <sstream>

NAMESPACE_TARO_BEGIN

inline std::string string_trim( std::string const& str, const char* f = " " )
{
    auto s = str.find_first_not_of( f );
    auto e = str.find_last_not_of( f );
    return str.substr( s, e - s + 1 );
}

inline std::string string_trim_back( std::string const& str, const char* f = " " )
{
    auto e = str.find_last_not_of( f );
    return str.substr( 0, e + 1 );
}

NAMESPACE_TARO_END
