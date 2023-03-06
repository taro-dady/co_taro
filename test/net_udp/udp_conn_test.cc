
#include "net/net_work.h"
#include "net/udp_conn.h"
#include "co_routine/inc.h"
#include "base/system/process.h"
#include "base/system/file_sys.h"
#include <string.h>

using namespace taro;
using namespace taro::rt;
using namespace taro::net;

USING_NAMESPACE_TARO

void udp_conn_sender()
{
    co_run []()
    {
        auto conn = net::create_udp_conn();
        while( true )
        {
            const char* req = "hello server 1";
            conn->sendto( IpAddr( "127.0.0.1", 22000 ), ( char* )req, strlen( req ) );

            IpAddr remote;
            char buf[128] = { 0 };
            NET_WARN << "start receive";
            auto ret = conn->recvfrom( buf, sizeof( buf ), remote, 2000 );
            if ( ret > 0 )
            {
                NET_WARN << std::string(buf, ret) << " remote:" << remote;
            }
            else
            {
                if (ret == TARO_ERR_TIMEOUT)
                {
                    NET_WARN << "timeout";
                }
                else
                {
                    NET_WARN << "receive error";
                }
            }
            co_wait(1000); 
        }
    };

    rt::co_loop();
}

void udp_conn_receiver()
{
    co_run []()
    {
        auto conn = net::create_udp_conn();
        TARO_ASSERT( conn->bind( net::IpAddr( "127.0.0.1", 22000 ) ) );
        while( true )
        {
            IpAddr remote;
            char buf[128] = { 0 };
            NET_WARN << "start receive";
            auto ret = conn->recvfrom( buf, sizeof( buf ), remote );
            if ( ret > 0 )
            {
                std::string resp( buf, ret );
                NET_WARN << "receive:" << resp << " remote:" << remote << std::endl;
                resp += " response";
                conn->sendto( remote, ( char* )resp.c_str(), resp.length() );  
            }
            else
            {
                if (ret == TARO_ERR_TIMEOUT)
                {
                    NET_WARN << "timeout";
                }
                else
                {
                    NET_WARN << "receive error";
                }
            }
        }
    };

    rt::co_loop();
}

void udp_conn_loop_recv()
{
    uint16_t port = 20000;
    const char* group = "239.0.0.10";
    co_run [=]()
    {
        auto conn = net::create_udp_conn();
        conn->set_group_loop( true );
        if ( !conn->join_group( port, group ) )
        {
            NET_WARN << "join group failed";
            return;
        }

        int cnt = 5;
        while( true )
        {
            IpAddr remote;
            char buf[128] = { 0 };
            auto ret = conn->recvfrom( buf, sizeof( buf ), remote );
            if ( ret > 0 )
            {
                std::cout << std::string(buf, ret) << " remote:" << remote << " local pid:" << Process::cur_pid() << std::endl;
                if ( 0 == --cnt )
                {
                    std::cout << "leave group" << std::endl;
                    conn->leave_group();
                } 
            }
        }
    };

    rt::co_loop();
}

void udp_conn_loop_send()
{
    uint16_t port = 20000;
    const char* group = "239.0.0.10";
    co_run [=]()
    {
        auto conn = net::create_udp_conn();
        conn->set_group_loop( true );
        int cnt = 0;
        while( 1 )
        {
            std::stringstream ss;
            ss << "udp group cast " << cnt << " pid:" << Process::cur_pid();
            printf( "send udp %d pid %d\n", cnt++, Process::cur_pid() );
            conn->sendto( IpAddr( group, port ), ( char* )ss.str().c_str(), ss.str().length() );
            co_wait(2000);
        }
    };

    rt::co_loop();
}

int main( int argc, char** argv )
{
    if ( argc < 2 )
    {
        perror( "parameter error\n" ); 
        exit( 0 );
    }

    rt::CoRoutine::init();
    net::start_network();
    
    switch ( atoi( argv[1] ) )
    {
    case 0:
        udp_conn_sender();
        break;
    case 1:
        udp_conn_receiver();
        break;
    case 2:
        udp_conn_loop_recv();
        break;
    case 3:
        udp_conn_loop_send();
        break;
    }
    net::stop_network();
    return 0;
}
