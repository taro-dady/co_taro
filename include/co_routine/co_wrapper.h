
#pragma once

#include "base/utils/utils.h"
#include "base/system/thread.h"
#include "co_routine/co_routine.h"
#include "base/utils/arg_expander.h"

NAMESPACE_TARO_RT_BEGIN

// 线程与协程的属性类型
enum ECoAttrType
{
    eCoAttrTypeName,         // 名称
    eCoAttrTypeStackSize,    // 栈大小
    eCoAttrTypePriority,     // 优先级
    eCoAttrTypeCpuAffinity,  // cpu亲和性，线程有效
};

// 属性选项
template<ECoAttrType>
struct CoAttrOpt;

#define CO_ATTR_OPT_DEC( ATTR_TYPE, ARG_TYPE )\
template<>\
struct CoAttrOpt<ATTR_TYPE>\
{\
    CoAttrOpt( ARG_TYPE const& a ) : arg( a ) {}\
    ARG_TYPE arg;\
}
CO_ATTR_OPT_DEC( eCoAttrTypeStackSize, size_t );
CO_ATTR_OPT_DEC( eCoAttrTypeName, std::string );
CO_ATTR_OPT_DEC( eCoAttrTypePriority, EPriority );

template<>
struct CoAttrOpt<eCoAttrTypeCpuAffinity>
{
    template<typename... Args>
    CoAttrOpt( Args&&... args )
    {
        append_container( cpus, std::forward<Args>( args )... );
    }
    std::vector<int32_t> cpus;
};

// 协程运行对象
class TARO_DLL_EXPORT CoRunner
{
PUBLIC:

    /**
    * @brief 构造函数
    * 
    * @param[in] name 协程名称 
    */
    CoRunner( std::string const& name );

    /**
    * @brief 析构函数
    */
    ~CoRunner();

    /**
    * @brief 设置协程的名称
    * 
    * @param[in] opt 协程属性 
    */
    CoRunner& operator,( CoAttrOpt<eCoAttrTypeName> const& opt );

    /**
    * @brief 设置协程的堆栈大小
    * 
    * @param[in] opt 协程属性 
    */
    CoRunner& operator,( CoAttrOpt<eCoAttrTypeStackSize> const& opt );

    /**
    * @brief 设置协程的优先级
    * 
    * @param[in] opt 协程属性 
    */
    CoRunner& operator,( CoAttrOpt<eCoAttrTypePriority> const& opt );

    /**
    * @brief 设置协程的运行函数
    * 
    * @param[in] f 运行函数 
    */
    CoRunner& operator<<( std::function<void()> const& f );

PRIVATE: // 私有变量
    
    struct Impl;
    Impl* impl_;
};

// 线程对象
class TARO_DLL_EXPORT CoThread
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] name 协程名称 
    */
    CoThread( std::string const& name = "" );

    /**
    * @brief 析构函数
    */
    ~CoThread();

    /**
    * @brief 设置线程的堆栈大小
    * 
    * @param[in] opt 属性 
    */
    CoThread& attr( CoAttrOpt<eCoAttrTypeStackSize> const& opt );

    /**
    * @brief 设置线程的名称
    * 
    * @param[in] opt 属性 
    */
    CoThread& attr( CoAttrOpt<eCoAttrTypeName> const& opt );

    /**
    * @brief 设置线程的优先级
    * 
    * @param[in] opt 属性 
    */
    CoThread& attr( CoAttrOpt<eCoAttrTypePriority> const& opt );

    /**
    * @brief 设置线程的CPU亲和性
    * 
    * @param[in] opt 属性 
    */
    CoThread& attr( CoAttrOpt<eCoAttrTypeCpuAffinity> const& opt );

    /**
    * @brief 添加协程
    * 
    * @param[in] name 协程处理函数
    * @param[in] name 协程名称 
    */
    CoThread& append( std::function<void()> const& co_func, std::string const& co_name );

    /**
    * @brief 设置协程的堆栈大小
    * 
    * @param[in] opt 协程属性 
    */
    CoThread& co_arg( CoAttrOpt<eCoAttrTypeStackSize> const& opt );

    /**
    * @brief 设置协程的名称
    * 
    * @param[in] opt 协程属性 
    */
    CoThread& co_arg( CoAttrOpt<eCoAttrTypeName> const& opt );

    /**
    * @brief 设置协程的优先级
    * 
    * @param[in] opt 协程属性 
    */
    CoThread& co_arg( CoAttrOpt<eCoAttrTypePriority> const& opt );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

NAMESPACE_TARO_RT_END

#define opt_stack(size)   taro::rt::CoAttrOpt<taro::rt::eCoAttrTypeStackSize>(size)
#define opt_name(n)       taro::rt::CoAttrOpt<taro::rt::eCoAttrTypeName>(n)
#define opt_prior(prior)  taro::rt::CoAttrOpt<taro::rt::eCoAttrTypePriority>(prior)
#define opt_affinity(...) taro::rt::CoAttrOpt<taro::rt::eCoAttrTypeCpuAffinity>(__VA_ARGS__)

// 启动协程
#define co_run taro::rt::CoRunner( TRACE_NAME ) << 

// 启动另一个带有协程的线程
#define co_task taro::rt::CoThread( TRACE_NAME )
