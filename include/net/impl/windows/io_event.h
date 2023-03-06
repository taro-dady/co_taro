
#pragma once

#include "net/defs.h"
#include "co_routine/inc.h"
#include "co_routine/impl/common.h"
#include <winsock2.h>
#include <tchar.h>
#include <windows.h>

NAMESPACE_TARO_NET_BEGIN

// 网络事件类型
enum ENetEventType
{
    eNetEvtRead      = 1 << 0,
    eNetEvtWrite     = 1 << 1,
    eNetEvtException = 1 << 2,
    eNetEvtTimeout   = 1 << 3,
    eNetEvtInvalid   = 0,
};

// 事件类型
struct IocpEvent : PUBLIC OVERLAPPED
{
    /**
    * @brief 构造函数
    */
    IocpEvent( ENetEventType type, uint32_t ms = 0 )
        : valid( true )
        , evt_id( -1 )
        , bytes( 0 )
        , type( type )
    {
        memset( ( OVERLAPPED* )this, 0, sizeof( OVERLAPPED ) );
    }

    /**
    * @brief 析构函数
    */
    virtual ~IocpEvent() = default;

    /**
    * @brief 事件处理函数
    */
    void on_event()
    {
        if ( evt_id >= 0 )
        {
            // 事件在超时之前发生则删除超时处理函数
            rt::get_scheduler()->remove_event( evt_id );
            evt_id = -1;
        }

        if( co != nullptr )
        {
            rt::get_scheduler()->push( co );
            co.reset();
        }
    }

    /**
    * @brief 等待事件发生
    */
    void wait( uint32_t ms = 0 )
    {
        co = rt::CoRoutine::current();
        if ( ms == 0 )
        {
            co->yield();
            return;
        }

        auto timeout_func = [this]()
        {
            // 事件等待超时处理
            evt_id = -1;
            type   = eNetEvtTimeout;
            rt::get_scheduler()->push( co );
            co     = nullptr;
        };
        evt_id = rt::get_scheduler()->add_event( timeout_func, ms );
        co->yield();
    }

    bool              valid;  // 事件是否有效，超时后该事件失效不要处理
    int32_t           evt_id; // 超时事件id
    uint32_t          bytes;  // 事件产生的数据长度
    ENetEventType     type;   // 事件类型
    rt::CoRoutineSPtr co;     // 当前协程对象
};

// 获取IOCP
extern HANDLE get_poller();

struct IocpTypeEvent : PUBLIC IocpEvent
{
    IocpTypeEvent( ENetEventType type, char* buffer = nullptr , size_t size = 0 )
        : IocpEvent( type )
    {
        wrap.buf = ( char* )buffer;
        wrap.len = ( ULONG )size;
    }

    ~IocpTypeEvent()
    {
        //printf( "~IocpTypeEvent\n" );
    }

    WSABUF wrap;
};

struct IocpAcceptEvent : PUBLIC IocpEvent
{
    explicit IocpAcceptEvent()
        : IocpEvent( eNetEvtRead )
        , client( INVALID_SOCKET )
    {
        memset( buf, 0, sizeof( buf ) );
    }

    SOCKET client;
    char   buf[( sizeof( SOCKADDR_IN ) + 16 ) * 2];
};

struct IocpUdpWriteEvent : PUBLIC IocpEvent
{
    IocpUdpWriteEvent( char* buffer, size_t size, IpAddr const& addr )
        : IocpEvent( eNetEvtWrite )
    {
        wrap.buf = ( char* )buffer;
        wrap.len = ( ULONG )size;
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_port = htons( addr.port );
        remote_addr.sin_addr.s_addr = inet_addr( addr.ip.c_str() );
    }

    ~IocpUdpWriteEvent()
    {
        //printf( "~IocpUdpWriteEvent\n" );
    }

    WSABUF      wrap;
    SOCKADDR_IN remote_addr;
};

struct IocpUdpReadEevent : PUBLIC IocpEvent
{
    IocpUdpReadEevent( char* buffer, size_t size )
        : IocpEvent( eNetEvtRead )
        , remote_len( sizeof( remote_addr ) )
    {
        wrap.buf = ( char* )buffer;
        wrap.len = ( ULONG )size;
    }

    ~IocpUdpReadEevent()
    {
        //printf( "~IocpUdpReadEevent\n" );
    }

    WSABUF      wrap;
    int32_t     remote_len;   //存储数据来源IP地址长度
    SOCKADDR_IN remote_addr;  //存储数据来源IP地址
};

NAMESPACE_TARO_NET_END
