
#include "net/udp_conn.h"
#include "net/net_work.h"
#include "base/utils/defer.h"
#include "base/utils/string_tool.h"
#include <fstream>

#if defined( _WIN32 ) || defined( _WIN64 )
#include<Winsock2.h>
#include<ws2tcpip.h>

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif

inline std::string get_hosts_path()
{
    char path[MAX_PATH] = { 0 };
    ::GetSystemDirectory( path, MAX_PATH );
    std::string hosts_path;
    hosts_path += path;
    hosts_path += "\\drivers\\etc\\hosts";
    return hosts_path;
}

#define STR_DUP _strdup

static const char* inet_ntop_v4( const void* src, char* dst, socklen_t size )
{
    const struct in_addr* as_in_addr = ( const struct in_addr* )src;
    snprintf( dst, size, "%d.%d.%d.%d",
        as_in_addr->S_un.S_un_b.s_b1,
        as_in_addr->S_un.S_un_b.s_b2,
        as_in_addr->S_un.S_un_b.s_b3,
        as_in_addr->S_un.S_un_b.s_b4 );
    return dst;
}

static const char* inet_ntop_v6( const void* src, char* dst, socklen_t size )
{
    const uint16_t* as_shorts = ( const uint16_t* )src;
    int runpos[8];
    int current = 1;
    int max = 0;
    int maxpos = -1;
    int run_array_size =8;

    // Run over the address marking runs of 0s.
    for( int i = 0; i < run_array_size; ++i )
    {
        if( as_shorts[i] == 0 )
        {
            runpos[i] = current;
            if( current > max )
            {
                maxpos = i;
                max = current;
            }
            ++current;
        }
        else
        {
            runpos[i] = -1;
            current = 1;
        }
    }

    if( max > 0 )
    {
        int tmpmax = maxpos;
        // Run back through, setting -1 for all but the longest run.
        for( int i = run_array_size - 1; i >= 0; i-- )
        {
            if( i > tmpmax )
            {
                runpos[i] = -1;
            }
            else if( runpos[i] == -1 )
            {
                // We're less than maxpos, we hit a -1, so the 'good' run is done.
                // Setting tmpmax -1 means all remaining positions get set to -1.
                tmpmax = -1;
            }
        }
    }

    char* cursor = dst;
    // Print IPv4 compatible and IPv4 mapped addresses using the IPv4 helper.
    // These addresses have an initial run of either eight zero-bytes followed
    // by 0xFFFF, or an initial run of ten zero-bytes.
    if( runpos[0] == 1 && ( maxpos == 5 || ( maxpos == 4 && as_shorts[5] == 0xFFFF ) ) )
    {
        *cursor++ = ':';
        *cursor++ = ':';
        if( maxpos == 4 )
        {
            cursor += snprintf( cursor, INET6_ADDRSTRLEN - 2, "ffff:" );
        }
        const struct in_addr* as_v4 =( const struct in_addr* )( &( as_shorts[6] ) );
        inet_ntop_v4( as_v4, cursor, ( socklen_t )( INET6_ADDRSTRLEN - ( cursor - dst ) ) );
    }
    else
    {
        for( int i = 0; i < run_array_size; ++i )
        {
            if( runpos[i] == -1 )
            {
                cursor += snprintf( cursor, INET6_ADDRSTRLEN - ( cursor - dst ), "%x", ntohs( as_shorts[i] ) );
                if( i != 7 && runpos[i + 1] != 1 )
                {
                    *cursor++ = ':';
                }
            }
            else if( runpos[i] == 1 )
            {
                // Entered the run; print the colons and skip the run.
                *cursor++ = ':';
                *cursor++ = ':';
                i += ( max - 1 );
            }
        }
    }
    return dst;
}

const char* inet_ntop( int af, const void* src, char* dst, socklen_t size )
{
    if( !src || !dst )
    {
        return NULL;
    }

    switch( af )
    {
    case AF_INET:
    {
        return inet_ntop_v4( src, dst, size );
    }
    case AF_INET6:
    {
        return inet_ntop_v6( src, dst, size );
    }
    }
    return NULL;
}
#else
#include <arpa/inet.h>
#define STR_DUP strdup
inline std::string get_hosts_path()
{
    return "/etc/hosts";
}
#endif

NAMESPACE_TARO_NET_BEGIN

#define DNS_HOST  0x01
#define DNS_CNAME 0x05
#define DNS_PORT  53

struct DnsHeader
{
    unsigned short id;
    unsigned short flags;
    unsigned short qdcount;
    unsigned short ancount;
    unsigned short nscount;
    unsigned short arcount;
};

struct DnsQuestion
{
    int length;
    unsigned short qtype;
    unsigned short qclass;
    char* qname;
};

static int32_t dns_create_header( DnsHeader* header )
{
    memset( header, 0, sizeof( DnsHeader ) );
    ::srand( ( uint32_t )time( NULL ) );

    header->id = ::rand();
    header->flags |= htons( 0x0100 );
    header->qdcount = htons( 1 );
    return 0;
}

static int32_t dns_create_question( DnsQuestion* question, const char* hostname )
{
    memset( question, 0, sizeof( DnsQuestion ) );
    question->qname  = ( char* )malloc( strlen( hostname ) + 2 );
    question->length = ( int )strlen( hostname ) + 2;
    question->qtype  = htons( 1 );
    question->qclass = htons( 1 );

    const char delim[2] = ".";
    char* hostname_dup = STR_DUP( hostname );
    char* token = strtok( hostname_dup, delim );
    char* qname_p = question->qname;
    while( token != NULL )
    {
        size_t len = strlen( token );
        *qname_p = ( char )len;
        qname_p++;
        strncpy( qname_p, token, len + 1 );
        qname_p += len;
        token = strtok( NULL, delim );
    }
    free( hostname_dup );
    return 0;
}

static int32_t dns_build_request( DnsHeader* header, DnsQuestion* question, char* request )
{
    int32_t header_s = sizeof( DnsHeader );
    int32_t question_s = question->length + sizeof( question->qtype ) + sizeof( question->qclass );
    int32_t length = question_s + header_s;
    int32_t offset = 0;

    memcpy( request + offset, header, sizeof( DnsHeader ) );
    offset += sizeof( DnsHeader );
    memcpy( request + offset, question->qname, question->length );
    offset += question->length;
    memcpy( request + offset, &question->qtype, sizeof( question->qtype ) );
    offset += sizeof( question->qtype );
    memcpy( request + offset, &question->qclass, sizeof( question->qclass ) );
    return length;
}

static int32_t is_pointer( int32_t in )
{
    return ( ( in & 0xC0 ) == 0xC0 );
}

// dns解析名字
static void dns_parse_name( unsigned char* chunk, unsigned char* ptr, char* out, int* len )
{
    int flag = 0, n = 0;
    char* pos = out + ( *len );

    while( 1 )
    {
        flag = ( int )ptr[0];
        if( flag == 0 )
            break;

        if( is_pointer( flag ) )
        {
            n = ( int )ptr[1];
            ptr = chunk + n;
            dns_parse_name( chunk, ptr, out, len );
            break;
        }
        else
        {
            ptr++;
            memcpy( pos, ptr, flag );
            pos += flag;
            ptr += flag;

            *len += flag;
            if( ( int )ptr[0] != 0 )
            {
                memcpy( pos, ".", 1 );
                pos += 1;
                ( *len ) += 1;
            }
        }
    }
}

static void dns_parse_response( char* buffer, std::vector< DnsItem >& domains )
{
    int i = 0;
    unsigned char* ptr = ( unsigned char* )buffer;
    ptr += 4;
    int querys = ntohs( *( unsigned short* )ptr );
    ptr += 2;
    int answers = ntohs( *( unsigned short* )ptr );
    ptr += 6;
    for( i = 0; i < querys; i++ )
    {
        while( 1 )
        {
            int flag = ( int )ptr[0];
            ptr += ( flag + 1 );
            if( flag == 0 )
                break;
        }
        ptr += 4;
    }

    char cname[128], aname[128], ip[20], netip[4];
    int len, type, datalen;

    int cnt = 0;
    domains.resize( answers );
    DnsItem* list = domains.data();
    for( i = 0; i < answers; i++ )
    {
        memset( aname, 0, sizeof( aname ) );
        len = 0;

        dns_parse_name( ( unsigned char* )buffer, ptr, aname, &len );
        ptr += 2;

        type = htons( *( unsigned short* )ptr );
        ptr += 4;

        //ttl = htons( *( unsigned short* )ptr );
        ptr += 4;

        datalen = ntohs( *( unsigned short* )ptr );
        ptr += 2;

        if( type == DNS_CNAME )
        {
            memset( cname, 0, sizeof( cname ) );
            len = 0;
            dns_parse_name( ( unsigned char* )buffer, ptr, cname, &len );
            ptr += datalen;
        }
        else if( type == DNS_HOST )
        {
            memset( ip, 0, sizeof( ip ) );
            if( datalen == 4 )
            {
                memcpy( netip, ptr, datalen );
                inet_ntop( AF_INET, netip, ip, sizeof( struct sockaddr ) );
                list[cnt].domain = aname;
                list[cnt].ip = ip;
                cnt++;
            }
            ptr += datalen;
        }
    }
}

static std::vector< DnsItem > get_host_by_name_remote( const char* name, const char* dns )
{
    auto conn = net::create_udp_conn();
    DnsHeader header ={ 0 };
    dns_create_header( &header );

    DnsQuestion question ={ 0 };
    Defer defer( [&]
    {
        if( question.qname )
        {
            free( question.qname );
        }
    } );
    dns_create_question( &question, name );

    char request[1024] ={ 0 };
    int req_len = dns_build_request( &header, &question, request );
    if( conn->sendto( IpAddr( dns, DNS_PORT ), request, req_len ) < 0 )
    {
        NET_ERROR << "send error";
        return std::vector< DnsItem >();
    }

    IpAddr remote;
    char resp[1024] ={ 0 };
    auto ret = conn->recvfrom( resp, 1024, remote, 2000 );
    if( ret < 0 )
    {
        NET_ERROR << "receive error";
        return std::vector< DnsItem >();
    }

    std::vector< DnsItem > items;
    dns_parse_response( resp, items );
    return items;
}

static std::vector<std::string> string_split( std::string str )
{
    std::vector<std::string> vec;
    while( 1 )
    {
        auto pos  = str.find( " " );
        auto pos1 = str.find( "\t" ); 
        if ( pos == std::string::npos && pos1 == std::string::npos )
            break;
        
        auto min = ( pos < pos1 ? pos : pos1 );
        auto temp = string_trim( str.substr( 0, min ) );
        str = str.substr( min + 1 );
        if ( !str.empty() )
            str = string_trim( str );
        vec.emplace_back( temp );
    }
    
    if ( !str.empty() )
        vec.emplace_back( str );
    return vec;
}

static std::vector< DnsItem > get_host_by_name_local( const char* name )
{
    std::ifstream fs( get_hosts_path() );
    if( !fs )
    {
        return std::vector< DnsItem >();
    }

    std::string line;
    std::vector< DnsItem > result;
    while( std::getline( fs, line ) )
    {
        if( line.empty() )
            continue;

        line = string_trim( line );
        if( line.empty() || line[0] == '#' || line[0] == ';' )
            continue;

        auto str_list = string_split( line );
        if( str_list.size() <= 1 )
            continue;

        auto value = str_list[0];
        value = string_trim( value );
        
        for( size_t i = 1; i < str_list.size(); ++i )
        {
            auto key = str_list[i];
            key = string_trim( key );
            if( key.empty() || key[0] == '#' || key[0] == ';' )
                continue;
            key = string_trim( key, "#" );
            if( key == name )
                result.emplace_back( DnsItem{ "", value });
        }
    }
    return result;
}

TARO_DLL_EXPORT std::vector< DnsItem > get_host_by_name( const char* name, const char* dns )
{
    if( !STRING_CHECK( name, dns ) )
    {
        NET_ERROR << "invalid parameter";
        return std::vector< DnsItem >();
    }

    auto result = get_host_by_name_local( name );
    if( !result.empty() )
    {
        return result;
    }
    return get_host_by_name_remote( name, dns );
}

NAMESPACE_TARO_NET_END
