
#include "base/error_no.h"
#include "base/utils/defer.h"
#include "base/system/file_sys.h"
#include "base/utils/arg_expander.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <list>
#include <sys/statfs.h>
#include <sys/sendfile.h>

NAMESPACE_TARO_BEGIN

int32_t FileSystem::check_dir( const char* dir )
{
    if ( !STRING_CHECK( dir ) )
    {
        return TARO_ERR_INVALID_ARG;
    }

    DIR* dp = nullptr;
    if ( nullptr == ( dp = opendir( dir ) ) )
    {
        return TARO_ERR_FAILED;
    }
    closedir( dp );
    return TARO_OK;
}

int32_t FileSystem::create_dir( const char* dir )
{
    if ( !STRING_CHECK( dir ) )
    {
        return TARO_ERR_INVALID_ARG;
    }
    
    std::string path = dir;
    for ( size_t i = 1; i < strlen( dir ); ++i ) 
    {
        if ( dir[i] == '/' )
        {
            path[i] = 0;
            if ( mkdir( path.c_str(), S_IRWXU ) != 0 ) 
            {
                if ( errno != EEXIST ) 
                {
                    return TARO_ERR_FAILED;
                }
            }
            path[i] = '/';
        }
    }

    if ( mkdir( path.c_str(), S_IRWXU ) != 0 ) 
    {
        if (errno != EEXIST) 
        {
            return TARO_ERR_FAILED;
        }
    }
    return TARO_OK;
}

int32_t FileSystem::check_file( const char* file )
{
    if ( !STRING_CHECK( file ) )
    {
        return TARO_ERR_INVALID_ARG;
    }

    struct stat st;
    if ( 0 != stat( file, &st ) )
    {
        return TARO_ERR_FAILED;
    }
    return S_ISREG( st.st_mode ) ? TARO_OK : TARO_ERR_FAILED;
}

int32_t FileSystem::copy_file( const char* src, const char* dst )
{
    if ( !STRING_CHECK( src, dst ) )
    {
        return TARO_ERR_INVALID_ARG;
    }

    int source = open( src, O_RDONLY, 0 );
    int dest   = open( dst, O_WRONLY | O_CREAT, 0644 );

    Defer defer([&]()
    {
        if ( source != -1 ) close( source );
        if ( dest != -1 )   close( dest );
    });

    if ( source == -1 || dest == -1 )
    {
        return TARO_ERR_FILE_OP;
    }

    struct stat stat_source;
    fstat( source, &stat_source );
    sendfile( dest, source, 0, stat_source.st_size );
    return TARO_OK;
}

std::list<FileSysNode> FileSystem::get_nodes( const char* dir )
{
    std::list<FileSysNode> ret;

    if ( !STRING_CHECK( dir ) )
        return ret;
    
    struct stat s;
    lstat( dir, &s );
    if ( !S_ISDIR( s.st_mode ) )
        return ret;

    DIR* dir_obj = opendir( dir );
    if ( nullptr == dir_obj )
        return ret;
    
    Defer rc( [dir_obj]()
    {
        closedir( dir_obj );
    } );

    struct dirent* filename;
    while ( ( filename = readdir( dir_obj ) ) != nullptr )
    {
        if ( 0 == strcmp( filename->d_name, "." ) 
          || 0 == strcmp( filename->d_name, ".." ) )
            continue;
        
        std::string full_path = dir;
        full_path += "/";
        full_path += filename->d_name;
        lstat( full_path.c_str(), &s );

        FileSysNode node;
        node.name  = filename->d_name;
        node.mtime = s.st_mtime;
        node.size  = s.st_size;

        if ( S_ISDIR( s.st_mode ) )
            node.type = eNodeTypeDir;
        else if ( S_ISREG( s.st_mode ) )
            node.type = eNodeTypeFile;
        else
            continue;
        ret.emplace_back( node );
    }
    return ret;
}

NAMESPACE_TARO_END
