
#pragma once

#include "base/ref_object.h"
#include <mutex>
#include <type_traits>

NAMESPACE_TARO_BEGIN

template<typename Mutex>
struct LockFreeGuard{
    LockFreeGuard( Mutex& ){}
};

struct LockFreeMutex {
    void lock(){}
    void unlock(){}
};

struct DoubleListHead {
    DoubleListHead() 
        : prev( 0 ), next( 0 ) {
    }
    virtual ~DoubleListHead() = default;

    DoubleListHead *prev;
    DoubleListHead *next;
};

template<typename Mutex, bool ThreadSafe = true>
class DoubleList {
PUBLIC:

    DoubleList() 
        : count_( 0 ) {
        head_ = new DoubleListHead();
        tail_ = head_;
    }

    ~DoubleList() {
        DoubleListHead* item = tail_->prev;
        while( item != head_ ) {
            item->prev->next = item->next;
            if ( item->next != nullptr )
                item->next->prev = item->prev;
            decrement_ref( item );
            item = item->prev;
        }
        delete head_;
        head_ = tail_ = 0;
        count_ = 0;
    }

    void push( DoubleListHead* node ) {
        LockGuard g( mutex_ );
        tail_->next = node;
        node->prev = tail_;
        tail_ = node;
        increment_ref( node );
        ++count_;
    }

    bool pop( DoubleListHead** node ) {
        LockGuard g( mutex_ );
        if ( tail_ == head_ ) {
            return false;
        }

        auto item = head_->next;
        item->prev->next = item->next;
        if ( item->next != nullptr )
            item->next->prev = item->prev;
        *node = item;
        decrement_ref( item );
        --count_;
        return true;
    }

    bool empty() const {
        LockGuard g( mutex_ );
        return count_ == 0;
    }

    uint32_t size() const {
        LockGuard g( mutex_ );
        return count_;
    }

    void move_list( DoubleListHead** head, DoubleListHead** tail ) {
        LockGuard g( mutex_ );
        ( *head )->next = head_->next;
        head_->next->prev = *head;
        *tail = tail_;
        head_->next = 0;
        tail_ = head_;
        count_ = 0;
    }

PRIVATE:
    using LockGuard = typename std::conditional<ThreadSafe, std::lock_guard<Mutex>,  LockFreeGuard<Mutex> >::type;

PRIVATE:
    TARO_NO_COPY( DoubleList );

PRIVATE:
    uint32_t count_;
    mutable Mutex mutex_;
    DoubleListHead* head_;
    DoubleListHead* tail_;
};

NAMESPACE_TARO_END
