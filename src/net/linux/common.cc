
#include "net/defs.h"
#include <sys/stat.h>
#include <sys/fcntl.h>

NAMESPACE_TARO_NET_BEGIN

bool set_fdnoblock( int32_t sfd, bool n )
{
    int flags, s;  
    flags = fcntl( sfd, F_GETFL, 0 );
    if( flags == -1 )
    {
        return false;
    }

    if( n )
    {
        flags |= O_NONBLOCK;
    }
    else
    {
        flags &= ~O_NONBLOCK;
    }

    s = fcntl( sfd, F_SETFL, flags );
    if( s == -1 )
    {
        return false;
    }
    return true;
}

bool is_disconnect( int ret )
{
    if( ret == 0 )
    {
        return true;
    }

    if( ret == -1 && ( ECONNRESET == errno || ENOTCONN == errno || EPIPE == errno ) )
    {
        return true;
    }
    return false;
}

NAMESPACE_TARO_NET_END