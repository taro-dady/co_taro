
#pragma once

#include "base/system/file_sys.h"
#include "base/utils/arg_expander.h"
#include <vector>

NAMESPACE_TARO_BEGIN

// 进程对象
class TARO_DLL_EXPORT Process
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    Process();

    /**
    * @brief 构造函数
    */
    Process( Process&& other );

    /**
    * @brief 析构函数
    */
    ~Process();

    /**
    * @brief 当前进程的进程号
    */
    static int32_t cur_pid();

    /**
    * @brief 启动进程
    * 
    * @param[in] entry 启动文件路径
    * @param[in] args  启动参数
    */
    template<typename... Args>
    bool start( const char* entry, Args... args )
    {
        if ( !STRING_CHECK( entry ) 
          || !FileSystem::check_file( entry ) )
        {
            printf( "entry not exist\n" );
            return false;
        }

        // 参数拼接
        std::vector<std::string> arg;
        arg.emplace_back( entry );
        type_to_str_expand
        (
            [&]( const char* name ){ arg.emplace_back( name ); }, 
            std::forward<Args>( args )... 
        );

        std::vector<const char*> cmd;
        for( auto& one : arg )
        {
            cmd.emplace_back( one.c_str() );
        }
        cmd.emplace_back( nullptr );
        return create( entry, ( char* const* )&cmd[0] );
    }

    /**
    * @brief 杀死进程 
    */
    bool kill();

    /**
    * @brief 获取进程号 
    */
    int32_t pid() const;

PRIVATE: // 私有函数

    TARO_NO_COPY( Process );

    /**
    * @brief 启动进程
    * 
    * @param[in] entry 启动文件路径
    * @param[in] args  启动参数
    */
    bool create( const char* entry, char* const* cmd );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

NAMESPACE_TARO_END
