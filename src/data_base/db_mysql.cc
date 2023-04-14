

#include "base/system/dyn_lib.h"
#include "data_base/impl/db_mysql.h"

std::unique_ptr<taro::DynLib> mysql_dyn_ptr;

#define MYSQL_LOAD_FUN(n) taro_##n = decltype( taro_##n )( mysql_dyn_ptr->func_addr( #n ) ); TARO_ASSERT( nullptr != taro_##n )

/****************************** MYSQL function ************************************/

static MYSQL* ( *taro_mysql_init )( MYSQL * );
static void ( *taro_mysql_close )( MYSQL* );
static MYSQL* ( *taro_mysql_real_connect )( MYSQL *mysql, const char *host,
                                  const char *user, const char *passwd,
                                  const char *db, unsigned int port,
                                  const char *unix_socket,
                                  unsigned long clientflag );
static int ( *taro_mysql_query )( MYSQL*, const char* );
static const char* ( *taro_mysql_error )( MYSQL* );
static uint64_t ( *taro_mysql_insert_id )( MYSQL* );

// query
static MYSQL_RES* ( *taro_mysql_store_result )( MYSQL* );
static void ( *taro_mysql_free_result )( MYSQL_RES* );
static MYSQL_ROW  ( *taro_mysql_fetch_row )( MYSQL_RES* );
static MYSQL_FIELD* ( *taro_mysql_fetch_fields )( MYSQL_RES* );
static unsigned int ( *taro_mysql_num_fields )( MYSQL_RES* );
static MYSQL_FIELD* ( *taro_mysql_fetch_field_direct )( MYSQL_RES*, unsigned int );

/****************************** MYSQL function ************************************/

NAMESPACE_TARO_DB_BEGIN

extern TARO_DLL_EXPORT int32_t load_mysql_library( const char* path )
{
    if( nullptr != mysql_dyn_ptr )
    {
        DB_ERROR << "mysql library already loaded";
        return TARO_ERR_MULTI_OP;
    }

    mysql_dyn_ptr.reset( new DynLib() );
    auto ret = mysql_dyn_ptr->load( path );
    if ( ret != TARO_OK )
    {
        DB_ERROR << "load library failed path:" << ( STRING_CHECK( path ) ? path : "null" );
        return TARO_ERR_FAILED;
    }

    // 加载动态接口
    MYSQL_LOAD_FUN( mysql_init );
    MYSQL_LOAD_FUN( mysql_real_connect );
    MYSQL_LOAD_FUN( mysql_close );
    MYSQL_LOAD_FUN( mysql_query );
    MYSQL_LOAD_FUN( mysql_error );
    MYSQL_LOAD_FUN( mysql_insert_id );

    MYSQL_LOAD_FUN( mysql_store_result );
    MYSQL_LOAD_FUN( mysql_free_result );
    MYSQL_LOAD_FUN( mysql_fetch_row );
    MYSQL_LOAD_FUN( mysql_fetch_fields );
    MYSQL_LOAD_FUN( mysql_num_fields );
    MYSQL_LOAD_FUN( mysql_fetch_field_direct );

    DB_TRACE << "load dynamic library ok.";
    return TARO_OK;
}

MySQLDBResult::MySQLDBResult( MYSQL_RES* result )
    : result_( result )
    , row_( taro_mysql_fetch_row( result ) )
    , max_col_( taro_mysql_num_fields( result ) )
{
    
}

MySQLDBResult::~MySQLDBResult()
{
    taro_mysql_free_result( result_ );
}

bool MySQLDBResult::valid() const
{
    return row_ != nullptr;
}   

bool MySQLDBResult::next()
{
    row_ = taro_mysql_fetch_row( result_ );
    return row_ != nullptr;
}

char* MySQLDBResult::get_col_val( int32_t col )
{
    TARO_ASSERT( row_ && col < max_col_, "max col:", max_col_ );
    return row_[col];
}

std::vector<std::string> MySQLDBResult::get_columns()
{
    std::vector<std::string> vec;
    auto fields = taro_mysql_fetch_fields( result_ );
    for ( int32_t i = 0; i < max_col_; ++i )
    {
        vec.push_back( fields[i].name );
    }
    return vec;
}

MySQLDB::MySQLDB()
    : connect_( false )
{
    taro_mysql_init( &handler_ );
    init_sql_param();
}

MySQLDB::~MySQLDB()
{
    taro_mysql_close( &handler_ );
}

void MySQLDB::init_sql_param()
{
    auto_inc_str_ = "auto_increment";
    var_char_str_ = "text";
    double_str_   = "double";
    float_str_    = "float";
    int_str_      = "int";
}

int32_t MySQLDB::connect( DBUri const& uri )
{
    if( connect_ )
    {
        DB_ERROR << "already connected";
        return TARO_ERR_MULTI_OP;
    }

    if( uri[eDBUriHost].empty() || uri[eDBUriUser].empty() || uri[eDBUriDB].empty() )
    {
        DB_ERROR << "uri invalid";
        return TARO_ERR_INVALID_ARG;
    }

    auto result = taro_mysql_real_connect( 
                &handler_, 
                uri[eDBUriHost].c_str(), 
                uri[eDBUriUser].c_str(),
                uri[eDBUriPwd].empty() ? NULL : uri[eDBUriPwd].c_str(),
                uri[eDBUriDB].c_str(), 
                uri.port().valid() ? uri.port().value() : 0, 
                0, 
                0 );
    if ( result == nullptr )
    {
        DB_ERROR << "sql connect to host:" << uri[eDBUriHost] << " db:" << uri[eDBUriDB] << " failed";
        return TARO_ERR_FAILED;
    }
    connect_ = true;
    return TARO_OK;
}

int32_t MySQLDB::excute_cmd( const char* sql )
{
    if( !connect_ )
    {
        DB_ERROR << "connect first";
        return TARO_ERR_INVALID_RES;
    }

    if( !STRING_CHECK( sql ) )
    {
        DB_ERROR << "sql invalid";
        return TARO_ERR_INVALID_ARG;
    }
    
    auto ret = taro_mysql_query( &handler_, sql );
    if ( 0 != ret )
    {
        DB_ERROR << "mysql query failed error:" << taro_mysql_error( &handler_ );
        return TARO_ERR_FAILED;
    }
    return TARO_OK;
}

int32_t MySQLDB::exec_cmd_ret_id( const char* sql, uint64_t& id )
{
    auto ret = excute_cmd( sql );
    if ( TARO_OK != ret )
    {
        return ret;
    }
    id = taro_mysql_insert_id( &handler_ );
    return TARO_OK;
}

DBQueryResultSPtr MySQLDB::query( const char* sql ) 
{
    if( !connect_ )
    {
        DB_ERROR << "connect first";
        return nullptr;
    }

    if( !STRING_CHECK( sql ) )
    {
        DB_ERROR << "sql invalid";
        return nullptr;
    }

    auto ret = taro_mysql_query( &handler_, sql );
    if ( 0 != ret )
    {
        DB_ERROR << "mysql query failed error:" << taro_mysql_error( &handler_ );
        return nullptr;
    }
    
    auto query_res = std::make_shared<MySQLDBResult>( taro_mysql_store_result( &handler_ ) );
    if ( !query_res->valid() )
    {
        DB_TRACE << "not found";
        return nullptr;
    }
    return query_res;
}

int32_t MySQLDB::begin_transaction() 
{
    return excute_cmd( "begin;" );
}

int32_t MySQLDB::commit_transaction() 
{
    return excute_cmd( "commit;" );
}

int32_t MySQLDB::rollback_transaction() 
{
    return excute_cmd( "rollback;" );
}

NAMESPACE_TARO_DB_END
