
#pragma once

#include "base/base.h"
#include <functional>

NAMESPACE_TARO_BEGIN

template<class F>
struct AutoCycle
{
PUBLIC: // function

    AutoCycle()
        : call_( true )
    {

    }

    AutoCycle( F const& f )
        : recycle_( f )
        , call_( true )
    {

    }

    ~AutoCycle()
    {
        if ( call_ && recycle_ )
        {
            recycle_();
        }
    }

    void set( F const& f )
    {
        recycle_ = f;
    }

    AutoCycle& operator=( bool call )
    {
        call_ = call;
        return *this;
    }

PRIVATE: // variable

    F    recycle_; 
    bool call_;       
};
using Defer = AutoCycle< std::function<void()> >;

NAMESPACE_TARO_END
