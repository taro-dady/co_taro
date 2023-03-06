
#include "co_routine/inc.h"
#include "net/net_work.h"
#include "net/ud_client.h"
#include "net/ud_server.h"

USING_NAMESPACE_TARO
USING_NAMESPACE_TARO_RT
USING_NAMESPACE_TRAO_NET

void ud_client_test(int tm)
{
    co_run [tm]()
    {
        UDClient cli;
        while ( !cli.connect( "ud_server", "ud_client" ) )
        {
            NET_WARN << "connect failed.......";
            rt::co_wait( 1000 );
        }
        NET_WARN << "connect ok.......";

        char buf[128];
        const char* send_msg = "hello ud server";
        while( 1 )
        {
            cli.send( ( char* )send_msg, strlen( send_msg ) );
            auto ret = cli.recv(buf, sizeof(buf), tm);
            if ( ret > 0 )
            {
                NET_WARN << "cli receive:" << std::string(buf, ret);
            }
            else if( ret == TARO_ERR_TIMEOUT )
            {
                NET_ERROR << "timeout";
                continue;
            }
            else
            {
                NET_ERROR << "disconnect";
                break;
            }
            rt::co_wait( 1000 );
        }
    };

    co_loop();
}

void ud_svr_conn( UDClientSPtr const& conn )
{
    char buf[128]; 
    const char* send_msg = "welcome ud client";
    while(1)
    {
        auto ret = conn->recv( buf, sizeof(buf) );
        if ( ret > 0 )
        {
            NET_WARN << "svr receive:" << std::string(buf, ret) << " peer:" << conn->get_peer_info();
            conn->send( ( char* )send_msg, strlen( send_msg )  );
        }
        else if( ret == TARO_ERR_TIMEOUT )
        {
            NET_ERROR << "timeout";
            continue;
        }
        else
        {
            NET_ERROR << "disconnect";
            break;
        }
    }
}

void ud_svr_test()
{
    co_run []()
    {
        auto svr = std::make_shared<UDServer>();
        if ( !svr->listen( "ud_server" ) )
        { 
            NET_ERROR << "listen failed";
            return;
        }

        while(1)
        {
            auto cli = svr->accept();
            co_run std::bind( ud_svr_conn, cli );
        }
    };

    co_loop();
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
        ud_client_test(TARO_FOREVER);
        break;
    case 1:
        ud_svr_test();
        break;
    }

    net::stop_network();
    return 0;
}
