
#include "co_routine/impl/co_context.h"
#include <stdio.h>
#include <cstdlib>
#include <cstring>

NAMESPACE_TARO_RT_BEGIN

#if defined __aarch64__
constexpr size_t REGISTERS_SIZE = 160;
#else
constexpr size_t REGISTERS_SIZE = 56;
#endif

extern "C" 
{
    extern void ctx_swap( void**, void** ) asm( "ctx_swap" );
};

struct CoContext
{
    CoContext()
        : sp( nullptr )
        , st( nullptr )
    {}

    char*    sp;
    CoStack* st;
};

thread_local char* co_main_sp = nullptr;

// 参考 Apollo的routine实现
// https://github.com/ApolloAuto/apollo/blob/master/cyber/croutine/detail/routine_context.cc
//  The stack layout looks as follows:
//
//              +------------------+
//              |      Reserved    |
//              +------------------+
//              |  Return Address  |   f1
//              +------------------+
//              |        RDI       |   arg
//              +------------------+
//              |        R12       |
//              +------------------+
//              |        R13       |
//              +------------------+
//              |        ...       |
//              +------------------+
// ctx->sp  =>  |        RBP       |
//              +------------------+
CoContext* make_ctx( void( *func )( void* ), void* arg, CoStack* st )
{
    auto ctx = new CoContext;
    ctx->st  = st;
    ctx->sp  = st->stack + st->stack_sz - 2 * sizeof( void* ) - REGISTERS_SIZE;
    std::memset( ctx->sp, 0, REGISTERS_SIZE );
#ifdef __aarch64__
    char *sp = st->stack + st->stack_sz - sizeof( void * );
#else
    char *sp = st->stack + st->stack_sz - 2 * sizeof( void * );
#endif
    *reinterpret_cast<void **>( sp ) = reinterpret_cast<void *> ( func );
    sp -= sizeof(void *);
    *reinterpret_cast<void **>( sp ) = const_cast<void *>( arg );
    return ctx;
}

void ctx_yield( CoContext* ctx )
{
    ctx_swap( reinterpret_cast<void**>( &ctx->sp ), reinterpret_cast<void**>( &co_main_sp ) );
}

void ctx_resume( CoContext* ctx )
{
    ctx_swap( reinterpret_cast<void**>( &co_main_sp ), reinterpret_cast<void**>( &ctx->sp ) );
}

void ctx_destroy( CoContext* ctx )
{
	delete ctx;
}

NAMESPACE_TARO_RT_END
