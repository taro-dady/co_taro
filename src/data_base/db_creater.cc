
#include "data_base/impl/db_sqlite.h"

NAMESPACE_TARO_DB_BEGIN

TARO_DLL_EXPORT DataBaseSPtr create_database( const char* type )
{
    if ( std::string( type ) == DB_TYPE_SQLITE )
    {
        return std::make_shared<SQLiteDB>();
    }
    return nullptr;
}

NAMESPACE_TARO_DB_END