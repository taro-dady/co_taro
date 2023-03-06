#pragma once

#include "base/utils/utils.h"
#include "base/utils/assert.h"
#include <sstream>
#include <functional>

NAMESPACE_TARO_BEGIN

// 入参解析器
class TARO_DLL_EXPORT ArgParser
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    ArgParser();

    /**
    * @brief 析构函数
    */
    ~ArgParser();

    /**
    * @brief 解析参数
    *
    * @param[in] argc 参数数量
    * @param[in] argv 参数列表
    */
    bool parse( int32_t argc, char* argv[] );

    /**
    * @brief 添加解析器
    *
    * @param[in] key      参数的标识
    * @param[in] callback 参数解析器
    */
    bool add_parser( const char* key, std::function<void(const char*)> const& callback );

PRIVATE: // 私有函数

    TARO_NO_COPY( ArgParser );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

NAMESPACE_TARO_END

// 参数解析的单例
using ArgParserInst = taro::Singleton<taro::ArgParser>;

template<typename Func>
struct PreCaller
{
    PreCaller( Func const& func )
    {
        func();
    }
};

/**
* @brief 解析进程参数
*/
inline void parse_main_args( int32_t argc, char* argv[] )
{
    ArgParserInst::instance().parse( argc, argv );
}

// 创建默认解析器
template<class T>
inline std::function<void( const char* )> create_parser( T& value )
{
    return [&]( const char* str )
    {
        try
        {
            std::stringstream ss;
            ss << str;
            ss >> value;
        }
        catch( ... )
        {
            // do nothing
        }
    };
}

// 创建默认解析器
template<class T>
inline std::function<void( const char* )> create_default_parser( T& value, T def_value )
{
    value = def_value;
    return [&, def_value]( const char* str )
    {
        try
        {
            std::stringstream ss;
            ss << str;
            ss >> value;
        }
        catch( const std::exception& )
        {
            value = def_value;
        }
    };
}

#define TARO_ARG_EXTERN( type, name ) extern type taro_arg_##name

#define TARO_ARG_DECL( type, name ) \
type taro_arg_##name;\
static PreCaller< std::function<void()> > pre_caller_taro_arg_##name( \
    [](){ ArgParserInst::instance().add_parser( #name, create_parser( taro_arg_##name ) ); } )

#define TARO_ARG_DEF_DECL( type, name, def_value ) \
type taro_arg_##name;\
static PreCaller< std::function<void()> > pre_caller_taro_arg_##name( \
    [](){ ArgParserInst::instance().add_parser( #name, create_default_parser( taro_arg_##name, def_value ) ); } )

/**
 * @brief 解析参数
 * 
 * @param[in] argc 参数数量
 * @param[in] argv 参数列表
*/
inline void parse_args( int32_t argc, char* argv[] )
{
    auto ret = ArgParserInst::instance().parse( argc, argv );
    TARO_ASSERT( ret );
}
