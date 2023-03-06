
#pragma once

#include "base/error_no.h"
#include "base/utils/utils.h"
#include "base/utils/assert.h"
#include "base/system/spin_locker.h"
#include <mutex>
#include <memory>

NAMESPACE_TARO_BEGIN

template< typename ObjType >
class RWQueObjBlock
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    RWQueObjBlock()
        : value_( nullptr )
        , stamp_( 0 )
    {

    }

    /**
    * @brief 析构函数
    */
    ~RWQueObjBlock()
    {
        if ( value_ != nullptr )
        {
            delete value_;
        }
    }

    /**
    * @brief 写数据
    * 
    * @param[in] stamp 数据戳
    * @param[in] data  数据
    */
    void write( uint32_t stamp, ObjType const& value )
    {
        std::lock_guard<SpinLocker> g( mutex_ );
        if ( nullptr == value_ )
        {
            value_ = new ObjType;
        }
        *value_ = value;
        stamp_  = stamp;
    }

    /**
    * @brief 读数据
    * 
    * @param[in]  stamp 数据戳
    * @param[out] data  读取的数据
    */
    int32_t read( uint32_t stamp, ObjType& obj )
    {
        std::lock_guard<SpinLocker> g( mutex_ );
        if ( stamp != stamp_ )
        {
            return TARO_ERR_FAILED;
        }

        if ( nullptr == value_ )
        {
            return TARO_ERR_INVALID_RES; // 该block没有被赋值
        }
        obj = *value_;
        return TARO_OK;
    }

PRIVATE: // 私有函数

    TARO_NO_COPY( RWQueObjBlock );

PRIVATE: // 私有变量

    ObjType*   value_;
    SpinLocker mutex_;
    uint32_t   stamp_;
};

template< typename ObjType >
class RWQueWriter
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] cap    队列容量
    */
    RWQueWriter( uint32_t cap )
        : first_round_( true )
        , cap_( cap )
        , stamp_( 0 )
        , write_pos_( 0 )
        , blocks_( new RWQueObjBlock<ObjType>[cap] )
    {
        TARO_ASSERT( cap > 0, "cap error" );
    }

    /**
    * @brief 析构函数
    */
    ~RWQueWriter()
    {
        delete [] blocks_;
    }

    /**
    * @brief 写数据
    * 
    * @param[in] data  数据对象
    */
    void write( ObjType const& data )
    {
        uint32_t pos, st;
        {
            // 定位写入位置
            std::lock_guard<SpinLocker> g( mutex_ );
            pos = write_pos_++;
            st  = stamp_++;
            while( write_pos_ >= cap_ ) 
            {
                first_round_ = false;
                write_pos_  -= cap_;
            }
        }
        blocks_[pos].write( st, data );
    }

    /**
    * @brief 读数据
    * 
    * @param[in]  pos   数据位置
    * @param[in]  st    数据戳
    * @param[out] data  读取的数据
    */
    int32_t read( uint32_t pos, uint32_t st, ObjType& data )
    {
        if ( pos >= cap_ )
        {
            return TARO_ERR_INVALID_ARG;
        }
        return blocks_[pos].read( st, data );
    }

    /**
    * @brief 获取当前的元数据
    * 
    * @param[out] pos 当前的写入位置
    * @param[out] st  当前的数据戳
    */
    void get_meta( uint32_t& pos, uint32_t& st, bool latest = true )
    {
        if ( latest )
        {
            std::lock_guard<SpinLocker> g( mutex_ );
            if ( 0 == write_pos_ )
                pos = cap_ - 1;
            else
                pos = write_pos_ - 1;
            st = stamp_ - 1;
            return;
        }
        
        // 取当前能拿到的最旧的数
        std::lock_guard<SpinLocker> g( mutex_ );
        if ( first_round_ ) // 若使用stamp判断是否写满过，存在回绕的情况
        {
            // 队列还未被写满过，最旧的数据为第0个数据
            pos = 0;
            st  = 0;
        }
        else
        {
            // 队列已经被写满过，当前将被写入的位置即为最老的数据
            pos = write_pos_;
            st  = stamp_ - cap_;
        }
    }

    /**
    * @brief 获取当前的数据戳
    */
    uint32_t get_stamp()
    {
        std::lock_guard<SpinLocker> g( mutex_ );
        return stamp_;
    }

    /**
    * @brief 获取队列容量
    */
    uint32_t cap() const
    {
        return cap_;
    }

PRIVATE: // 私有函数

    TARO_NO_COPY( RWQueWriter );

PRIVATE: // 私有变量

    bool                    first_round_; // 队列是否被写满过
    SpinLocker              mutex_;
    uint32_t                cap_;         // 队列容量
    uint32_t                stamp_;       // 最新数据戳
    uint32_t                write_pos_;   // 最新写入位置
    RWQueObjBlock<ObjType>* blocks_;
};

template< typename ObjType >
using RWQueWriterSPtr = std::shared_ptr< RWQueWriter< ObjType > >;

template< typename ObjType >
using RWQueWriterWPtr = std::weak_ptr< RWQueWriter< ObjType > >;

// 读数据
template< typename ObjType >
class RWQueReader
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] writer 写对象
    * @param[in] latest 读取最新的数据 true 读取最老的数据 false
    */
    RWQueReader( RWQueWriterSPtr< ObjType > const& writer, bool latest = true )
        : latest_( latest )
        , cap_( writer->cap() )
        , writer_( writer )
    {
        writer->get_meta( pos_, stamp_, latest );
    }

    /**
    * @brief 读数据
    * 
    * @param[out] data  读取的数据 
    */
    int32_t read( ObjType& data )
    {
        auto sptr = writer_.lock();
        if ( nullptr == sptr )
        {
            return TARO_ERR_INVALID_RES;
        }

        while( 1 )
        {
            // 如果数据戳比当前的小，则说明还有数据没有读
            if ( is_newer<uint32_t>( sptr->get_stamp(), stamp_ ) )
            {
                auto ret = sptr->read( pos_, stamp_, data );
                if ( ret == TARO_OK )
                {
                    forward();
                    return TARO_OK;
                }
                else if ( ret == TARO_ERR_FAILED )
                {
                    // 数据戳不匹配说明写得太快读得太慢, 进行读指针同步后继续读取
                    sptr->get_meta( pos_, stamp_, latest_ );
                    continue;
                }
            }
            break;
        }
        return TARO_ERR_FAILED;
    }

PRIVATE: // 私有函数

    TARO_NO_COPY( RWQueReader );
    
    /**
    * @brief 向前移动一格
    */
    void forward()
    {
        ++stamp_;
        ++pos_;
        if( pos_ >= cap_ )
        {
            pos_ -= cap_;
        }
    }

PRIVATE: // 私有变量

    bool                       latest_;
    uint32_t                   cap_;      // 队列容量
    uint32_t                   stamp_;    // 当前读到的数据戳
    uint32_t                   pos_;      // 当前读的位置
    RWQueWriterWPtr< ObjType > writer_;   // 写对象
};

template< typename ObjType >
using RWQueReaderSPtr = std::shared_ptr< RWQueReader< ObjType > >;

NAMESPACE_TARO_END
