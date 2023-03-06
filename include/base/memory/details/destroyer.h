#pragma once

#include "base/base.h"

NAMESPACE_TARO_BEGIN

namespace detail 
{
    struct Destroyer
    {
        virtual void release() = 0;
        virtual ~Destroyer() {}
    };
    
    template<class T>
    struct NormalDestroyer : public Destroyer
    {
        explicit NormalDestroyer( T* p )
            : p_( p )
        {

        }

        virtual void release() override
        {
            if( p_ != nullptr )
                delete p_;
        }

        T* p_;
    };

    template<class T, class D>
    struct UserDestroyer : public Destroyer
    {
        UserDestroyer( T* p, D const& del )
            : p_( p )
            , del_( del )
        {

        }

        virtual void release() override
        {
            if( p_ != nullptr )
                del_( p_ );
        }

        T* p_;
        D  del_;
    };

    template<typename T>
    inline void delete_obj( T* obj )
    {
        delete obj;
    }
    
    template<typename T>
    inline void delete_nothing( T* ){}
}

NAMESPACE_TARO_END