
#include "base/error_no.h"
#include "base/system/file_sys.h"
#include "base/utils/arg_expander.h"
#include <Winsock2.h>
#include <windows.h>
#include <shlwapi.h>
#include <fstream>
#include <list>
#include <algorithm>

#pragma comment( lib,"shlwapi.lib" )

NAMESPACE_TARO_BEGIN

int32_t FileSystem::check_dir( const char* dir )
{
    if ( !STRING_CHECK( dir ) )
    {
        return TARO_ERR_INVALID_ARG;
    }
    return PathIsDirectory( dir ) ? TARO_OK : TARO_ERR_FAILED; 
}

int32_t FileSystem::create_dir( const char* dir )
{
    if( !STRING_CHECK( dir ) )
    {
        return TARO_ERR_INVALID_ARG;
    }

    if( PathIsDirectory( dir ) )
    {
        return TARO_OK;
    }

    size_t pos;
    std::string directory = dir;
    while( ( pos = directory.find( "\\" ) ) != std::string::npos )
    {
        directory.replace( pos, strlen( "\\" ), "/");
    }
    
    for( size_t i = 1; i < directory.length(); ++i )
    {
        if( directory[i] == '/' )
        {
            directory[i] = 0;
            if( PathIsDirectory( directory.c_str() ) )
            {
                directory[i] = '/';
                continue;
            }

            if( !CreateDirectory( directory.c_str(), NULL ) )
            {
                return TARO_ERR_FAILED;
            }
            directory[i] = '/';
        }
    }

    if( !CreateDirectory( directory.c_str(), NULL ) )
    {
        return TARO_ERR_FAILED;
    }
    return TARO_OK;
}

int32_t FileSystem::check_file( const char* path )
{
    if ( !STRING_CHECK( path ) )
    {
        return TARO_ERR_INVALID_ARG;
    }  

    WIN32_FIND_DATA FindFileData;
    BOOL bValue  = FALSE;
    HANDLE hFind = FindFirstFile( path, &FindFileData );
    if ( ( hFind != INVALID_HANDLE_VALUE ) && !( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
    {
        bValue = TRUE;
    }
    
    if ( hFind != INVALID_HANDLE_VALUE )
    {
        FindClose( hFind );
    }
    return TRUE == bValue ? TARO_OK : TARO_ERR_FAILED;
}

int32_t FileSystem::copy_file( const char* src, const char* dst )
{
    if ( !STRING_CHECK( src, dst ) )
    {
        return TARO_ERR_INVALID_ARG;
    }

    std::ifstream src_fs( src, std::ios::binary );
    std::ofstream dst_fs( dst, std::ios::binary );
    if ( !src_fs || !dst_fs )
    {
        return TARO_ERR_FILE_OP;
    }
    dst_fs << src_fs.rdbuf();
    return TARO_OK;
}

std::list<FileSysNode> FileSystem::get_nodes( const char* dir )
{
    std::list<FileSysNode> ret;

    if ( !STRING_CHECK( dir ) )
    {
        return ret;
    }

    HANDLE          hFind;
    WIN32_FIND_DATA findData;

    std::string dir_new = dir;
    dir_new += "/*.*";
    hFind = FindFirstFile( dir_new.c_str(), &findData );
    if ( hFind == INVALID_HANDLE_VALUE )
    {
        return ret;
    }
    
    do
    {
        if ( 0 == strcmp( findData.cFileName, "." ) || 0 == strcmp( findData.cFileName, ".." ) )
        {
            continue;
        }
        
        FileSysNode node;
        node.name  = findData.cFileName;
        node.mtime = ( ( uint64_t )findData.ftLastWriteTime.dwHighDateTime << 32 ) | findData.ftLastWriteTime.dwLowDateTime;
        node.size  = ( ( uint64_t )findData.nFileSizeHigh << 32 ) | findData.nFileSizeLow;
        node.type  = ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ? eNodeTypeDir : eNodeTypeFile;
        ret.emplace_back( node );

    } while ( FindNextFile( hFind, &findData ) );

    FindClose( hFind ); 
    return ret;
}

NAMESPACE_TARO_END
