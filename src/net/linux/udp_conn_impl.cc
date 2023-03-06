
#include "net/impl/linux/udp_conn_impl.h"

NAMESPACE_TARO_NET_BEGIN

UdpConnImpl::UdpConnImpl()
    : socket_( ::socket(AF_INET, SOCK_DGRAM, 0) )
    , co_evt_( nullptr )
    , group_setting_( NULL )
{

}

UdpConnImpl::~UdpConnImpl()
{
    ::close( socket_ );
    socket_ = TARO_INVALID_SOCKET;

    if ( nullptr != group_setting_ )
    {
        delete group_setting_;
        group_setting_ = nullptr;
    }
}

bool UdpConnImpl::bind( IpAddr const& addr, bool reuse )
{
    if ( !addr.valid() )
    {
        NET_ERROR << "invalid address";
        return false;
    }

    int32_t optval = reuse ? 1 : 0;
    if( 0 != setsockopt( socket_, SOL_SOCKET, SO_REUSEADDR, ( char* )&optval, sizeof( optval ) ) )
    {
        NET_ERROR << "set socket option failed. reuse:" << reuse;
        return false;
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons( addr.port );
    if ( ::inet_pton( AF_INET, addr.ip.c_str(), &server_addr.sin_addr.s_addr) == -1 )
    {
        NET_ERROR << "inet_pton error.";
        return false;
    }

    if ( ::bind( socket_, (struct sockaddr *)&server_addr, sizeof(struct sockaddr) ) == -1 )
    {
        NET_ERROR << "bind error!";
        return false;
    }
    return true;
}

int32_t UdpConnImpl::sendto( IpAddr const& remote, char* buffer, size_t bytes )
{
    if ( !remote.valid() || nullptr == buffer || 0 == bytes )
    {
        NET_ERROR << "parameter invalid";
        return TARO_ERR_INVALID_ARG;
    }

    if ( nullptr == co_evt_ )
    {
        co_evt_ = std::make_shared<CoNetEvent>( socket_ );
    }

    co_evt_->wait( eNetEvtWrite );
    if ( eNetEvtWrite != co_evt_->evt_type() )
    {
        NET_ERROR << "exception happen type:" << co_evt_->evt_type();
        return TARO_ERR_DISCONNECT;
    }

    struct sockaddr_in sock_addr;
    sock_addr.sin_family      = AF_INET;
    sock_addr.sin_port        = htons( remote.port );
    sock_addr.sin_addr.s_addr = ::inet_addr( remote.ip.c_str() );
    memset( sock_addr.sin_zero, 0x00, 8 );
    uint32_t tolen = sizeof( struct sockaddr );

    auto ret = ::sendto( socket_, ( char* )buffer, bytes, 0, ( struct sockaddr * )&sock_addr, tolen );
    if( ret <= 0 )
    {
        NET_ERROR << "sendto failed, remote:" << remote;
        return TARO_ERR_FAILED;
    }
    return ret;
}

int32_t UdpConnImpl::recvfrom( char* buffer, size_t bytes, IpAddr& remote, int32_t ms )
{
    if( nullptr == buffer || 0 == bytes )
    {
        NET_ERROR << "parameter invalid";
        return TARO_ERR_INVALID_ARG;
    }

    if ( nullptr == co_evt_ )
    {
        co_evt_ = std::make_shared<CoNetEvent>( socket_ );
    }

    co_evt_->wait( eNetEvtRead, ms > 0 ? ms : 0 );
    if ( eNetEvtTimeout == co_evt_->evt_type() )
    {
        NET_DEBUG << "receive time out";
        return TARO_ERR_TIMEOUT;
    }
    else if( eNetEvtRead != co_evt_->evt_type() )
    {
        NET_TRACE << "socket event exception";
        return TARO_ERR_FAILED;
    }

    socklen_t src_len = sizeof( struct sockaddr_in );
    struct sockaddr_in src_addr;
    int32_t ret = ::recvfrom( socket_, ( char * )buffer, bytes, 0, ( sockaddr* )&src_addr, &src_len );
    if( ret > 0 )
    {
        remote = IpAddr( inet_ntoa( src_addr.sin_addr ), ntohs( src_addr.sin_port ) );
        return ret;
    }
    return TARO_ERR_FAILED;
}

bool UdpConnImpl::set_group_loop( bool loop )
{
    int32_t opt = loop ? 1 : 0;
    if( 0 != setsockopt( socket_, IPPROTO_IP, IP_MULTICAST_LOOP, &opt, sizeof(opt) ) )
    {
        NET_ERROR << "set socket loop option failed. loop:" << loop;
        return false;
    }
    return true;
}

bool UdpConnImpl::join_group( uint16_t local_port, std::string const& group )
{
    if ( !UdpConnImpl::bind( IpAddr( "0.0.0.0", local_port ), true ) )
    {
        NET_ERROR << "bind error, port:" << local_port;
        return false;
    }

    if ( group_setting_ != nullptr )
    {
        delete group_setting_;
    }
    group_setting_ = new struct ip_mreqn;

    memset( group_setting_, 0, sizeof( struct ip_mreqn ) );
    inet_pton( AF_INET, group.c_str(), &group_setting_->imr_multiaddr.s_addr );
    inet_pton( AF_INET, "0.0.0.0",     &group_setting_->imr_address.s_addr );
    if( 0 != setsockopt( socket_, IPPROTO_IP, IP_ADD_MEMBERSHIP, group_setting_, sizeof(struct ip_mreqn) ) )
    {
        delete group_setting_;
        group_setting_ = nullptr;
        NET_ERROR << "set socket add member option failed. group:" << group;
        return false;
    }
    return true;
}

bool UdpConnImpl::leave_group()
{
    if ( group_setting_ == nullptr )
    {
        NET_ERROR << "no group joined";
        return false;
    }

    if( 0 != setsockopt( socket_, IPPROTO_IP, IP_DROP_MEMBERSHIP, group_setting_, sizeof(struct ip_mreqn) ) )
    {
        NET_ERROR << "set socket option failed.";
        return false;
    }

    delete group_setting_;
    group_setting_ = nullptr;
    return true;
}

UdpConnSPtr UdpConnImpl::copy()
{
    auto conn = std::make_shared<UdpConnImpl>();
    conn->socket_ = ::dup( socket_ );
    return conn;
}

UdpConnSPtr create_udp_conn()
{
    return std::make_shared<UdpConnImpl>();
}

NAMESPACE_TARO_NET_END
