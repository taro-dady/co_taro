
#pragma once

#include "base/base.h"
#include <string>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <iostream>

NAMESPACE_TARO_BEGIN

// sha1加密
class TARO_DLL_EXPORT SHA1
{
PUBLIC: // 公共函数

    /**
     * @brief 编码
     * 
     * @param[in] text 数据
     * @param[in] len  数据长度
     * @return 编码后的数据
    */
    static std::string encode( const char* text, uint32_t len );
};

NAMESPACE_TARO_END
