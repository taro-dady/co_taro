
#include "base/utils/arg_parser.h"
#include "base/utils/string_tool.h"
#include <map>
#include <string>

NAMESPACE_TARO_BEGIN

struct ArgParser::Impl
{
    std::map< std::string, std::function<void(const char*)> > parsers_;
};

inline bool parse_item( char* v, std::string& key, std::string& value )
{
    std::string cmd( v );
    auto pos = cmd.find( "=" );
    if( std::string::npos == pos )
        return false;

    key   = cmd.substr( 0, pos );
    value = cmd.substr( pos + 1 );
    key   = string_trim( key, "-" );
    return true;
}

ArgParser::ArgParser()
    : impl_( new Impl )
{

}

ArgParser::~ArgParser()
{
    delete impl_;
}

bool ArgParser::parse( int32_t argc, char* argv[] )
{
    for( int32_t i = 1; i < argc; ++i )
    {
        std::string key, value;
        if( !parse_item( argv[i], key, value ) )
        {
            return false;
        }

        auto it = impl_->parsers_.find( key );
        if( it != impl_->parsers_.end() )
        {
            it->second( value.c_str() );
        }
    }
    return true;
}

bool ArgParser::add_parser( const char* key, std::function<void(const char*)> const& callback )
{
    TARO_ASSERT( STRING_CHECK( key ) && callback );

    return impl_->parsers_.insert( std::make_pair( std::string( key ), callback ) ).second;
}

NAMESPACE_TARO_END
