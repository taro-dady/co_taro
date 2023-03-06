

#include "net/net_work.h"
#include "net/tcp_client.h"
#include "net/tcp_server.h"
#include "co_routine/inc.h"
#include "base/system/file_sys.h"
#include <string.h>

USING_NAMESPACE_TARO

void client_handle( net::TcpClientSPtr const& client )
{
    while( true )
    {
        char buf[128] = { 0 };
        auto ret = client->recv( buf, 128, 2000 );
        if ( ret > 0 )
        {
            std::string resp( buf, ret );
            NET_WARN << "receive:" << resp << std::endl;
            resp += " response";  
            ret = client->send( ( char* )resp.c_str(), resp.length() );
            if ( ret <= 0 ) 
            {
                NET_WARN << "write disconnect";
                break; 
            }
        }
        else if( ret == TARO_ERR_TIMEOUT )
        {
            NET_WARN << "wait timeout";
        }
        else
        {
            NET_WARN << "read disconnect";
            break;
        }
    }
    printf("client finish\n");  
}

void tcp_svr_test()
{
    log::set_sys_rotate_cfg( "log/tcp_server" );
    
    // 协程开启tcp服务
    co_run []()
    {
        auto svr = net::create_tcp_svr();
        svr->listen( "0.0.0.0", 22000 );
        while( true )
        {
            auto client = svr->accept();
            if ( nullptr == client )
            {
                NET_WARN << "server invalid.";
                break;
            }
            NET_WARN << "tcp server accept create";

            // 接收处理对端数据的协程
            co_run std::bind( client_handle, client );
        }
    };

    rt::co_loop();
}

void tcp_client_test()
{
    log::set_sys_rotate_cfg( "log/tcp_client" );
    
    co_run []()
    {
        auto client = net::create_tcp_cli();
        while( !client->connect("127.0.0.1", 22000) )
        {
            rt::co_wait(1000);
            printf("connect failed\n");  
        }
        printf("connect ok .........\n");
 
        while(1) 
        {
            const char* req = "hello server 1";
            client->send( ( char* )req, strlen( req ) ); 
            char buf[128] = { 0 };
            auto ret = client->recv( buf, sizeof( buf ), 2000 );
            if ( ret > 0 )
            {
                NET_WARN << std::string(buf, ret);
            }
            else if ( ret == TARO_ERR_CONTINUE )
            {
                continue; 
            }
            else
            {
                if ( ret == TARO_ERR_TIMEOUT )
                {
                    NET_WARN << "timeout";
                    continue; 
                }
                printf("disconnect\n");
                break;
            }
            rt::co_wait(200);
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
        tcp_client_test();
        break;
    case 1:
        tcp_svr_test();
        break;
    }
    net::stop_network();
    return 0;
}
