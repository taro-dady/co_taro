﻿
#include "base/error_no.h"
#include "base/config/yaml_config.h"
#include "base/utils/arg_expander.h"
#include <fstream>

NAMESPACE_TARO_BEGIN

static const char* saving_flag = "#tarw";
static const char* ready_flag  = "#taro";
static uint32_t flag_len = 5;

extern Optional<int32_t> get_index( std::string const& index );

static bool save_file( const char* path, YAML::Node const& node )
{
    std::ofstream fs( path );
    if ( !fs )
    {
        return false;
    }

    fs.write( ( char* )saving_flag, flag_len );
    fs << " configuration file!" << std::endl;
    fs << node;

    fs.seekp( 0 );
    fs.write( ( char* )ready_flag, flag_len );
    return true;
}

static bool load_file( const char* path, YAML::Node& node )
{
    std::ifstream fs( path );
    if ( !fs )
    {
        std::cout << "open file failed:" << path << std::endl;
        return false;
    }

    std::string line;
    std::getline( fs, line );
    if ( line.empty() )
    {
        std::cout << "file get line error path:" << path << std::endl;
        return false;
    }

    std::string prefix = line.substr( 0, flag_len );
    if ( prefix != ready_flag )
    {
        std::cout << "file state error:" << path << " prefix:" << prefix << std::endl;
        return false;
    }

    std::string context;
    context += line;
    context += "\n";
    while( std::getline( fs, line ) )
    {
        context += line;
        context += "\n";
    }

    try
    {
        node = YAML::Load( context );
    }
    catch( ... )
    {
        std::cout << "load file failed:" << path << std::endl;
        return false;
    }
    return true;
}

static YAML::Node to_yaml_path( std::string path, YAML::Node n, bool& end, bool& error )
{
    if ( n.IsNull() || error )
    {
        error = true;
        return YAML::Node();
    }

    std::string item;
    auto pos = path.find( "." );
    if ( pos != std::string::npos )
    {
        item = path.substr( 0, pos );
        path = path.substr( pos + 1 );
    }
    else
    {
        end = true;
        item = path;
    }

    auto index = get_index( item );
    if ( index.valid() ) 
    {
        if ( !end )
            return to_yaml_path( path, n[index.value()], end, error );
        else
            return n[index.value()];
    }

    if ( !end )
        return to_yaml_path( path, n[item], end, error );
    else
        return n[item];
}

struct YamlConfig::Impl
{
    YAML::Node  cfg_;
    std::string path_;
    std::string path_bk_;
};

YamlConfig::YamlConfig()
    : impl_( new Impl )
{

}

YamlConfig::~YamlConfig()
{
    delete impl_;
}

int32_t YamlConfig::set_param( const char* user, const char* user_backup )
{
    if ( !STRING_CHECK( user ) )
    {
        return TARO_ERR_INVALID_ARG;
    }

    impl_->path_    = user;
    impl_->path_bk_ = ( user_backup != nullptr ) ? user_backup : "";
    return TARO_OK;
}

int32_t YamlConfig::load()
{
    if ( impl_->path_.empty() )
    {
        return TARO_ERR_INVALID_RES;
    }

    if ( !load_file( impl_->path_.c_str(), impl_->cfg_ ) )
    {
        if ( !impl_->path_bk_.empty() && load_file( impl_->path_bk_.c_str(), impl_->cfg_ ) )
        {
            ( void )save_file( impl_->path_.c_str(), impl_->cfg_ );
        }
        else
        {
            return TARO_ERR_FAILED;
        }
    }
    return TARO_OK;
}

bool YamlConfig::set_config( YAML::Node* config )
{
    if ( impl_->cfg_ == *config )
    {
        return true;
    }

    impl_->cfg_ = *config;
    save_file( impl_->path_.c_str(), impl_->cfg_ );
    if ( !impl_->path_bk_.empty() )
    {
        save_file( impl_->path_bk_.c_str(), impl_->cfg_ );
    }
    return true;
}

bool YamlConfig::set_config( const char* path, YAML::Node* config )
{
    bool end = false, error = false;
    auto ret = to_yaml_path( path, impl_->cfg_, end, error );
    if ( ret.IsNull() || error )
    {
        return false;
    }

    ret = *config;
    save_file( impl_->path_.c_str(), impl_->cfg_ );
    if ( !impl_->path_bk_.empty() )
    {
        save_file( impl_->path_bk_.c_str(), impl_->cfg_ );
    }
    return true;
}

void YamlConfig::get_config( YAML::Node** config )
{
    *config = &impl_->cfg_;
}

void YamlConfig::get_config( const char* path, YAML::Node** config )
{
    bool end = false, error = false;
    auto ret = to_yaml_path( path, impl_->cfg_, end, error );
    if ( ret && !error )
    {
        *config = new YAML::Node( ret );
    }
}

void YamlConfig::delete_node( YAML::Node* node )
{
    if( node != &impl_->cfg_ )
    {
        delete node;
    }
}

NAMESPACE_TARO_END
