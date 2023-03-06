
#pragma once

#include "nlohmann_json.h"
#include "base/base.h"

NAMESPACE_TARO_BEGIN

using Json = nlohmann::json;

inline bool json_parse( std::string const& str, Json& value )
{
    try
    {
        value = nlohmann::json::parse( str );
        return true;
    }
    catch(const nlohmann::json::parse_error& )
    {
        return false;
    }
}

NAMESPACE_TARO_END
