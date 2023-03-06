
#pragma once

#include "base/utils/assert.h"
#include <mutex>
#include <vector>
#include <stdlib.h>
#include <string.h>

NAMESPACE_TARO_BEGIN

// id管理对象
class TARO_DLL_EXPORT IdAllocator
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    IdAllocator();

    /**
    * @brief 析构函数
    */
    ~IdAllocator();

    /**
    * @brief 是否为空
    */
    bool empty() const;

    /**
    * @brief 初始化
    * 
    * @param[in] num 最大的数量
    */
    void init( size_t num );

    /**
    * @brief 申请id
    * 
    * @return >=0 正常的id <0 见错误码
    */
    int32_t alloc_id();

    /**
    * @brief 释放id
    * 
    * @param[in] id 待释放的id
    */
    int32_t free_id( int32_t id );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

NAMESPACE_TARO_END