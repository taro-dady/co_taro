
#pragma once

#include "base/base.h"
#include <string>
#include <string.h>

NAMESPACE_TARO_BEGIN

// base64编解码
struct TARO_DLL_EXPORT Base64
{
PUBLIC: // 公共函数

    /**
     * @brief 编码
     * 
     * @param[in] in 需要编码的数据
     * @return 编码后的数据
    */
    static std::string encode( std::string const& in );
    
    /**
     * @brief 解码
     * 
     * @param[in] in 需要解码的数据
     * @return 解码后的数据
    */
    static std::string decode( std::string const& in );
};

NAMESPACE_TARO_END
