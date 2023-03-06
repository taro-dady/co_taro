
#pragma once

#include "base/function/inc.h"
#include "co_routine/co_wrapper.h"
#include "co_routine/co_condition.h"
#include <type_traits>

NAMESPACE_TARO_RT_BEGIN

template<typename ReturnType>
struct CoAsyncItem
{
    Condition  cond;
    ReturnType ret;  // 结果
};

template<typename ReturnType>
using CoAsyncItemSPtr = std::shared_ptr< CoAsyncItem<ReturnType> >;

// 数据同步对象
template<typename ReturnType>
class CoFuture
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    CoFuture( CoAsyncItemSPtr<ReturnType> const& item = nullptr )
        : item_( item )
    {

    }

    /**
    * @brief 获取结果
    */
    ReturnType get_result()
    {
        item_->cond.cond_wait();
        return item_->ret;
    }

PRIVATE: // 私有变量

    CoAsyncItemSPtr<ReturnType> item_;
};

// 基于协程的异步任务
template<typename F>
class CoAsyncTask
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    CoAsyncTask( F proc, std::string const& name )
        : proc_( proc )
        , item_( std::make_shared< ItemType >() )
        , name_( name )
    {
        
    }

    /**
    * @brief 异步调用
    */
    CoFuture<typename FunctionTraits<F>::ResultType> async_call()
    {
        co_run std::bind
        ( 
            []( ItemSPtr item, F func )
            {
                item->ret = func();
                item->cond.notify_one();
            }, 
            item_, 
            proc_ 
        ), opt_name( name_ );
        
        return CoFuture<RetType>( item_ );
    }

PRIVATE: // 私有类型

    using RetType  = typename FunctionTraits<F>::ResultType;
    using ItemType = CoAsyncItem<typename FunctionTraits<F>::ResultType>;
    using ItemSPtr = std::shared_ptr<ItemType>;

PRIVATE: // 私有变量

    F           proc_;
    ItemSPtr    item_;
    std::string name_;
};

/**
* @brief 函数与异步任务的转换器
*/
struct CoAsyncTransfer
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    CoAsyncTransfer( std::string const& co_name )
        : co_name_( co_name )
    {

    }

    /**
    * @brief 异步调用函数
    */
    template<typename F, typename... Args>
    typename std::result_of<F( Args... )>::type
    operator<<( F f )
    {
        using FuncType = std::function< typename std::result_of<F( Args... )>::type(Args...) >;
        return CoAsyncTask<FuncType>( f, co_name_ ).async_call().get_result();
    }

PRIVATE: // 私有变量

    std::string co_name_;
};

NAMESPACE_TARO_RT_END

#define co_async taro::rt::CoAsyncTransfer( TRACE_NAME )<< 
