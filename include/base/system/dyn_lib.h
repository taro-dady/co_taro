
#pragma once

#include "base/base.h"

NAMESPACE_TARO_BEGIN

// 动态库
class TARO_DLL_EXPORT DynLib
{
PUBLIC: // 公共函数

    /**
	* @brief 构造函数
	*/
    DynLib();

    /**
	* @brief 构造函数
	*/
    DynLib( DynLib&& other );

    /**
    * @brief 析构函数
    */
    ~DynLib();

    /**
    * @brief 加载动态库
    * 
    * @param[in] path 动态库路径
    */
    int32_t load( const char* path );

    /**
    * @brief 卸载动态库
    */
    int32_t unload();

    /**
    * @brief 获取函数指针
    * 
    * @param[in] func_name 函数名称
    */
    void* func_addr( const char* func_name );

PRIVATE: // 私有函数

    TARO_NO_COPY( DynLib );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

NAMESPACE_TARO_END
