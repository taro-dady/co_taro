
#include "base/error_no.h"
#include "base/utils/assert.h"
#include "base/memory/shm_que.h"
#include "base/system/file_locker.h"
#include "base/utils/arg_expander.h"
#include "base/system/share_memory.h"
#include <functional>

NAMESPACE_TARO_BEGIN

/* 
内存结构
--------------------------------------------------------------
元数据  | 数据块信息0 ... 数据块信息N  | 数据块0 ... 数据块N
--------------------------------------------------------------
*/

// 元数据区
struct ShmQueMeta
{
    uint32_t block_num;   // 队列中数据块数量
    uint32_t block_size;  // 队列中数据块大小
    uint32_t write_pos;   // 当前的写入位置
    uint32_t seq;         // 数据序列号，单调递增
};

// 数据块信息
struct ShmQueBlock
{
    uint32_t seq;   // 当前数据的序列号
    uint32_t size;  // 保存数据的大小
};

struct MetaReadGuard
{
    MetaReadGuard( FileLocker& lock )
        : lock_( lock )
    {
        lock.read_lock( 0, 1 );
    }

    ~MetaReadGuard()
    {
        lock_.unlock( 0, 1 );
    }

    FileLocker& lock_;
};

struct MetaWriteGuard
{
    MetaWriteGuard( FileLocker& lock )
        : lock_( lock )
    {
        lock.write_lock( 0, 1 );
    }

    ~MetaWriteGuard()
    {
        lock_.unlock( 0, 1 );
    }

    FileLocker& lock_;
};

struct BlockReadGuard
{
    BlockReadGuard( FileLocker& lock, int32_t index )
        : lock_( lock )
        , range_( index + 1 )
    {
        lock.read_lock( ( size_t )range_, ( size_t )range_ + 1 );
    }

    ~BlockReadGuard()
    {
        lock_.unlock( ( size_t )range_, ( size_t )range_ + 1 );
    }

    FileLocker& lock_;
    int32_t     range_;
};

struct BlockWriteGuard
{
    BlockWriteGuard( FileLocker& lock, int32_t index )
        : lock_( lock )
        , range_( index + 1 )

    {
        lock.write_lock( ( size_t )range_, ( size_t )range_ + 1 );
    }

    ~BlockWriteGuard()
    {
        lock_.unlock( ( size_t )range_, ( size_t )range_ + 1 );
    }

    FileLocker& lock_;
    int32_t     range_;
};

/**
* @brief 获取块信息
*/
inline ShmQueBlock* get_block( void* base, int32_t index )
{
    ShmQueBlock* first = ( ShmQueBlock* )( ( ShmQueMeta* )base + 1 );
    return first + index;
} 

/**
* @brief 计算下个节点的位置
*/
inline int32_t next_block( int32_t index, uint32_t num )
{
    return ( index + 1 ) % num; 
}

/**
* @brief 获取内存的地址
*/
inline char* get_block_mem( void* base, int32_t index )
{
    ShmQueMeta* head = ( ShmQueMeta* )base;
    return ( char* )base + sizeof( ShmQueMeta ) + head->block_num * sizeof( ShmQueBlock ) + index * head->block_size;
}

/**
* @brief 计算队列所需要的内存大小
*/
inline size_t caculate_mem_size( uint32_t block_num, uint32_t block_sz )
{
    return sizeof( ShmQueMeta ) + block_num * sizeof( ShmQueBlock ) + block_num * block_sz;
}

/**
* @brief 初始化共享内存信息区
*/
static void init_meta( void* buffer, uint32_t block_num, uint32_t block_sz )
{
    auto head = ( ShmQueMeta* )buffer;
    head->block_num  = block_num;
    head->block_size = block_sz; 
    head->write_pos  = 0;
    head->seq        = 0;
    memset( (char*)( head + 1 ), 0, sizeof( ShmQueBlock ) * block_num );
}

static bool create_mem( const char* name, uint32_t block_num, uint32_t block_sz, ShmMemory& mem )
{
    if ( !mem.create( name, caculate_mem_size( block_num, block_sz ) ) )
    {
        return false;
    }
    else
    {
        init_meta( mem.get(), block_num, block_sz );
    }
    return true;
}

struct ShmQueWriter::Impl
{
    bool       valid_;
    uint32_t   block_num_;
    uint32_t   block_size_;
    ShmMemory  mem_;
    FileLocker locker_;
};

ShmQueWriter::ShmQueWriter()
    : impl_( new Impl )
{
    impl_->valid_ = false;
}

ShmQueWriter::~ShmQueWriter()
{
    delete impl_;
}

int32_t ShmQueWriter::init( const char* path, const char* file_locker, uint32_t block_num, uint32_t block_sz, bool create )
{
    if ( !STRING_CHECK( path, file_locker ) || block_num == 0 || 0 == block_sz )
    {
        return TARO_ERR_INVALID_ARG;
    }

    if ( impl_->locker_.init( file_locker ) != TARO_OK )
    {
        printf( "init file locker failed %s\n", file_locker );
        return TARO_ERR_FAILED;
    }

    if ( create )
    {
        if ( !create_mem( path, block_num, block_sz, impl_->mem_ ) )
        {
            impl_->valid_ = false;
            printf( "create memory failed %s\n", path );
            return TARO_ERR_FAILED;
        }

        impl_->valid_      = true;
        impl_->block_num_  = block_num;
        impl_->block_size_ = block_sz;
        init_meta( impl_->mem_.get(), block_num, block_sz );
        return TARO_OK;
    }

    if ( !impl_->mem_.open( path ) )
    {
        if ( !create_mem( path, block_num, block_sz, impl_->mem_ ) )
        {
            impl_->valid_ = false;
            printf( "create memory failed %s\n", path );
            return TARO_ERR_FAILED;
        }
        init_meta( impl_->mem_.get(), block_num, block_sz );
    }

    impl_->valid_      = true;
    impl_->block_num_  = block_num;
    impl_->block_size_ = block_sz;
    return TARO_OK;
}

bool ShmQueWriter::valid() const
{
    return impl_->valid_;
}

int32_t ShmQueWriter::write( char* buffer, int32_t bytes )
{
    if( nullptr == buffer || bytes <= 0 )
    {
        return TARO_ERR_INVALID_ARG;
    }

    if ( !impl_->valid_ )
    {
        return TARO_ERR_INVALID_RES;
    }

    ShmQueMeta* meta  = ( ShmQueMeta* )impl_->mem_.get();
    uint32_t  seq     = 0;
    uint32_t  index   = 0;
    {
        MetaWriteGuard g( impl_->locker_ );
        seq             = meta->seq++;
        index           = meta->write_pos;
        meta->write_pos = next_block( meta->write_pos, impl_->block_num_ );
    }

    int32_t      wbytes = std::min<int32_t>( bytes, ( int32_t )impl_->block_size_ );
    ShmQueBlock* block  = get_block( ( void* )meta, index );
    {
        void* buf = get_block_mem( ( void* )meta, index );
        BlockWriteGuard g( impl_->locker_, index );
        block->seq  = seq;
        block->size = wbytes;
        memcpy( buf, buffer, wbytes );
    }
    return wbytes;
}

struct ShmQueReader::Impl
{
    // 数据读取逻辑处理 
    bool read( std::function<int32_t()> const& read_block )
    {
        if ( !valid_ )
        {
            return false;
        }

        auto new_seq = [&]()
        { 
            MetaReadGuard g( locker_ ); 
            return ( ( ShmQueMeta* )mem_.get() )->seq;
        };

        while( is_newer( new_seq(), read_seq_ ) )
        {
            auto ret = read_block();
            if ( ret == TARO_ERR_STATE )
            {
                auto head = ( ShmQueMeta* )mem_.get();
                MetaReadGuard g( locker_ ); 
                if ( head->seq > 0 )
                {
                    // 前移一格，指向最新的数据
                    read_pos_ = ( head->write_pos == 0 ) ? ( head->block_num - 1 ) : ( head->write_pos - 1 );
                    read_seq_ = head->seq - 1;
                }
                else
                {
                    read_pos_ = head->write_pos;
                    read_seq_ = head->seq;
                }
                continue;
            }
            else if( ret == TARO_OK )
            {
                ++read_seq_;
                read_pos_ = next_block( read_pos_, block_num_ );
            }
            break;
        }
        return true;
    }

    bool       valid_;
    uint32_t   block_num_;
    uint32_t   block_size_;
    ShmMemory  mem_;
    FileLocker locker_;
    uint32_t   read_pos_;
    uint32_t   read_seq_;
};

ShmQueReader::ShmQueReader()
    : impl_( new Impl )
{
    impl_->valid_ = false;
}

ShmQueReader::~ShmQueReader()
{
    delete impl_;
}

int32_t ShmQueReader::init( const char* path, const char* file_locker, uint32_t block_num, uint32_t block_sz )
{
    if ( !STRING_CHECK( path, file_locker ) || block_num == 0 || 0 == block_sz )
    {
        return TARO_ERR_INVALID_ARG;
    }

    if ( impl_->locker_.init( file_locker ) != TARO_OK )
    {
        printf( "init file locker failed %s\n", file_locker );
        return TARO_ERR_FAILED;
    }

    if ( !impl_->mem_.open( path ) )
    {
        printf( "open share memroy failed %s\n", path );
        return TARO_ERR_FAILED;
    }

    auto head          = ( ShmQueMeta* )impl_->mem_.get();
    impl_->block_num_  = head->block_num;
    impl_->block_size_ = head->block_size;

    MetaReadGuard g( impl_->locker_ );
    if ( head->seq > 0 )
    {
        // 前移一格，指向最新的数据
        impl_->read_pos_ = ( head->write_pos == 0 ) ? ( head->block_num - 1 ) : ( head->write_pos - 1 );
        impl_->read_seq_ = head->seq - 1;
    }
    else
    {
        impl_->read_pos_ = head->write_pos;
        impl_->read_seq_ = head->seq;
    }
    return TARO_OK;
}

bool ShmQueReader::valid() const
{
    return impl_->valid_;
}

DynPacketSPtr ShmQueReader::read()
{
    DynPacketSPtr packet;
    auto read_block = [&]()
    {
        ShmQueBlock* block = get_block( impl_->mem_.get(), impl_->read_pos_ );
        TARO_ASSERT( block != nullptr );

        BlockReadGuard g( impl_->locker_, impl_->read_pos_ );
        if ( block->size == 0 )
        {
            printf("block empty\n");
            return TARO_ERR_INVALID_RES;
        }

        if ( block->seq != impl_->read_seq_ )
        {
            printf( "block seq mismatch %d %d\n", block->seq, impl_->read_seq_ );
            return TARO_ERR_STATE;
        }
        packet = create_default_packet( block->size );
        memcpy( packet->buffer(), get_block_mem( impl_->mem_.get(), impl_->read_pos_ ), block->size );
        packet->resize( block->size );
        return TARO_OK;
    };

    ( void )impl_->read( read_block );
    return packet;
}

bool ShmQueReader::read( std::function< void( char*, size_t ) > const& func )
{
    if( !func )
    {
        set_errno( TARO_ERR_INVALID_ARG );
        return false;
    }

    auto read_block = [&, func]()
    {
        ShmQueBlock* block = get_block( impl_->mem_.get(), impl_->read_pos_ );
        TARO_ASSERT( block != nullptr );

        BlockReadGuard g( impl_->locker_, impl_->read_pos_ );
        if( block->size == 0 )
        {
            printf( "block empty\n" );
            return TARO_ERR_INVALID_RES;
        }

        if( block->seq != impl_->read_seq_ )
        {
            printf( "block seq mismatch %d %d\n", block->seq, impl_->read_seq_ );
            return TARO_ERR_STATE;
        }

        func( get_block_mem( impl_->mem_.get(), impl_->read_pos_ ), block->size );
        return TARO_OK;
    };
    return impl_->read( read_block );
}

NAMESPACE_TARO_END
