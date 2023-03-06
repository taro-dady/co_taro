
#include "base/error_no.h"
#include "base/config/json_config.h"
#include "base/utils/arg_expander.h"
#include <fstream>

NAMESPACE_TARO_BEGIN

static const char* saving_flag = "tarw";
static const char* ready_flag  = "taro";
static constexpr uint32_t flag_len = 4;

static bool save_file( const char* path, Json const& value )
{
    std::ofstream fs( path );
    if ( !fs )
    {
        return false;
    }

    fs.write( ( char* )saving_flag, flag_len );
    fs << " configuration file!" << std::endl;
    fs << value.dump( 2 );

    fs.seekp( 0 );
    fs.write( ( char* )ready_flag, flag_len );
    return true;
}

static bool load_file( const char* path, Json& value )
{
    std::ifstream fs( path );
    if ( !fs )
    {
        std::cout << "open file failed:" << path << std::endl;
        return false;
    }

    char buf[flag_len + 1] = { 0 };
    fs.read( buf, flag_len );
    if ( strcmp( buf, ready_flag ) != 0 )
    {
        std::cout << "file state error:" << path << " data:" << buf << std::endl;
        return false;
    }

    std::string line;
    std::getline( fs, line );

    std::string context;
    while( std::getline( fs, line ) )
    {
        context += line;
    }
    return json_parse( context, value );
}

struct JsonConfig::Impl
{
    Json        cfg_;
    std::string path_;
    std::string path_bk_;
};

JsonConfig::JsonConfig()
    : impl_( new Impl )
{

}

JsonConfig::~JsonConfig()
{
    delete impl_;
}

int32_t JsonConfig::set_param( const char* user, const char* user_backup )
{
    if ( !STRING_CHECK( user ) )
    {
        return TARO_ERR_INVALID_ARG;
    }

    impl_->path_    = user;
    impl_->path_bk_ = ( user_backup != nullptr ) ? user_backup : "";
    return TARO_OK;
}

int32_t JsonConfig::load()
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

bool JsonConfig::set_config( Json* jcfg )
{
    if ( impl_->cfg_ == *jcfg )
    {
        return true;
    }

    impl_->cfg_ = *jcfg;
    save_file( impl_->path_.c_str(), impl_->cfg_ );
    if ( !impl_->path_bk_.empty() )
    {
        save_file( impl_->path_bk_.c_str(), impl_->cfg_ );
    }
    return true;
}

void JsonConfig::get_config( Json** config )
{
    *config = &impl_->cfg_;
}

NAMESPACE_TARO_END
