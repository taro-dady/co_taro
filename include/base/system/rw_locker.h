
#pragma once

#include "base/base.h"

NAMESPACE_TARO_BEGIN

// 读写锁
class TARO_DLL_EXPORT RWLocker
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    RWLocker();

    /**
    * @brief 析构函数
    */
    ~RWLocker();

    /**
    * @brief 读锁
    */
    void read_lock();
    
    /**
    * @brief 写锁
    */
    void write_lock();
    
    /**
    * @brief 读解锁
    */
    void read_unlock();

    /**
    * @brief 写解锁
    */
    void write_unlock();

PRIVATE: // 私有函数

    TARO_NO_COPY( RWLocker );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

class TARO_DLL_EXPORT WriteGuard
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] locker 读写锁的引用
    */
    WriteGuard( RWLocker& locker );

    /**
    * @brief 析构函数
    */
    ~WriteGuard();

PRIVATE: // 私有函数

    TARO_NO_COPY( WriteGuard );

PRIVATE: // 私有变量

    RWLocker& locker_;
};

class TARO_DLL_EXPORT ReadGuard
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] locker 读写锁的引用
    */
    ReadGuard( RWLocker& locker );

    /**
    * @brief 析构函数
    */
    ~ReadGuard();

PRIVATE: // 私有函数

    TARO_NO_COPY( ReadGuard );

PRIVATE: // 私有变量

    RWLocker& locker_;
};

NAMESPACE_TARO_END
