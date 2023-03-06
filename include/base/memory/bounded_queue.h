
#pragma once

#include "base/base.h"
#include <mutex>

NAMESPACE_TARO_BEGIN

struct NullMutex{};

template<typename T, typename Mutex>
class BoundedQueue
{
PUBLIC:

    BoundedQueue()
        : start_( 0 )
        , count_( 0 )
        , cap_( 0 )
        , buffer_( nullptr )
    {
        
    }

    ~BoundedQueue()
    {
        clear();
        ::free( buffer_ );
        buffer_ = nullptr;
    }

    void init( size_t max_count )
    {
        cap_    = max_count;
        buffer_ = std::calloc( max_count, sizeof( T ) );
    }

    void push_front( T const& data )
    {
        std::lock_guard<Mutex> locker( mutex_ );
        if ( count_ >= cap_ )
        {
            expand();
        }
        
        start_ = start_ ? ( start_ - 1 ) : ( cap_ - 1 );
        ++count_;
        new ( ( T* )buffer_ + start_ ) T( data );
    }

    void push_back( T const& data )
    {
        std::lock_guard<Mutex> locker( mutex_ );
        if ( count_ >= cap_ )
        {
            expand();
        }

        new ( ( T* )buffer_ + this->mod( start_ + count_, cap_ ) ) T( data );
        ++count_;
    }

    bool front( T& data )
    {
        std::lock_guard<Mutex> locker( mutex_ );
        if ( count_ > 0 )
        {
            T* const p = ( T* )buffer_ + start_;
            data = *p;
            return true;
        }
        return false;
    }

    bool back( T& data )
    {
        std::lock_guard<Mutex> locker( mutex_ );
        if ( count_ > 0 )
        {
            T* const p = ( T* )buffer_ + this->mod( start_ + count_, cap_ );
            data = *p;
            return true;
        }
        return false;
    }

    bool pop_front( T& data )
    {
        std::lock_guard<Mutex> locker( mutex_ );
        if ( count_ > 0 )
        {
            --count_;
            T* const p = ( T* )buffer_ + start_;
            data = *p;
            ( ( T* )buffer_ + start_ )->~T();
            start_ = this->mod( start_ + 1, cap_ );
            return true;
        }
        return false;
    }

    bool pop_back( T& data )
    {
        std::lock_guard<Mutex> locker( mutex_ );
        if ( count_ > 0 )
        {
            --count_;
            T* const p = ( T* )buffer_ + this->mod( start_ + count_, cap_ );
            data = *p;
            p->~T();
            return true;
        }
        return false;
    }

    void clear() 
    {
        std::lock_guard<Mutex> locker( mutex_ );
        for (size_t i = 0; i < count_; ++i) 
        {
            ( (T*)buffer_ + this->mod(start_ + i, cap_) )->~T();
        }
        count_ = 0;
        start_ = 0;
    }

    bool empty() const
    {
        std::lock_guard<Mutex> locker( mutex_ );
        return count_ == 0;
    }

    size_t size() const
    {
        std::lock_guard<Mutex> locker( mutex_ );
        return count_;
    }

PRIVATE:

    void expand()
    {
        auto new_sz  = cap_ * 2;
        auto new_buf = std::calloc( new_sz, sizeof( T ) );
        auto tmp_buf = ( T* )new_buf;
        for ( size_t i = 0; i < count_; ++i ) 
        {
            T* p = ( ( T* )buffer_ + this->mod( start_ + i, cap_ ) );
            new ( tmp_buf ) T( *p );
            p->~T();
            tmp_buf += 1;
        }
        free( buffer_ );
        buffer_ = new_buf;
        cap_    = new_sz;
        start_  = 0;
    }

    size_t mod( size_t off, size_t cap )
    {
        while ( off >= cap )
        {
            off -= cap;
        }
        return off;
    }

PRIVATE:

    size_t start_;
    size_t count_;
    size_t cap_;
    void*  buffer_;
    mutable Mutex  mutex_;
};

template<typename T>
class BoundedQueue< T, NullMutex >
{
PUBLIC:

    BoundedQueue()
        : start_( 0 )
        , count_( 0 )
        , cap_( 0 )
        , buffer_( nullptr )
    {

    }
    
    ~BoundedQueue()
    {
        clear();
        ::free( buffer_ );
        buffer_ = nullptr;
    }

    void init( size_t max_count )
    {
        cap_    = max_count;
        buffer_ = std::calloc( max_count, sizeof( T ) );
    }

    void push_front( T const& data )
    {
        if ( count_ >= cap_ )
        {
            expand();
        }
        
        start_ = start_ ? ( start_ - 1 ) : ( cap_ - 1 );
        ++count_;
        new ( ( T* )buffer_ + start_ ) T( data );
    }

    void push_back( T const& data )
    {
        if ( count_ >= cap_ )
        {
            expand();
        }

        new ( ( T* )buffer_ + this->mod( start_ + count_, cap_ ) ) T( data );
        ++count_;
    }

    bool front( T& data )
    {
        if( count_ > 0 )
        {
            T* const p = ( T* )buffer_ + start_;
            data = *p;
            return true;
        }
        return false;
    }

    bool back( T& data )
    {
        if ( count_ > 0 )
        {
            T* const p = ( T* )buffer_ + this->mod( start_ + count_, cap_ );
            data = *p;
            return true;
        }
        return false;
    }

    bool pop_front( T& data )
    {
        if ( count_ > 0 )
        {
            --count_;
            T* const p = ( T* )buffer_ + start_;
            data = *p;
            ( ( T* )buffer_ + start_ )->~T();
            start_ = this->mod( start_ + 1, cap_ );
            return true;
        }
        return false;
    }

    bool pop_back( T& data )
    {
        if ( count_ > 0 )
        {
            --count_;
            T* const p = ( T* )buffer_ + this->mod( start_ + count_, cap_ );
            data = *p;
            p->~T();
            return true;
        }
        return false;
    }

    void clear() 
    {
        for (size_t i = 0; i < count_; ++i) 
        {
            ( (T*)buffer_ + this->mod(start_ + i, cap_) )->~T();
        }
        count_ = 0;
        start_ = 0;
    }

    bool empty() const
    {
        return count_ == 0;
    }

    size_t size() const
    {
        return count_;
    }

PRIVATE:

    void expand()
    {
        auto new_sz  = cap_ * 2;
        auto new_buf = std::calloc( new_sz, sizeof( T ) );
        auto tmp_buf = ( T* )new_buf;
        for ( size_t i = 0; i < count_; ++i ) 
        {
            T* old = ( (T*)buffer_ + this->mod( start_ + i, cap_ ) );
            new ( tmp_buf ) T( *old );
            old->~T();
            tmp_buf += 1;
        }
        free( buffer_ );
        buffer_ = new_buf;
        cap_    = new_sz;
        start_  = 0;
    }

    size_t mod( size_t off, size_t cap )
    {
        while ( off >= cap )
        {
            off -= cap;
        }
        return off;
    }

PRIVATE:

    size_t start_;
    size_t count_; 
    size_t cap_;
    void*  buffer_;
};

NAMESPACE_TARO_END
