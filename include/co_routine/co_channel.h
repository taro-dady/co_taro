

#pragma once

#include "base/memory/rw_que.h"
#include "co_routine/co_condition.h"
#include <map>

NAMESPACE_TARO_RT_BEGIN

using NotifySPtr = std::shared_ptr<rt::Condition>;

template<typename T>
class Channel
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] cap     对象数量
    * @param[in] lastest true 读取最新的数据 false 读取最老的数据
    */
    Channel( uint32_t cap = 32, bool lastest = false )
        : lastest_( lastest )
        , notify_( std::make_shared<rt::Condition>() )
        , locker_map_( std::make_shared< LockerMap >() )
        , writer_( std::make_shared< RWQueWriter<T> >( cap ) )
    {

    }

    /**
    * @brief 析构函数
    */
    ~Channel()
    {
        
    }

    /**
    * @brief 写数据
    * 
    * @param[in] data 数据
    */
    void write( T const& data )
    {
        writer_->write( data );
        notify_->notify_all();
    }

    /**
    * @brief 读数据
    * 
    * @param[out] data  读取的数据
    * @param[in]  ms    等待时间 0表示永远等待 
    */
    bool read( T& data, uint32_t ms = 0 )
    {
        RWQueReaderSPtr<T> reader;
        {
            auto co  = CoRoutine::current();
            auto cid = co->cid();
            
            std::lock_guard<SpinLocker> g( locker_map_->mutex_ );
            auto it = locker_map_->co_map_.find( cid );
            if ( taro_likely( it != locker_map_->co_map_.end() ) )
            {
                reader = it->second;
            }
            else
            {
                reader = std::make_shared< RWQueReader<T> >( writer_, lastest_ );
                locker_map_->co_map_[cid] = reader;

                LockerMapWPtr wp( locker_map_ );
                co->on_exit([=]()
                {
                    auto lock_map_sptr = wp.lock();
                    if ( lock_map_sptr != nullptr )
                    {
                        std::lock_guard<SpinLocker> g( locker_map_->mutex_ );
                        lock_map_sptr->co_map_.erase( cid );
                    }
                } );
            }
        }

        if ( 0 == ms )
        {
            while( reader->read( data ) != TARO_OK )
            {
                notify_->cond_wait( 0 );
            }
            return true;
        }
        
        if( reader->read( data ) != TARO_OK )
        {
            notify_->cond_wait( ms );
            return reader->read( data ) == TARO_OK;
        }
        return true;
    }

PRIVATE: // 私有类型

    using CoMap = std::map< int32_t, RWQueReaderSPtr<T> >;

    struct LockerMap
    {
        CoMap      co_map_;
        SpinLocker mutex_;
    };
    using LockerMapSPtr = std::shared_ptr<LockerMap>;
    using LockerMapWPtr = std::weak_ptr<LockerMap>;

PRIVATE: // 私有函数

    TARO_NO_COPY( Channel );

    /**
    * @brief 读数据
    * 
    * @param[out] data 数据
    * @param[in]  chn  通道对象
    */
    friend void operator<<( T& data, Channel<T>& chn )
    {
        chn.read( data );
    }

    /**
    * @brief 写数据
    * 
    * @param[in] data 数据
    * @param[in] chn  通道对象
    */
    friend void operator>>( T const& data, Channel<T>& chn )
    {
        chn.writer_->write( data );
    }

PRIVATE: // 私有变量

    bool               lastest_;
    NotifySPtr         notify_;
    LockerMapSPtr      locker_map_;
    RWQueWriterSPtr<T> writer_;
};

NAMESPACE_TARO_RT_END
