
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
    /**
     * @brief constructor
    */
    DoubleList() 
        : count_( 0 ) {
        head_ = new DoubleListHead();
        tail_ = head_;
    }

    /**
     * @brief destructor
    */
    ~DoubleList() {
        DoubleListHead* item = tail_;
        while( item != head_ ) {
            item->prev->next = item->next;
            if ( item->next )
                item->next->prev = item->prev;
            decrement_ref( item );
            item = item->prev;
        }
        delete head_;
        head_ = tail_ = 0;
        count_ = 0;
    }

    /**
     * @brief push new node
    */
    void push( DoubleListHead* node ) {
        LockGuard g( mutex_ );
        tail_->next = node;
        node->prev = tail_;
        tail_ = node;
        increment_ref( node );
        ++count_;
    }

    /**
     * @brief pop front node
    */
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

    /**
     * @brief check if list empty
    */
    bool empty() const {
        LockGuard g( mutex_ );
        return count_ == 0;
    }

    /**
     * @brief get list size
    */
    uint32_t size() const {
        LockGuard g( mutex_ );
        return count_;
    }

    /**
     * @brief remove list
    */
    void remove_all( DoubleListHead**head, DoubleListHead** tail, uint32_t& count ) {
        LockGuard g( mutex_ );
        *head = head_->next;
        *tail = tail_;
        count = count_;

        head_->next = 0;
        head_->prev = 0;
        tail_ = head_;
        count_ = 0;
    }

    /**
     * @brief append list
    */
    void append( DoubleListHead* head, DoubleListHead* tail, uint32_t count ) {
        LockGuard g( mutex_ );
        tail_->next = head;
        head->prev = tail_;
        tail_ = tail;
        count_ += count;
    }
    
PRIVATE:
    using LockGuard = typename std::conditional<ThreadSafe, std::lock_guard<Mutex>,  LockFreeGuard<Mutex> >::type;
    TARO_NO_COPY( DoubleList );

    uint32_t count_;
    mutable Mutex mutex_;
    DoubleListHead* head_;
    DoubleListHead* tail_;
};

NAMESPACE_TARO_END
