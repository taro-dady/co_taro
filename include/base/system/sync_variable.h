
#pragma once

#include "base/base.h"

NAMESPACE_TARO_BEGIN

// 条件变量
struct CondVariableImpl;
class ConditionVariable;

// 互斥锁
class TARO_DLL_EXPORT Mutex
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    Mutex();

    /**
	* @brief 析构函数
	*/
    ~Mutex();

    /**
	* @brief 加锁
	*/
    void lock();

    /**
	* @brief 解锁
	*/
    void unlock();

PRIVATE: // 私有类型
    
    friend class ConditionVariable;

PRIVATE: // 私有函数

    TARO_NO_COPY( Mutex );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

// 条件变量
class TARO_DLL_EXPORT ConditionVariable
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    ConditionVariable();

    /**
	* @brief 析构函数
	*/
    ~ConditionVariable();

    /**
	* @brief 等待通知
    *
    * @param[in] mutex 互斥锁
	*/
    void wait( Mutex& mutex );

    /**
	* @brief 等待通知
    *
    * @param[in] mutex 互斥锁
    * @param[in] ms    毫秒  0 无限等待 >0 等待ms毫秒
    * @return  TARO_ERR_TIMEOUT: 超时  TARO_OK: 正常唤醒
	*/
    int32_t wait( Mutex& mutex, uint32_t ms );

    /**
	* @brief 唤醒单个对象
	*/
    void notify_one();

    /**
	* @brief 唤醒所有等待对象
	*/
    void notify_all();

PRIVATE: // 私有函数

    TARO_NO_COPY( ConditionVariable );

PRIVATE: // 私有变量
    
    CondVariableImpl* impl_;
};

NAMESPACE_TARO_END
