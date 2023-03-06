
#include "net/net_work.h"
#include "net/impl/windows/io_event.h"
#include "net/impl/windows/tcp_client_raw.h"
#include "net/impl/windows/tcp_server_raw.h"
#include <ws2tcpip.h>
#include <IPHlpApi.h>

#pragma comment(lib,"IPHlpApi.lib")
#pragma comment(lib, "ws2_32.lib")

NAMESPACE_TARO_NET_BEGIN

#define EXIT_EVT ( ( OVERLAPPED* )( ( __int64 )0 ) )
thread_local HANDLE thrd_iocp = nullptr;

std::function<TcpClientSPtr( SSLContext* )> ssl_create_cli_func;
std::function<TcpServerSPtr( SSLContext* )> ssl_create_svr_func;

/**
* @brief IOCP的事件处理
* 
* @param[in] poller 事件轮询器
* @param[in] ms     事件等待超时时间
*/
extern void worker_poll( HANDLE poller, int32_t ms );

// 基于IOCP的事件轮询对象
class IocpEventPoller : PUBLIC rt::EventPoller
{
PUBLIC:

    /**
    * @brief 构造函数
    * 
    * @param[in] poller 事件轮询器
    */
    IocpEventPoller( HANDLE poller )
        : poller_( poller )
    {

    }

    /**
    * @brief 等待指定时间
    * 
    * @param[in] ms 等待时间(毫秒)
    */
    virtual void wait( int32_t ms ) override
    {
        worker_poll( poller_, ms );
    }

    /**
    * @brief 唤醒
    */
    virtual void wakeup() override
    {
        PostQueuedCompletionStatus( poller_, 0, 0, EXIT_EVT );
    }

PRIVATE:

    HANDLE poller_;
};

HANDLE get_poller()
{
    if ( nullptr == thrd_iocp )
    {
        // 替换调度器事件轮询对象
        thrd_iocp = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 1 );
        rt::get_scheduler()->set_evt_poller( std::make_shared<IocpEventPoller>( thrd_iocp ) );
    }
    return thrd_iocp;
}

void worker_poll( HANDLE poller, int32_t ms )
{
    DWORD        dwIoSize        = 0;
    void*        lpCompletionKey = nullptr;
    LPOVERLAPPED lpOverlapped    = nullptr;

    BOOL ret = GetQueuedCompletionStatus( poller, &dwIoSize, ( PULONG_PTR )&lpCompletionKey, ( LPOVERLAPPED* )&lpOverlapped, ms );
    if ( lpOverlapped == EXIT_EVT )
    {
        return;
    }

    auto io_evt = static_cast< IocpEvent* >( lpOverlapped );
    if( io_evt->type == eNetEvtTimeout )
    {
        delete io_evt;
        return;
    }

    if ( !ret )
    {
        io_evt->type = eNetEvtException;
    }
    io_evt->bytes = dwIoSize;
    io_evt->on_event();
}

void start_network()
{
    WSADATA data;
    ( void )WSAStartup( MAKEWORD( 2, 2 ), &data );
}

void stop_network()
{
    WSACleanup();
}

TcpClientSPtr create_tcp_cli( SSLContext* ctx )
{
    if( ctx )
    {
        TARO_ASSERT( ssl_create_cli_func, "init tls" );
        return ssl_create_cli_func( ctx );
    }
    return std::make_shared<TcpClientRaw>();
}

TcpServerSPtr create_tcp_svr( SSLContext* ctx )
{
    if( ctx )
    {
        TARO_ASSERT( ssl_create_svr_func, "init tls" );
        return ssl_create_svr_func( ctx );
    }
    return std::make_shared<TcpServerRaw>();
}

std::vector<NetInterface> get_net_if()
{
    std::vector<NetInterface> info;

    PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO;
    unsigned long ulSize = sizeof( IP_ADAPTER_INFO );
    int32_t nRet = GetAdaptersInfo( pIpAdapterInfo, &ulSize );
    if( ERROR_BUFFER_OVERFLOW == nRet )
    {       
        delete pIpAdapterInfo;

        pIpAdapterInfo = ( PIP_ADAPTER_INFO ) new BYTE[ulSize];
        nRet = GetAdaptersInfo( pIpAdapterInfo, &ulSize );
        if( ERROR_SUCCESS != nRet )
        {
            if( pIpAdapterInfo != NULL )
            {
                delete [] pIpAdapterInfo;
            }
            return info;
        }
    }

    char szMacAddr[20];

    PIP_ADAPTER_INFO pIterater = pIpAdapterInfo;
    while( pIterater )
    {
        NetInterface adapter;

        adapter.name = pIterater->AdapterName; /// AdapterName
        sprintf_s(  szMacAddr, 20, "%02X-%02X-%02X-%02X-%02X-%02X",
                    pIterater->Address[0],
                    pIterater->Address[1],
                    pIterater->Address[2],
                    pIterater->Address[3],
                    pIterater->Address[4],
                    pIterater->Address[5] );

        adapter.mac = szMacAddr; /// MAC Address

        PIP_ADDR_STRING pIpAddr = &pIterater->IpAddressList;
        while( pIpAddr )
        {
            adapter.ip.emplace_back( pIpAddr->IpAddress.String ); /// Ip
            adapter.mask.emplace_back( pIpAddr->IpMask.String );  /// IpMask
            pIpAddr = pIpAddr->Next;
        }

        PIP_ADDR_STRING pGateAwayList = &pIterater->GatewayList;
        while ( pGateAwayList )
        {
            adapter.gateway.emplace_back( pGateAwayList->IpAddress.String );  //// NG
            pGateAwayList = pGateAwayList->Next;
        }
        pIterater = pIterater->Next;

        info.emplace_back( adapter );
    }

    if( pIpAdapterInfo )
    {
        delete [] pIpAdapterInfo;
    }
    return info;
}

NAMESPACE_TARO_NET_END
