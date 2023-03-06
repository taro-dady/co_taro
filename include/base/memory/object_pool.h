
#pragma once

#include "base/base.h"
#include "base/utils/assert.h"
#include <atomic>
#include <string.h>
#include <memory>

NAMESPACE_TARO_BEGIN

template<class T>
struct ObjNode
{
    T           data;
    ObjNode<T>* next;
};

template<class T>
class ObjectPool : PUBLIC std::enable_shared_from_this< ObjectPool<T> >
{
PUBLIC:

    ObjectPool( size_t count )
        : obj_begin_( ::calloc( count, sizeof( ObjNode<T> ) ) )
        , obj_end_( ( char* )obj_begin_ + count * sizeof( ObjNode<T> ) )
        , count_( count )
    {
        TARO_ASSERT( count_ > 0 && obj_begin_ != nullptr, "object is null" );
        init();
    }

    ~ObjectPool()
    {
        /// @brief 未析构使用的节点，可能会导致内存泄漏，不过考虑到对象池使用的都是单例，在程序退出时才会析构可不考虑析构问题
        if ( obj_begin_ != nullptr )
        {
            free( obj_begin_ );
            obj_begin_ = nullptr;
            obj_end_   = nullptr;
            count_     = 0;
        }
    }

    template<typename... Args>
    std::shared_ptr<T> get_object( Args&&... args )
    {
        auto node = alloc_node();
        if ( nullptr == node )
        {
            return nullptr;
        }

        new ( &node->data ) T( std::forward<Args>( args )... );
        auto self = this->shared_from_this();
        return std::shared_ptr<T>( &node->data, [self]( T* obj )
        {
            if ( nullptr == obj )
            {
                return;
            }
            obj->~T();
            self->free_node( reinterpret_cast< ObjNode<T>* >( obj ) );
        } );
    }

PRIVATE:

    ObjectPool( ObjectPool const& );
    ObjectPool& operator=( ObjectPool const& );

    ObjNode<T>* alloc_node()
    {
        ObjNode<T>* new_head;
        ObjNode<T>* old_head = head_.load( std::memory_order_acquire );
        do
        {
            if ( taro_likely( old_head == nullptr ) )
            {
                return nullptr;
            }
            new_head = old_head->next;
        } while ( !head_.compare_exchange_weak( old_head, 
                                                new_head, 
                                                std::memory_order_acq_rel,
                                                std::memory_order_acquire ) );
        return old_head;
    }

    void free_node( ObjNode<T>* node )
    {
        TARO_ASSERT( is_valid_node( node ), "node invalid" );

        auto old_head = head_.load( std::memory_order_acquire );
        node->next    = old_head;
        while( !head_.compare_exchange_weak( node->next, 
                                             node, 
                                             std::memory_order_acq_rel,
                                             std::memory_order_acquire ) );
    }

    void init()
    {
        auto obj  = ( ObjNode<T>* )obj_begin_;
        auto prev = obj;
        for ( size_t i = 1; i < count_; ++i )
        {
            obj[i].next = nullptr;
            prev->next  = &obj[i];
            prev        = &obj[i];
        }
        head_.store( obj );
    }

    bool is_valid_node( ObjNode<T>* node )
    {
        if ( node == nullptr || node < obj_begin_ || node >= obj_end_ )
        {
            return false;
        }
        return 0 == ( ( char* )node - ( char* )obj_begin_ ) % sizeof( ObjNode<T> );
    }

PRIVATE:

    void*                      obj_begin_;
    void*                      obj_end_;
    size_t                     count_;
    std::atomic< ObjNode<T>* > head_;
};

NAMESPACE_TARO_END
