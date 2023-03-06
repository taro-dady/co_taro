
#pragma once

#include "base/base.h"
#include <memory>

NAMESPACE_TARO_BEGIN

template<typename T>
class Optional
{
PUBLIC: // 公共类型

    using value_type = T;

PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    Optional() = default;

    /**
    * @brief 构造函数
    */
    Optional( T const& value )
        : value_( new T )
    {
        *value_ = value;
    }

    /**
    * @brief 构造函数
    */
    Optional( Optional<T> const& r )
    {
        if ( r.value_ == nullptr )
        {
            return;
        }

        value_.reset( new T );
        *value_ = *r.value_;
    }

    /**
    * @brief 构造函数
    */
    Optional( Optional<T>&& r )
        : value_( std::move( r.value_ ) )
    {
        
    }

    /**
    * @brief 赋值函数
    */
    Optional& operator=( Optional<T> const& r )
    {
        if ( r.value_ == nullptr )
        {
            value_.reset();
            return *this;
        }

        if ( nullptr == value_ )
        {
            value_.reset( new T );
        }
        *value_ = *r.value_;
        return *this;
    }

    /**
    * @brief 赋值函数
    */
    Optional& operator=(T const& value)
    {
        if ( nullptr == value_ )
        {
            value_.reset( new T );
        }
        *value_ = value;
        return *this;
    }

    /**
    * @brief 取值函数
    */
    T value() const
    {
        return *value_;
    }
    
    /**
    * @brief 取引用函数
    */
    operator T& ()
    {
        return *value_;
    }

    /**
    * @brief 取指针
    */
    T* operator->()
    {
        if ( nullptr == value_ )
        {
            value_.reset( new T );
        }
        return value_.get();
    }

    /**
    * @brief 有效判断
    */
    bool valid() const
    {
        return value_ != nullptr;
    }

PRIVATE: // 私有变量

    std::unique_ptr<T> value_;
};

NAMESPACE_TARO_END
