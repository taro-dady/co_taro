
#pragma once

#include "base/utils/utils.h"
#include "base/memory/dyn_packet.h"
#include "base/memory/details/destroyer.h"
#include <functional>

NAMESPACE_TARO_BEGIN

// 写共享内存队列对象
class TARO_DLL_EXPORT ShmQueWriter
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    ShmQueWriter();

    /**
    * @brief 析构函数
    */
    ~ShmQueWriter();

    /**
    * @brief 初始化
    * 
    * @param[in] path        共享内存名称
    * @param[in] file_locker 文件锁路径
    * @param[in] block_num   共享内存块数量
    * @param[in] block_sz    共享内存块大小
    * @param[in] create      是否重新创建共享内存，创建后原内存失效
    */
    int32_t init( const char* path, const char* file_locker, uint32_t block_num, uint32_t block_sz, bool create = false );

    /**
    * @brief 是否有效
    */
    bool valid() const;

    /**
    * @brief 写数据
    * 
    * @param[in] buffer 数据
    * @param[in] bytes  数据大小
    */
    int32_t write( char* buffer, int32_t bytes );

PRIVATE: // 私有函数

    TARO_NO_COPY( ShmQueWriter );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

// 读共享内存队列对象
class TARO_DLL_EXPORT ShmQueReader
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    ShmQueReader();

    /**
    * @brief 析构函数
    */
    ~ShmQueReader();

    /**
    * @brief 初始化
    * 
    * @param[in] path        共享内存名称
    * @param[in] file_locker 文件锁路径
    * @param[in] block_num   共享内存块数量
    * @param[in] block_sz    共享内存块大小
    */
    int32_t init( const char* path, const char* file_locker, uint32_t block_num, uint32_t block_sz );

    /**
    * @brief 是否有效
    */
    bool valid() const;

    /**
    * @brief 读数据
    * 
    * @return packet 数据
    */
    DynPacketSPtr read();

    /**
    * @brief 读取数据
    * 
    * @param[in] func 读回调(可以将数据直接反序列化，省去一次拷贝)
    */
    bool read( std::function< void( char*, size_t ) > const& func );

PRIVATE: // 私有函数

    TARO_NO_COPY( ShmQueReader );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

using ShmQueWriterSPtr = std::shared_ptr<ShmQueWriter>;
using ShmQueReaderSPtr = std::shared_ptr<ShmQueReader>;

NAMESPACE_TARO_END
