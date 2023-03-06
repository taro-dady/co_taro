
#pragma once

#include "base/base.h"
#include <memory>

NAMESPACE_TARO_BEGIN

enum EFunctionType
{
    eFunctionTypeNormal,
    eFunctionTypeSharedPtr,
    eFunctionTypeWeakPtr,
};

template<class R, class... Args>
class FunctionBase
{
PUBLIC:  // function

    FunctionBase( EFunctionType type )
        : type_( type )
    {

    }

    EFunctionType type() const
    {
        return type_;
    }

    virtual ~FunctionBase() {}
    virtual R operator()( Args... ) = 0;
    virtual bool equal( FunctionBase* ) const = 0;
    virtual bool valid() const = 0;

PRIVATE: /// variable

    EFunctionType type_;
};

template<class R, class... Args>
class NormalFunction : PUBLIC FunctionBase<R, Args...>
{
PUBLIC: // public

    explicit NormalFunction( R( *f )( Args... ) )
        : FunctionBase<R, Args...>( eFunctionTypeNormal )
        , function_( f )
    {
        TARO_ASSERT( function_ != nullptr );
    }

    virtual R operator()( Args... args ) final
    {
        return function_( args... );
    }

    virtual bool equal( FunctionBase<R, Args...>* f ) const final
    {
        auto function_ptr = dynamic_cast< NormalFunction* >( f );
        if ( nullptr == function_ptr )
        {
            throw std::exception( "function type error" );
        }
        return function_ == function_ptr->function_;
    }

    virtual bool valid() const final
    {
        return function_ != nullptr;
    }

private: // variable

    R( *function_ )( Args... );
};

template<class T, class R, class... Args>
class SharedPtrFunction : PUBLIC FunctionBase<R, Args...>
{
PUBLIC: // function

    SharedPtrFunction( R( T::* f )( Args... ), std::shared_ptr<T> const& obj )
        : FunctionBase<R, Args...>( eFunctionTypeSharedPtr )
        , function_( f )
        , class_inst_( obj )
    {
        TARO_ASSERT( function_ != nullptr && class_inst_ != nullptr );
    }

    virtual R operator()( Args... args ) final
    {
        return ( ( *class_inst_ ).*function_ )( args... );
    }

    virtual bool equal( FunctionBase<R, Args...>* f ) const final
    {
        auto ptr = dynamic_cast< SharedPtrFunction* >( f );
        if ( nullptr == ptr )
        {
            throw std::exception( "function type error" );
        }
        return ( function_ == ptr->function_ ) && ( class_inst_ == ptr->class_inst_ );
    }

    virtual bool valid() const final
    {
        return ( function_ != nullptr ) && ( class_inst_ != nullptr );
    }

private: // variable

    R( T::* function_ )( Args... );
    std::shared_ptr<T> class_inst_;
};

template<class T, class R, class... Args>
class WeakPtrFunction : PUBLIC FunctionBase<R, Args...>
{
PUBLIC:

    WeakPtrFunction( R( T::* f )( Args... ), std::weak_ptr<T> const& obj )
        : FunctionBase<R, Args...>( eFunctionTypeWeakPtr )
        , raw_pointer_( nullptr )
        , function_( f )
        , class_inst_( obj )
    {
        TARO_ASSERT( function_ != nullptr );

        auto class_inst_ptr = obj.lock();
        if ( function_ == nullptr || class_inst_ptr == nullptr )
        {
            throw std::exception( "weak pointer expired" );
        }
        raw_pointer_ = class_inst_ptr.get();
    }

    virtual R operator()( Args... args ) final
    {
        auto class_inst_sptr = class_inst_.lock();
        if ( class_inst_sptr == nullptr )
        {
            throw std::exception( "weak pointer expired" );
        }
        return ( ( *class_inst_sptr ).*function_ )( args... );
    }

    virtual bool equal( FunctionBase<R, Args...>* f ) const final
    {
        auto ptr = dynamic_cast< WeakPtrFunction* >( f );
        if ( function_ == nullptr && ptr == nullptr )
        {
            throw std::exception( "invalid parameter" );
        }
        return ( function_ == ptr->function_ ) && ( raw_pointer_ == ptr->raw_pointer_ );
    }

    virtual bool valid() const final
    {
        return ( function_ != nullptr ) && ( !class_inst_.expired() );
    }

private: // variable

    T* raw_pointer_;
    R( T::* function_ )( Args... );
    std::weak_ptr<T> class_inst_;
};

template<class T>
class Function {};

template<class R, class... Args>
class Function<R( Args... )>
{
PUBLIC: // function

    Function() = default;
    Function( std::nullptr_t ) {}

    explicit Function( R( *f )( Args... ) )
        : base_( new NormalFunction<R, Args...>( f ) )
    {
        TARO_ASSERT( base_ != nullptr );
    }

    template<class T>
    explicit Function( R( T::* f )( Args... ), std::shared_ptr<T> const& obj )
        : base_( new  SharedPtrFunction<T, R, Args...>( f, obj ) )
    {
        TARO_ASSERT( base_ != nullptr );
    }

    template<class T>
    explicit Function( R( T::* f )( Args... ), std::weak_ptr<T> const& obj )
        : base_( new  WeakPtrFunction<T, R, Args...>( f, obj ) )
    {
        TARO_ASSERT( base_ != nullptr );
    }

    R operator()( Args... args )
    {
        if ( base_ == nullptr )
        {
            throw std::exception( "invalid function" );
        }
        return ( *base_ )( args... );
    }

    bool operator==( Function const& r ) const
    {
        if ( this == &r )
        {
            return true;
        }

        if ( ( base_ == nullptr ) && ( r.base_ == nullptr ) )
        {
            return true;
        }

        if ( ( ( base_ == nullptr ) && ( r.base_ != nullptr ) )
          || ( ( base_ != nullptr ) && ( r.base_ == nullptr ) ) )
        {
            return false;
        }

        if ( base_->type() != r.base_->type() )
        {
            return false;
        }

        return base_->equal( r.base_.get() );
    }

    bool operator!=( Function const& r ) const
    {
        return !( *this == r );
    }

    operator bool() const
    {
        return base_ != nullptr && base_->valid();
    }

private: // variable

    std::shared_ptr< FunctionBase<R, Args...> > base_;
};

NAMESPACE_TARO_END
