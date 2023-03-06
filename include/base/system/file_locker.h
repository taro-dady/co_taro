
#pragma once

#include "base/base.h"

NAMESPACE_TARO_BEGIN

// 文件锁
class TARO_DLL_EXPORT FileLocker
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    FileLocker();

    /**
	* @brief 构造函数
	*/
    FileLocker( FileLocker&& other );

    /**
    * @brief 析构函数
    */
    ~FileLocker();

    /**
    * @brief 初始化
    * 
    * @param[in] 文件路径
    */
    int32_t init( const char* path );

    /**
    * @brief 写锁
    * 
    * @param[in] start  锁在文件中的启始位置
    * @param[in] end    锁在文件中的结束位置
    */
    void write_lock( size_t start = 0, size_t end = 0 );

    /**
    * @brief 尝试写锁
    * 
    * @param[in] start  锁在文件中的启始位置
    * @param[in] end    锁在文件中的结束位置
    * @return 锁的结果
    */
    bool try_write_lock( size_t start = 0, size_t end = 0 );

    /**
    * @brief 读锁
    * 
    * @param[in] start  锁在文件中的启始位置
    * @param[in] end    锁在文件中的结束位置
    */
    void read_lock( size_t start = 0, size_t end = 0 );

    /**
    * @brief 尝试读锁
    * 
    * @param[in] start  锁在文件中的启始位置
    * @param[in] end    锁在文件中的结束位置
    * @return 锁的结果
    */
    bool try_read_lock( size_t start = 0, size_t end = 0 );

    /**
    * @brief 解锁
    * 
    * @param[in] start  锁在文件中的启始位置
    * @param[in] end    锁在文件中的结束位置
    * @return 锁的结果
    */
    void unlock( size_t start = 0, size_t end = 0 );

PRIVATE: // 私有函数

    TARO_NO_COPY( FileLocker );
    
PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

NAMESPACE_TARO_END

