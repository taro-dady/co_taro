
#pragma once

#include "co_routine/defs.h"

NAMESPACE_TARO_RT_BEGIN

// 协程栈
struct CoStack
{
    char*   stack;
    int32_t stack_sz;
};

// 协程上下文
struct CoContext;

/**
* @brief 创建线程上下文
*
* @param[in] func 协程处理函数
* @param[in] arg  协程参数
* @param[in] st   协程栈
*/
CoContext* make_ctx( void( *func )( void* ), void* arg, CoStack* st );

/**
* @brief 协程让出
*
* @param[in] ctx 将被切换的协程
*/
void ctx_yield( CoContext* ctx );

/**
* @brief 协程恢复
*
* @param[in] ctx 将被恢复的协程
*/
void ctx_resume( CoContext* ctx );

/**
* @brief 销毁协程上下文
*/
void ctx_destroy( CoContext* ctx );

NAMESPACE_TARO_RT_END
