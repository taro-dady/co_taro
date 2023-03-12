
#include "base/error_no.h"
#include "base/memory/pakcet_list.h"
#include <list>
#include <string.h>

NAMESPACE_TARO_BEGIN

struct PacketList::Impl
{
    uint32_t total_bytes_;
    std::list<DynPacketSPtr> packets_;
};

PacketList::PacketList()
    : impl_( new Impl )
{
    impl_->total_bytes_ = 0;
}

PacketList::PacketList( PacketList&& other )
    : impl_( other.impl_ )
{
    other.impl_ = nullptr;
}

PacketList::~PacketList()
{
    if ( impl_ == nullptr )
        return;
    delete impl_;
}

int32_t PacketList::append( DynPacketSPtr const& p )
{
    if( nullptr == p || nullptr == p->buffer() || 0 == p->size() )
    {
        return TARO_ERR_INVALID_ARG;
    }

    impl_->total_bytes_ += ( uint32_t )p->size();
    impl_->packets_.emplace_back( p );
    return TARO_OK;
}

int32_t PacketList::size() const
{
    return ( int32_t )impl_->total_bytes_;
}

int32_t PacketList::try_read( uint8_t* buf, uint32_t size, uint32_t offset )
{
    if( nullptr == buf || 0 == size || offset >= impl_->total_bytes_ )
    {
        return TARO_ERR_INVALID_ARG;
    }

    uint32_t bytes = 0, temp = 0;
    auto it = impl_->packets_.begin();
    for( ; it != impl_->packets_.end(); ++it )
    {
        if( offset <= bytes + ( *it )->size() )
        {
            temp = ( offset - bytes );
            break;
        }
        bytes += ( uint32_t )( *it )->size();
    }

    int32_t read_bytes = 0;
    for( ; it != impl_->packets_.end(); ++it )
    {
        uint32_t to_read_bytes = size - read_bytes;
        uint32_t no_read_bytes = ( uint32_t )( *it )->size() - temp;

        if( to_read_bytes <= no_read_bytes )
        {
            memcpy( buf + read_bytes, ( char* )( *it )->buffer() + temp, to_read_bytes );
            read_bytes += to_read_bytes;
            return read_bytes;
        }

        memcpy( buf + read_bytes, ( char* )( *it )->buffer() + temp, no_read_bytes );
        read_bytes += no_read_bytes;
        temp = 0;
    }
    return read_bytes;
}

DynPacketSPtr PacketList::read( uint32_t size )
{
    DynPacketSPtr p;
    if( size > impl_->total_bytes_ )
    {
        return p;
    }

    auto it = impl_->packets_.begin();
    if( ( *it )->size() == ( uint32_t )size )
    {
        p = *it;
        impl_->packets_.erase( it );
        impl_->total_bytes_ -= size;
        return p;
    }

    p = create_default_packet( size );
    if( nullptr == p )
    {
        return p;
    }

    int32_t read_bytes = 0;
    while( it != impl_->packets_.end() )
    {
        uint32_t to_read_bytes = size - read_bytes;
        uint32_t no_read_bytes = ( uint32_t )( *it )->size();
        if( to_read_bytes == no_read_bytes )
        {
            p->append( ( *it )->buffer(), to_read_bytes );
            impl_->total_bytes_ -= to_read_bytes;
            it = impl_->packets_.erase( it );
            break;
        }
        else if( to_read_bytes < no_read_bytes )
        {
            p->append( ( *it )->buffer(), to_read_bytes );
            impl_->total_bytes_ -= to_read_bytes;
            uint32_t head_offset = ( *it )->get_head_offset();
            head_offset += to_read_bytes;
            ( *it )->set_head_offset( head_offset );
            ( *it )->resize( ( uint32_t )( *it )->size() - to_read_bytes );
            break;
        }

        p->append( ( *it )->buffer(), ( uint32_t )( *it )->size() );

        read_bytes          += ( uint32_t )( *it )->size();
        impl_->total_bytes_ -= ( uint32_t )( *it )->size();
        it                   = impl_->packets_.erase( it );
    }
    return p;
}

int32_t PacketList::consume( uint32_t size )
{
    if( size >= impl_->total_bytes_ )
    {
        auto ret = impl_->total_bytes_;
        reset();
        return ret;
    }

    int32_t read_bytes = 0;
    auto it = impl_->packets_.begin();
    while( it != impl_->packets_.end() )
    {
        uint32_t to_read_bytes = size - read_bytes;
        uint32_t no_read_bytes = ( uint32_t )( *it )->size();
        if( to_read_bytes == no_read_bytes )
        {
            impl_->total_bytes_ -= to_read_bytes;
            it = impl_->packets_.erase( it );
            break;
        }
        else if( to_read_bytes < no_read_bytes )
        {
            impl_->total_bytes_ -= to_read_bytes;
            uint32_t head_offset = ( *it )->get_head_offset();
            head_offset += to_read_bytes;
            ( *it )->set_head_offset( head_offset );
            ( *it )->resize( ( uint32_t )( *it )->size() - to_read_bytes );
            break;
        }

        read_bytes += ( uint32_t )( *it )->size();
        impl_->total_bytes_ -= ( uint32_t )( *it )->size();
        it = impl_->packets_.erase( it );
    }
    return size;
}

bool PacketList::search( const char* str, uint32_t offset, uint32_t& pos, StringCompareFunc cb )
{
    if( nullptr == str || 0 == strlen( str ) )
    {
        return false;
    }

    uint32_t size = ( uint32_t )strlen( str );
    std::string read_str( size, 0 );
    while( try_read( ( uint8_t* )read_str.c_str(), size, offset ) == ( int32_t )size )
    {
        bool equal = false;
        if( cb )
        {
            equal = cb( read_str, std::string( str ) );
        }
        else
        {
            equal = ( read_str == str );
        }

        if( equal )
        {
            pos = offset;
            return true;
        }
        ++offset;
    }
    return false;
}

void PacketList::reset()
{
    impl_->packets_.clear();
    impl_->total_bytes_ = 0;
}

NAMESPACE_TARO_END
