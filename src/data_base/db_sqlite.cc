
#include "data_base/impl/db_sqlite.h"

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

SQLiteDB::SQLiteDB()
    : handler_( nullptr )
{

}

SQLiteDB::~SQLiteDB()
{
    if( handler_ != nullptr )
    {
        sqlite3_close( handler_ );
        handler_ = nullptr;
    }
}

int32_t SQLiteDB::connect( const char* uri )
{
    if ( !STRING_CHECK( uri ) )
    {
        DB_ERROR << "uri invalid";
        return TARO_ERR_INVALID_ARG;
    }

    auto res = sqlite3_open( uri, &handler_ );
    if( res != SQLITE_OK || handler_ == nullptr )
    {
        DB_DEBUG << "open uri failed";
        return TARO_ERR_FAILED;
    }
    return TARO_OK;
}

int32_t SQLiteDB::disconnect()
{
    if( nullptr != handler_ )
    {
        sqlite3_close( handler_ );
        handler_ = nullptr;
    }
    return TARO_OK;
}

int32_t SQLiteDB::excute_cmd( const char* sql )
{
    if ( !STRING_CHECK( sql ) )
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
    if ( ret == TARO_OK )
    {
        id = sqlite3_last_insert_rowid( handler_ );
    }
    return ret;
}

DBQueryResultSPtr SQLiteDB::query( const char* sql )
{
    if ( !STRING_CHECK( sql ) )
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
    return exec_trans( "BEGIN;" );
}

int32_t SQLiteDB::commit_transaction()
{
    return exec_trans( "COMMIT;" );
}

int32_t SQLiteDB::rollback_transaction()
{
    return exec_trans( "ROLLBACK;" );
}

int32_t SQLiteDB::exec_trans( const char* cmd )
{
    if ( !STRING_CHECK( cmd ) )
    {
        DB_ERROR << "cmd invalid";
        return TARO_ERR_INVALID_ARG;
    }

    if( handler_ == nullptr )
    {
        DB_ERROR << "invalid handler";
        return TARO_ERR_INVALID_RES;
    }

    char* errmsg = nullptr;
    auto res = sqlite3_exec( handler_, cmd, 0, 0, &errmsg );
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

NAMESPACE_TARO_DB_END
