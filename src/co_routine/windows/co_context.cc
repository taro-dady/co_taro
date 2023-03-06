
#include "co_routine/impl/co_context.h"
#include <stddef.h>
#include <string.h>

NAMESPACE_TARO_RT_BEGIN

using fcontext_t = void*;

struct transfer_t 
{
    fcontext_t fctx;
    void* data;
};

extern "C" transfer_t jump_fcontext( fcontext_t const to, void* vp );
extern "C" fcontext_t make_fcontext( void* sp, size_t size, void ( *fn )( transfer_t ) );

struct CoContext
{
    fcontext_t ctx;
    CoStack*   st;

	// 用户调用
    void( *f )( void* );
    void* arg;

    char used;
} __declspec( align( 1 ) );

// 线程上下文
thread_local fcontext_t thrd_ctx = nullptr;

/**
* @brief 协程处理函数
*/
static void ctx_handle( transfer_t t )
{
	CoContext* ctx = ( CoContext* )t.data;
    thrd_ctx = t.fctx;
    ctx->f( ctx->arg );
}

CoContext* make_ctx( void( *func )( void* ), void* arg, CoStack* st )
{
    auto co_ctx = new CoContext;
    memset( co_ctx, 0, sizeof( CoContext ) );

    void* sp = st->stack + st->stack_sz;
    co_ctx->ctx = make_fcontext( sp, st->stack_sz, ctx_handle );
    co_ctx->st  = st;
    co_ctx->f   = func;
    co_ctx->arg = arg;
    return co_ctx;
}

void ctx_yield( CoContext* )
{
    jump_fcontext( thrd_ctx, NULL );
}

void ctx_resume( CoContext* co_ctx )
{
    if( !co_ctx->used )
    {
        co_ctx->used = 1;
        co_ctx->ctx  = jump_fcontext( co_ctx->ctx, co_ctx ).fctx;
    }
    else
    {
        co_ctx->ctx = jump_fcontext( co_ctx->ctx, NULL ).fctx;
    }
}

void ctx_destroy( CoContext* ctx )
{
	delete ctx;
}

NAMESPACE_TARO_RT_END
