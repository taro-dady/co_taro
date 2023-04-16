
#include "data_base/impl/db_sqlite.h"
#include "data_base/impl/db_mysql.h"

NAMESPACE_TARO_DB_BEGIN

TARO_DLL_EXPORT DataBaseSPtr create_database( const char* type )
{
    if ( std::string( type ) == DB_TYPE_SQLITE )
    {
        return std::make_shared<SQLiteDB>();
    }
    else if ( std::string( type ) == DB_TYPE_MYSQL )
    {
        return std::make_shared<MySQLDB>();
    }
    return nullptr;
}

NAMESPACE_TARO_DB_END