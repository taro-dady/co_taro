
#include "base/error_no.h"
#include "base/utils/id_alloc.h"
#include "base/system/spin_locker.h"
#include <mutex>

NAMESPACE_TARO_BEGIN

struct IdAllocator::Impl
{
    Impl()
        : cnt_( 0 )
        , num_( 0 )
        , array_sz_( 0 )
        , last_sz_( 0 )
        , id_array_( 0 )
    {}

    SpinLocker mtx_;
    size_t     cnt_;
    size_t     num_;
    size_t     array_sz_;
    size_t     last_sz_;
    uint32_t*  id_array_;
};

IdAllocator::IdAllocator()
    : impl_( new Impl )
{
    
}

IdAllocator::~IdAllocator()
{
    free( impl_->id_array_ );
    delete impl_;
}

bool IdAllocator::empty() const
{
    std::lock_guard<SpinLocker> g( impl_->mtx_ );
    return 0 == impl_->cnt_;
}

void IdAllocator::init( size_t num )
{
    std::lock_guard<SpinLocker> g( impl_->mtx_ );
    if ( impl_->num_ > 0 )
    {
        return;
    }

    impl_->num_       = num;
    impl_->array_sz_  = num / sizeof( uint32_t );
    impl_->last_sz_   = num % sizeof( uint32_t );
    impl_->array_sz_ += impl_->last_sz_ ? 1 : 0;
    impl_->id_array_  = ( uint32_t* )malloc( sizeof( uint32_t ) * impl_->array_sz_ );
    memset( ( void* )impl_->id_array_, 0, num );
}

int32_t IdAllocator::alloc_id()
{
    TARO_ASSERT( impl_->id_array_ != NULL );

    constexpr uint32_t FULL = 0xFFFFFFFF;

    std::lock_guard<SpinLocker> g( impl_->mtx_ );
    if ( impl_->cnt_ >= impl_->num_ )
    {
        return TARO_ERR_FAILED;
    }

    for ( size_t i = 0; i < impl_->array_sz_; ++i )
    {
        if ( impl_->id_array_[i] == FULL )
            continue;
        
        uint32_t& v  = impl_->id_array_[i];
        uint32_t len = ( ( i + 1 ) == impl_->array_sz_ ) ? ( uint32_t )impl_->last_sz_ : 32; 
        for( uint32_t j = 0; j < len; ++j )
        {
            if ( !( v & ( 1 << j ) ) )
            {
                ++impl_->cnt_;
                v |= ( 1 << j );
                return ( int32_t )( i * 32 + j );
            }
        }
    }
    TARO_ASSERT( 0 );
    return TARO_ERR_INVALID_ARG;
}

int32_t IdAllocator::free_id( int32_t id )
{
    if ( id < 0 && id >= ( int32_t )impl_->num_ )
    {
        return TARO_ERR_INVALID_ARG;
    }

    auto bit_index = id % 32;
    auto ar_index  = id / 32;

    std::lock_guard<SpinLocker> g( impl_->mtx_ );
    if ( !( impl_->id_array_[ar_index] & ( 1 << bit_index ) ) )
    {
        return TARO_ERR_INVALID_ARG;
    }
    --impl_->cnt_;
    impl_->id_array_[ar_index] &= ~( 1 << bit_index );
    return TARO_OK;
}

NAMESPACE_TARO_END
