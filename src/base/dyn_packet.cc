
#include "base/memory/dyn_packet.h"
#include <string.h>

NAMESPACE_TARO_BEGIN

class DynPacketDefault : PUBLIC DynPacket
{
PUBLIC: // 公共函数

    /**
    * @brief      构造函数
    */
    DynPacketDefault( uint32_t extra_size, uint32_t power )
        : size_( 0 )
        , capcity_( 0 )
        , offset_( 0 )
        , align_( 1 << power )
        , extra_( nullptr )
        , extra_bytes_( extra_size )
        , buffer_( nullptr )
    {
        if ( extra_size > 0 )
            extra_ = ::malloc( extra_size );
    }

    /**
    * @brief      构造函数
    */
    DynPacketDefault()
        : size_( 0 )
        , capcity_( 0 )
        , offset_( 0 )
        , align_( 0 )
        , extra_( nullptr )
        , extra_bytes_( 0 )
        , buffer_( nullptr )
    {

    }

    /**
    * @brief      析构函数
    */
    ~DynPacketDefault()
    {
        if ( extra_ != nullptr )
        {
            ::free( extra_ );
            extra_ = nullptr;
        }

        if ( buffer_ != nullptr )
        {
            ::free( buffer_ );
            buffer_ = nullptr;
        }
    }

    /**
    * @brief      添加数据
    *
    * @param[in]  buffer  数据指针
    * @param[in]  len     数据字节数
    */
    virtual bool append( void* buffer, uint32_t len ) override
    {
        if ( ( nullptr == buffer ) || ( 0 == len ) )
        {
            return false;
        }

        auto rest = capcity_ - size_ - offset_;
        if ( rest < ( uint32_t )len )
        {
            if ( !expand( size_ + offset_ + len ) )
            {
                return false;
            }
        }
        memcpy( buffer_ + size_ + offset_, ( uint8_t* )buffer, len );
        size_ += len;
        return true;
    }

    /**
    * @brief      添加数据
    *
    * @param[in]  str  字符串
    */
    virtual bool append( std::string const& str ) override
    {
        if ( str.length() == 0 )
            return false;
        return append( ( void* )str.c_str(), ( uint32_t )str.length() );
    }

    /**
    * @brief      扩展存储大小
    *
    * @param[in]  sz 需要扩展的内存大小
    */
    virtual bool expand( uint32_t sz ) override
    {
        if ( sz <= capcity_ )
        {
            return true;
        }

        auto mem_sz = TARO_ALIGN( sz, align_ );
        if ( nullptr == buffer_ )
        {
            buffer_ = ( uint8_t* )::malloc( mem_sz );
        }
        else
        {
            auto* tmp = ( uint8_t* )::realloc( buffer_, mem_sz );
            if( nullptr == tmp )
            {
                return false;
            }
            buffer_ = tmp;
        }
        capcity_ = mem_sz;
        return true;
    }

    /**
    * @brief      获取数据大小
    *
    * @return     int64_t
    */
    virtual int64_t size() const override
    {
        return size_;
    }

    /**
    * @brief      重置数据大小
    *
    * @param[in]  sz 数据大小
    */
    virtual bool resize( uint32_t sz ) override
    {
        if ( ( capcity_ - offset_ ) < sz )
        {
            return false;
        }
        size_ = sz;
        return true;
    }

    /**
    * @brief      获取内存容量
    *
    * @return     int64_t
    */
    virtual int64_t capcity() const override
    {
        return capcity_;
    }

    /**
    * @brief      获取数据地址 数据地址= 缓冲起始地址 + 头部偏移(由set_offset设置若不设置则为0)
    *
    * @return     void*
    * @retval     数据地址
    */
    virtual void* buffer() const override
    {
        if ( nullptr == buffer_ )
        {
            return nullptr;
        }
        return buffer_ + offset_;
    }

    /**
    * @brief      设置头部的偏移，用于头的填充，需要手动调用resize重新调整packet数据大小( size - offset )
    *
    * @param[in]  offset 偏移大小
    */
    virtual bool set_head_offset( int64_t offset ) override
    {
        if ( size_ + offset_ < ( uint32_t )offset )
        {
            return false;
        }
        offset_ = ( uint32_t )offset;
        return true;
    }

    /**
    * @brief      获取头部偏移
    *
    * @return     int32_t 偏移大小
    */
    virtual int32_t get_head_offset() const override
    {
        return offset_;
    }

    /**
    * @brief      获取附加数据
    *
    * @return     void* 扩展数据指针
    */
    virtual void* extra_data() const override
    {
        return extra_;
    }

    /**
    * @brief      获取附加数据大小
    *
    * @return     uint32_t 扩展数据大小
    */
    virtual int32_t extra_data_size() const override
    {
        return extra_bytes_;
    }

    /**
    * @brief      扩展附加数据大小
    *
    * @param[in]  sz 需要扩展的内存大小
    */
    virtual bool expand_extra_data( uint32_t sz ) override
    {
        if ( sz < extra_bytes_ )
            return true;

        if ( nullptr == extra_ )
        {
            extra_ = malloc( sz );
        }
        else
        {
            void* tmp = ::realloc( extra_, sz );
            if ( tmp != nullptr )
                extra_ = tmp;
            else
                return false;
        }
        return true;
    }

PRIVATE: // 私有变量

    uint32_t size_;            /// 数据大小
    uint32_t capcity_;         /// 缓冲大小
    uint32_t offset_;          /// 头部偏移
    uint32_t align_;           /// 内存对齐字节
    void*    extra_;           /// 附加数据区
    uint32_t extra_bytes_;
    uint8_t* buffer_;          /// 数据缓冲
};

extern TARO_DLL_EXPORT DynPacket*  create_packet( uint32_t cap, uint32_t extra_size, uint32_t power )
{
    DynPacket* pkt = new DynPacketDefault( extra_size, power );
    if ( pkt == nullptr )
    {
        return pkt;
    }

    if ( cap > 0 )
    {
        if ( !pkt->expand( cap ) )
        {
            return nullptr;
        }
    }
    return pkt;
}

extern TARO_DLL_EXPORT void free_packet( DynPacket* obj )
{
    delete obj;
}

NAMESPACE_TARO_END