
#pragma once

#include "base/base.h"
#include <vector>
#include <string.h>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <functional>

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

inline std::string to_upper( std::string str )
{
    transform( str.begin(), str.end(), str.begin(), ::toupper );
    return str;
}

inline std::string to_lower( std::string str )
{
    transform( str.begin(), str.end(), str.begin(), ::tolower );
    return str;
}

inline bool string_compare( std::string const& a, 
                            std::string const& b, 
                            std::function<std::string( std::string )> const& transfer = to_upper )
{
    return transfer( a ) == transfer( b );
}

inline std::vector<std::string> split_string( std::string const& str, char sep = ' ' )
{
    std::string tmp;
    std::vector<std::string> val;
    std::istringstream instream( str );
    while ( getline( instream, tmp, sep ) )
        val.push_back( tmp );
    return val;
}

inline bool is_wildcard( std::string const& str )
{
    return str.find( "?" ) != std::string::npos || str.find( "*" ) != std::string::npos;
}

inline bool wildcard_match( std::string const& pattern, std::string const& compare_str )
{
    char* pat = ( char* )pattern.c_str();
    char* str = ( char* )compare_str.c_str();
    char* s = NULL;
    char* p = NULL;
    bool star = false;
    bool is_break = false;
    do
    {
        is_break = false;
        for( s = str, p = pat; *s; ++s, ++p )
        {
            switch( *p )
            {
            case '?':
                break;
            case '*':
                star = true;
                str = s;
                pat = p;
                if( !*++pat ) return true;
                is_break = true;
                break;
            default:
                if( *s != *p )
                {
                    if( !star ) return false;
                    str++;
                    is_break = true;
                }
                break;
            }

            if( is_break )
                break;
        }

        if( !is_break )
        {
            if( *p == '*' ) ++p;
            return ( !*p );
        }
    } while( true );
}

NAMESPACE_TARO_END
