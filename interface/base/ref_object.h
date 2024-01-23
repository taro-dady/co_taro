
#pragma once

#include "base/base.h"
#include <atomic>

NAMESPACE_TARO_BEGIN

struct RefObject {
PUBLIC:
    RefObject() 
        : ref_( 1 ) {
    }

    virtual ~RefObject() = default;
    
    void increment_ref() {
        ++ref_;
    }

    void decrement_ref() {
        if ( --ref_ == 0 ) {
            delete this;
        }
    }

PRIVATE:
    TARO_NO_COPY( RefObject );

PRIVATE:
    std::atomic_int32_t ref_;
};

template<typename T>
typename std::enable_if< std::is_base_of< RefObject, T >::value, void >::type
increment_ref( T* ref ) {
    ref->increment_ref();
}

template<typename T>
typename std::enable_if< !std::is_base_of< RefObject, T >::value, void >::type
increment_ref( T* ) {
}

template<typename T>
typename std::enable_if< std::is_base_of< RefObject, T >::value, void >::type
decrement_ref( T* ref ) {
    ref->decrement_ref();
}

template<typename T>
typename std::enable_if< !std::is_base_of< RefObject, T >::value, void >::type
decrement_ref( T* ) {
}

NAMESPACE_TARO_END
