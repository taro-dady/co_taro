
#include "data_base/impl/db_sqlite.h"
#include "data_base/impl/db_helper_impl.h"

NAMESPACE_TARO_DB_BEGIN

SQLiteDBResult::SQLiteDBResult( sqlite3_stmt* stmt )
    : stmt_( stmt )
{

}

SQLiteDBResult::~SQLiteDBResult()
{
    if( stmt_ != nullptr )
    {
        sqlite3_finalize( stmt_ );
    }
}

bool SQLiteDBResult::next()
{
    return sqlite3_step( stmt_ ) == SQLITE_ROW;
}

char* SQLiteDBResult::get_col_val( int32_t col )
{
    return ( char* )sqlite3_column_text( stmt_, col );
}

std::vector<std::string> SQLiteDBResult::get_columns()
{
    std::vector<std::string> ret;

    auto col_num = sqlite3_column_count( stmt_ );
    for( int i = 0; i < col_num; ++i )
    {
        ret.emplace_back( sqlite3_column_name( stmt_, i ) );
    }
    return ret;
}

SQLiteDB::SQLiteDB()
    : handler_( nullptr )
{
    init_sql_param();
}

SQLiteDB::~SQLiteDB()
{
    if( handler_ != nullptr )
    {
        sqlite3_close( handler_ );
        handler_ = nullptr;
    }
}

void SQLiteDB::init_sql_param()
{
    auto_inc_str_ = "autoincrement";
    var_char_str_ = "varchar";
    double_str_   = "real";
    float_str_    = "real";
    int_str_      = "integer";
}

int32_t SQLiteDB::connect( DBUri const& uri )
{
    std::string file = uri[eDBUriFile];
    if( file.empty() )
    {
        DB_ERROR << "uri invalid";
        return TARO_ERR_INVALID_ARG;
    }

    auto res = sqlite3_open( file.c_str(), &handler_ );
    if( res != SQLITE_OK || handler_ == nullptr )
    {
        DB_DEBUG << "sqlite open file:" << file.c_str();
        return TARO_ERR_FAILED;
    }
    return TARO_OK;
}

int32_t SQLiteDB::excute_cmd( const char* sql )
{
    if( !STRING_CHECK( sql ) )
    {
        DB_ERROR << "sql invalid";
        return TARO_ERR_INVALID_ARG;
    }

    if( handler_ == nullptr )
    {
        DB_ERROR << "invalid handler";
        return TARO_ERR_INVALID_RES;
    }

    char* errmsg = nullptr;
    auto res = sqlite3_exec( handler_, sql, nullptr, nullptr, &errmsg );
    if( res != SQLITE_OK )
    {
        if( errmsg != nullptr )
        {
            DB_ERROR << "exec failed, error:" << errmsg;
            sqlite3_free( errmsg );
        }
        return TARO_ERR_FAILED;
    }
    return TARO_OK;
}

int32_t SQLiteDB::exec_cmd_ret_id( const char* sql, uint64_t& id )
{
    auto ret = excute_cmd( sql );
    if( ret == TARO_OK )
    {
        id = sqlite3_last_insert_rowid( handler_ );
    }
    return ret;
}

DBQueryResultSPtr SQLiteDB::query( const char* sql )
{
    if( !STRING_CHECK( sql ) )
    {
        DB_ERROR << "sql invalid";
        return nullptr;
    }

    if( handler_ == nullptr )
    {
        DB_ERROR << "invalid handler";
        return nullptr;
    }

    sqlite3_stmt* stmt = nullptr;
    auto res = sqlite3_prepare_v2( handler_, sql, -1, &stmt, nullptr );
    if( res != SQLITE_OK || sqlite3_step( stmt ) != SQLITE_ROW )
    {
        if( stmt != nullptr )
        {
            DB_ERROR << "query failed";
            sqlite3_finalize( stmt );
        }
        return nullptr;
    }
    return std::make_shared<SQLiteDBResult>( stmt );
}

int32_t SQLiteDB::begin_transaction()
{
    return excute_cmd( "BEGIN;" );
}

int32_t SQLiteDB::commit_transaction()
{
    return excute_cmd( "COMMIT;" );
}

int32_t SQLiteDB::rollback_transaction()
{
    return excute_cmd( "ROLLBACK;" );
}

NAMESPACE_TARO_DB_END
