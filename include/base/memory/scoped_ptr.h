
#pragma once

#include "base/memory/details/destroyer.h"
#include <cstddef>

NAMESPACE_TARO_BEGIN

// 作用与std::unique_ptr相同,解决unique_ptr必须接受完整类，无法用于接口类的问题
template<class T>
class ScopedPtr
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    ScopedPtr()
        : p_( nullptr )
        , del_( nullptr )
    {

    }

    /**
    * @brief 构造函数
    */
    explicit ScopedPtr( std::nullptr_t )
        : p_( nullptr )
        , del_( nullptr )
    {

    }

    /**
    * @brief  构造函数
    * 
    * @param[in] p 对象指针
    */
    explicit ScopedPtr( T* p )
        : p_( p )
        , del_( new detail::NormalDestroyer<T>( p ) )
    {

    }

    /**
    * @brief 构造函数
    * 
    * @param[in] p   对象指针
    * @param[in] del 指针释放方法
    */
    template<class D>
    ScopedPtr( T* p, D del )
        : p_( p )
        , del_( new detail::UserDestroyer<T, D>( p, del ) )
    {

    }

    /**
    * @brief 构造函数
    */
    explicit ScopedPtr( ScopedPtr&& other )
        : p_( other.p_ )
        , del_( other.del_ )
    {
        other.del_ = nullptr;
        other.p_   = nullptr;
    }

    /**
    * @brief 析构函数
    */
    ~ScopedPtr()
    {
        if( del_ )
        {
            del_->release();
            delete del_;
        }
        del_ = nullptr;
    }

    /**
    * @brief 获取引用
    */
    T& operator*() const
    {
        return *p_;
    }

    /**
    * @brief 获取指针
    */
    T* operator->() const
    {
        return p_;
    }

    /**
    * @brief 重置指针
    * 
    * @param[in] p   对象指针
    */
    void reset( T* p = nullptr )
    {
        ScopedPtr( p ).swap( *this );
    }

    /**
    * @brief 重置指针
    * 
    * @param[in] p   对象指针
    * @param[in] del 指针释放方法
    */
    template<class D>
    void reset( T* p, D del )
    {
        ScopedPtr( p, del ).swap( *this );
    }

    /**
    * @brief 有效性判断
    */
    explicit operator bool() const
    {
        return p_ != nullptr;
    }

    /**
    * @brief 获取原始指针
    */
    T* get() const
    {
        return p_;
    }

PRIVATE: // 私有函数

    /**
    * @brief 交换对象
    */
    void swap( ScopedPtr<T>& r )
    {
        T*                 ptr = p_;
        detail::Destroyer* del = del_;

        p_     = r.p_;
        del_   = r.del_;
        r.p_   = ptr;
        r.del_ = del;
    }

    // 禁止拷贝构造
    TARO_NO_COPY( ScopedPtr );

PRIVATE: // 私有变量

    T*                 p_;
    detail::Destroyer* del_;
};

NAMESPACE_TARO_END

template<class T, class U>
bool operator==( taro::ScopedPtr<T> const& l, taro::ScopedPtr<U> const& r )
{
    return l.get() == r.get();
}

template<class T, class U>
bool operator!=( taro::ScopedPtr<T> const& l, taro::ScopedPtr<U> const& r )
{
    return l.get() != r.get();
}

template<class T>
bool operator==( taro::ScopedPtr<T> const& l, std::nullptr_t )
{
    return l.get() == nullptr;
}

template<class T>
bool operator==( std::nullptr_t, taro::ScopedPtr<T> const& r )
{
    return r.get() == nullptr;
}

template<class T>
bool operator!=( taro::ScopedPtr<T> const& l, std::nullptr_t )
{
    return nullptr != l.get();
}

template<class T>
bool operator!=( std::nullptr_t, taro::ScopedPtr<T> const& r )
{
    return nullptr != r.get();
}
