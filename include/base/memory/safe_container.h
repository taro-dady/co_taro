
#pragma once

#include "base/base.h"
#include "base/system/spin_locker.h"
#include <map>
#include <mutex>

NAMESPACE_TARO_BEGIN

template<typename Key, typename Value>
class KeyContainer
{
PUBLIC:

    /**
     * @brief 插入数据
     * 
     * @param[in] key   关键字
     * @param[in] value 值
     */
    bool insert( Key const& key, Value const& value )
    {
        std::unique_lock<SpinLocker> g( locker_ );
        return container_.insert( std::make_pair( key, value ) ).second;
    }

    /**
     * @brief 删除数据
     * 
     * @param[in] key 关键字
     */
    bool erase( Key const& key )
    {
        std::unique_lock<SpinLocker> g( locker_ );
        auto it = container_.find( key );
        if ( it != container_.end() )
        {
            container_.erase( it );
            return true;
        }
        return false;
    }

    /**
     * @brief 查询
     * 
     * @param[in] key 关键字
     */
    Value find( Key const& key )
    {
        std::unique_lock<SpinLocker> g( locker_ );
        auto it = container_.find( key );
        if ( it == container_.end() )
        {
            return Value();
        }
        return it->second;
    }

    /**
     * @brief 查询
     * 
     * @param[in]  key   关键字
     * @param[out] value 值
     */
    bool find( Key const& key, Value& value )
    {
        std::unique_lock<SpinLocker> g( locker_ );
        auto it = container_.find( key );
        if ( it == container_.end() )
        {
            return false;
        }
        value = it->second;
        return true;
    }

    /**
     * @brief 获取copy值
     */
    std::map<Key, Value> copy()
    {
        std::unique_lock<SpinLocker> g( locker_ );
        return container_;
    }

PRIVATE:

    // 容器定义
    using Container = std::map<Key, Value>;

PRIVATE:

    SpinLocker locker_;
    Container  container_;
};

NAMESPACE_TARO_END
