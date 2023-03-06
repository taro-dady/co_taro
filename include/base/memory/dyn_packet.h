
#pragma once

#include "base/base.h"
#include <memory>

NAMESPACE_TARO_BEGIN

// 数据包接口
TARO_INTERFACE DynPacket
{
PUBLIC: // 公共函数定义

    /**
    * @brief      虚析构函数
    */
    virtual ~DynPacket() {};

    /**
    * @brief      添加数据
    *
    * @param[in]  buffer  数据指针
    * @param[in]  len     数据字节数
    */
    virtual bool append( void* buffer, uint32_t len ) = 0;

    /**
    * @brief      添加数据
    *
    * @param[in]  str  字符串
    */
    virtual bool append( std::string const& str ) = 0;

    /**
    * @brief      扩展存储大小
    *
    * @param[in]  sz 需要扩展的内存大小
    */
    virtual bool expand( uint32_t sz ) = 0;

    /**
    * @brief      获取数据大小
    *
    * @return     int64_t
    */
    virtual int64_t size() const = 0;

    /**
    * @brief      重置数据大小
    *
    * @param[in]  sz 数据大小
    */
    virtual bool resize( uint32_t sz ) = 0;

    /**
    * @brief      获取内存容量
    *
    * @return     int64_t
    */
    virtual int64_t capcity() const = 0;

    /**
    * @brief      获取数据地址 数据地址= 缓冲起始地址 + 头部偏移(由set_offset设置若不设置则为0)
    *
    * @return     void*
    * @retval     数据地址
    */
    virtual void* buffer() const = 0;
    
    /**
    * @brief      设置头部的偏移，用于头的填充 size = ( size - offset )
    *
    * @param[in]  offset 偏移大小
    */
    virtual bool set_head_offset( int64_t offset ) = 0;

    /**
    * @brief      获取头部偏移
    *
    * @return     int32_t 偏移大小
    */
    virtual int32_t get_head_offset() const = 0;

    /**
    * @brief      获取附加数据
    *
    * @return     void* 扩展数据指针
    */
    virtual void* extra_data() const = 0;

    /**
    * @brief      获取附加数据大小
    *
    * @return     uint32_t 扩展数据大小
    */
    virtual int32_t extra_data_size() const = 0;

    /**
    * @brief      扩展附加数据大小
    *
    * @param[in]  sz 需要扩展的内存大小
    */
    virtual bool expand_extra_data( uint32_t sz ) = 0;
};

/**
* @brief      创建数据包
*
* @param[in]  cap        容量
* @param[in]  extra_size 附加数据大小
* @param[in]  power      扩容时最小块大小 2^power
* @return     动态数据指针
*/
extern TARO_DLL_EXPORT DynPacket* create_packet( uint32_t cap = 0, uint32_t extra_size = 0, uint32_t power = 8 );

/**
* @brief 销毁函数
* 
* @param[in] obj 对象
*/
extern TARO_DLL_EXPORT void free_packet( DynPacket* obj );

// 动态数据智能指针
using DynPacketSPtr = std::shared_ptr<DynPacket>;
using DynPacketWPtr = std::weak_ptr<DynPacket>;

inline DynPacketSPtr create_default_packet( uint32_t cap )
{
    return DynPacketSPtr( create_packet( cap ), free_packet );
}

inline DynPacketSPtr string_packet( std::string const& str )
{
    auto packet = create_default_packet( ( uint32_t )str.length() );
    packet->append( str );
    return packet;
}

NAMESPACE_TARO_END
