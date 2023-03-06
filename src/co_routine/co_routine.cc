
#include "base/utils/id_alloc.h"
#include "co_routine/impl/co_routine_impl.h"
#include <atomic>

NAMESPACE_TARO_RT_BEGIN

static std::atomic_bool init_flag( false ); 
static CoRoutineAttr co_attr;
static IdAllocator id_alloc;
thread_local CoRoutineSPtr curr_co_rt;

extern void sched_co( CoRoutineSPtr co );
extern void init_async_log();

extern bool has_co_routine()
{
    return !id_alloc.empty();
}

static void co_handle( void* arg )
{
    CoRoutineImpl* impl = ( CoRoutineImpl* )arg;
    impl->func_();
    impl->state_ = eCoRoutineStateFinish;
    ctx_yield( impl->ctx_ );
}

CoRoutineImpl::CoRoutineImpl()
    : cid_( -1 )
    , ctx_( nullptr )
    , prio_( ePriorityMid )
    , state_( eCoRoutineStateReady )
{
    memset( &st_, 0, sizeof( st_ ) );
}

void CoRoutineImpl::resume( CoRoutineSPtr dst )
{
    TARO_ASSERT( dst != nullptr );
    curr_co_rt = dst;
    dst->resume();
}

void CoRoutineImpl::destroy( CoRoutine* co )
{
    delete co;
}

void CoRoutineImpl::reset_curr_co()
{
    curr_co_rt.reset();
}

CoRoutine::CoRoutine()
    : impl_( new CoRoutineImpl )
{

}

CoRoutine::~CoRoutine()
{
printf("CoRoutine::~CoRoutine %s\n", impl_->name_.c_str());

    for ( auto& func : impl_->exit_func_ )
    {
        func();
    }
    impl_->exit_func_.clear();
    
    id_alloc.free_id( impl_->cid_ );
    if ( impl_->ctx_ != nullptr )
    {
        ctx_destroy( impl_->ctx_ );
    }
    delete impl_;
}

CoRoutineSPtr CoRoutine::create( const char* name,
                                 std::function<void()> const& func,
                                 bool sched_auto,
                                 EPriority prio,
                                 uint32_t stack_size )
{
    if ( !STRING_CHECK( name ) || !func )
    {
        RT_ERROR << "invalid parameter";
        return nullptr;
    }

    auto cid = id_alloc.alloc_id();
    if ( cid < 0 )
    {
        RT_ERROR << "alloc co routine identity failed";
        return nullptr;
    }

    std::shared_ptr<CoRoutine> co_rt( new CoRoutine, CoRoutineImpl::destroy );
    CoRoutineImpl& impl = *co_rt->impl_;
    impl.name_ = name;
    impl.func_ = func;
    impl.prio_ = prio;
    impl.cid_  = cid;

    if ( stack_size < co_attr.stack_min_size )
    {
        stack_size = co_attr.stack_min_size;
    }
    
    if ( stack_size > co_attr.stack_max_size )
    {
        stack_size = co_attr.stack_max_size;
    }
    impl.st_.stack    = ( char* )::malloc( stack_size );
    impl.st_.stack_sz = stack_size;

    if ( sched_auto )
    {
        sched_co( co_rt );
    }
    return co_rt;
}

void CoRoutine::init( CoRoutineAttr const& attr )
{
    bool expr = false;
    if ( !init_flag.compare_exchange_strong( expr, true ) )
    {
        return;
    }
    
    co_attr = attr;
    id_alloc.init( co_attr.max_co_routine );
    init_async_log();
}

CoRoutineSPtr CoRoutine::current()
{
    return curr_co_rt;
}

const char* CoRoutine::name() const
{
    return impl_->name_.c_str();
}

int32_t CoRoutine::cid() const
{
    CoRoutineSPtr tr;
    tr.reset();
    return impl_->cid_;
}

ECoRoutineState CoRoutine::state() const
{
    return impl_->state_;
}

EPriority CoRoutine::priority() const
{
    return impl_->prio_;
}

void CoRoutine::yield()
{
    impl_->state_ = eCoRoutineStateSuspend;
    ctx_yield( impl_->ctx_ );
}

void CoRoutine::resume()
{
    if ( eCoRoutineStateReady == impl_->state_ )
    {
        impl_->ctx_ = make_ctx( co_handle, ( void* )impl_, &impl_->st_ );
    }
    impl_->state_ = eCoRoutineStateRunning;
    ctx_resume( impl_->ctx_ );
}

void CoRoutine::on_exit( std::function<void()> const& func )
{
    if ( func )
    {
        impl_->exit_func_.push_back( func );
    }
}

NAMESPACE_TARO_RT_END
