
#pragma once

#include "base/function/function.h"
#include "base/system/spin_locker.h"
#include <list>
#include <mutex>
#include <algorithm>

NAMESPACE_TARO_BEGIN

template<class ...Args>
class Observers
{
PUBLIC: // 公共类型定义

    using ObserverType = Function<void( Args... )>;

PUBLIC: // 公共函数定义

    bool push_back( ObserverType const& f )
    {
        if( !f )
        {
            return false;
        }

        std::lock_guard<SpinLocker> g( locker_ );
        if( existed( f, bool() ) )
        {
            return false;
        }
        list_.emplace_back( f );
        return true;
    }

    bool push_front( ObserverType const& f )
    {
        if( !f )
        {
            return false;
        }

        std::lock_guard<SpinLocker> g( locker_ );
        if( is_existed( f ) )
        {
            return false;
        }
        list_.emplace_front( f );
        return true;
    }

    bool remove( ObserverType const& f )
    {
        if( !f )
        {
            return false;
        }

        std::lock_guard<SpinLocker> g( locker_ );
        auto it = std::find( std::begin( list_ ), std::end( list_ ), f );
        if( it != std::end( list_ ) )
        {
            list_.erase( it );
            return true;
        }
        return false;
    }

    bool existed( ObserverType const& f ) const
    {
        std::lock_guard<SpinLocker> g( locker_ );
        return is_existed( f );
    }

    void operator()( Args... args )
    {
        std::list<ObserverType> temp;
        {
            std::lock_guard<SpinLocker> g( locker_ );
            temp = list_;
        }

        std::list<ObserverType> remove_list;
        for( auto& f : temp )
        {
            try
            {
                f( args... );
            }
            catch( ... )
            {
                remove_list.emplace_back( f );
            }
        }

        if( !remove_list.empty() )
        {
            std::lock_guard<SpinLocker> g( locker_ );
            for( auto& f : remove_list )
            {
                auto it = std::find( std::begin( list_ ), std::end( list_ ), f );
                if( it != std::end( list_ ) )
                {
                    list_.erase( it );
                }
            }
        }
    }

    bool empty() const
    {
        std::lock_guard<SpinLocker> g( locker_ );
        return list_.empty();
    }

PRIVATE: // 私有函数

    bool is_existed( ObserverType const& f ) const
    {
        return std::find( std::begin( list_ ), std::end( list_ ), f ) != std::end( list_ );
    }

PRIVATE: // 私有变量

    mutable SpinLocker      locker_;
    std::list<ObserverType> list_;
};

NAMESPACE_TARO_END
