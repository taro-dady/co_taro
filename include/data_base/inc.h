
#pragma once 

#include "base/serialize/str_serialize.h"
#include "data_base/db_helper.h"
#include "data_base/data_base.h"

NAMESPACE_TARO_DB_BEGIN

/**
* @brief  加载mysql的动态库
*/
extern TARO_DLL_EXPORT int32_t load_mysql_library( const char* path );

/**
* @brief  创建Database
*/
extern TARO_DLL_EXPORT DataBaseSPtr create_database( const char* type );

NAMESPACE_TARO_DB_END
