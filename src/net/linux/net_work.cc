
#include "net/net_work.h"
#include "net/impl/linux/tcp_client_raw.h"
#include "net/impl/linux/tcp_server_raw.h"
#include <string.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <unistd.h>

NAMESPACE_TARO_NET_BEGIN

std::function<TcpClientSPtr( SSLContext* )> ssl_create_cli_func;
std::function<TcpServerSPtr( SSLContext* )> ssl_create_svr_func;

void start_network(){}
void stop_network(){}

TcpClientSPtr create_tcp_cli( SSLContext* ctx )
{
    if ( ctx )
    {
        TARO_ASSERT( ssl_create_cli_func, "init tls" );
        return ssl_create_cli_func( ctx );
    }
    return std::make_shared<TcpClientRaw>();
}

TcpServerSPtr create_tcp_svr( SSLContext* ctx )
{
    if ( ctx )
    {
        TARO_ASSERT( ssl_create_svr_func, "init tls" );
        return ssl_create_svr_func( ctx );
    }
    return std::make_shared<TcpServerRaw>();
}

/**
* @brief 获取本机的网络接口
*/
std::vector<NetInterface> get_host_interfaces()
{
    struct sockaddr_in *sin;
    struct ifreq ifr;
    memset( &ifr, 0, sizeof( ifr ) );
    std::vector<NetInterface> info;

    int socket_fd;
    if( ( socket_fd = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 )
    {
        return info;
    }

    struct ifaddrs * ifAddrStruct = nullptr;
    void * tmpAddrPtr = nullptr;

    getifaddrs( &ifAddrStruct );

    while( ifAddrStruct != nullptr )
    {
        if( ifAddrStruct->ifa_addr->sa_family == AF_INET )
        {
            NetInterface netCardInfo;

            // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr = &( ( struct sockaddr_in * )ifAddrStruct->ifa_addr )->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop( AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN );

            netCardInfo.name = ifAddrStruct->ifa_name;  /// NetCardName
            netCardInfo.ip.emplace_back( addressBuffer ); /// ip

            strcpy( ifr.ifr_name, ifAddrStruct->ifa_name );
            memset( &sin, 0, sizeof( sin ) );

            if( ioctl( socket_fd, SIOCGIFHWADDR, &ifr ) != -1 )
            {
                char mac[32];
                sin = ( struct sockaddr_in* )&ifr.ifr_netmask;
                sprintf( mac, "%02x:%02x:%02x:%02x:%02x:%02x",
                    ( unsigned char )ifr.ifr_netmask.sa_data[0],
                    ( unsigned char )ifr.ifr_netmask.sa_data[1],
                    ( unsigned char )ifr.ifr_netmask.sa_data[2],
                    ( unsigned char )ifr.ifr_netmask.sa_data[3],
                    ( unsigned char )ifr.ifr_netmask.sa_data[4],
                    ( unsigned char )ifr.ifr_netmask.sa_data[5] );
                netCardInfo.mac = mac; /// Mac
            }

            if( ioctl( socket_fd, SIOCGIFNETMASK, &ifr ) != -1 )
            {
                unsigned long mask_val = *(unsigned long*)&ifr.ifr_netmask.sa_data[2];
                netCardInfo.mask.emplace_back( inet_ntoa(*(in_addr*)&mask_val) );
            }
            info.emplace_back( netCardInfo ); /// push
        }
        ifAddrStruct = ifAddrStruct->ifa_next;
    }

    freeifaddrs( ifAddrStruct );
    ::close( socket_fd );
    return info;
}

NAMESPACE_TARO_NET_END
