
#include "data_base/impl/db_sqlite.h"
#include "data_base/impl/db_helper_impl.h"

NAMESPACE_TARO_DB_BEGIN

// 约束条件转换为字符
static std::string constr_to_str( DBContraintArgBaseSPtr const& base_ptr, uint32_t ignore = 0 )
{
    TARO_ASSERT( base_ptr != nullptr );

    // 忽略的类型不处理
    if( ignore & base_ptr->kind() )
    {
        return "";
    }

    if( eDBConstraintPrimaryKey == base_ptr->kind() )
    {
        return "primary key";
    }
    else if( eDBConstraintNotNull == base_ptr->kind() )
    {
        return "not null";
    }
    else if( eDBConstraintUnique == base_ptr->kind() )
    {
        return "unique";
    }
    else if( eDBConstraintDefault == base_ptr->kind() )
    {
        return "default";
    }
    else if( eDBConstraintDefaultValue == base_ptr->kind() )
    {
        auto val_ptr = std::dynamic_pointer_cast< DBContraintArg >( base_ptr );
        TARO_ASSERT( val_ptr != nullptr );
        return "default " + val_ptr->get();
    }
    else if( eDBConstraintAutoInc == base_ptr->kind() )
    {
        return "autoincrement";
    }
    else if( eDBConstraintCheck == base_ptr->kind() )
    {
        auto val_ptr = std::dynamic_pointer_cast< DBContraintArg >( base_ptr );
        TARO_ASSERT( val_ptr != nullptr );
        return "check(" + val_ptr->get() + ")";
    }

    TARO_ASSERT( 0 );
    return "";
}

static bool is_string_type( const std::type_info* type )
{
    return ( *type == typeid( std::string ) || *type == typeid( const char* ) || *type == typeid( char* ) );
}

static bool type_to_str( const std::type_info* p, std::string& tstr )
{
    TARO_ASSERT( p != nullptr );

    if( is_string_type( p ) )
    {
        tstr = "varchar";
    }
    else if( *p == typeid( int32_t )
        || *p == typeid( int16_t )
        || *p == typeid( int8_t )
        || *p == typeid( uint32_t )
        || *p == typeid( uint16_t )
        || *p == typeid( uint8_t ) )
    {
        tstr = "integer";
    }
    else if( *p == typeid( int64_t ) )
    {
        tstr = "bigint";
    }
    else if( *p == typeid( uint64_t ) )
    {
        tstr = "unsigned big int";
    }
    else if( *p == typeid( double ) || *p == typeid( float ) )
    {
        tstr = "real";
    }
    else if( *p == typeid( bool ) )
    {
        tstr = "boolean";
    }
    else
    {
        return false;
    }
    return true;
}

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
    if( !STRING_CHECK( uri ) )
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
    if( !STRING_CHECK( cmd ) )
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

std::string SQLiteDB::create_tbl_sql( const char* cls_name,
    std::vector<ClsMemberReflectorSPtr> const& members,
    CreateTblConstraint const& constraint )
{
    TARO_ASSERT( STRING_CHECK( cls_name ) && members.size() > 0 );

    // 获取约束条件
    std::set< std::string > primar_keys;
    std::vector< std::pair<std::string, std::string> > forein_keys;
    std::map< std::string, std::vector< DBContraintArgBaseSPtr > > col_cstrs;
    for( auto& one : constraint.contrains )
    {
        auto impl = DBContraintImpl::get( one );
        for( auto& arg : impl->kinds_ )
        {
            if( arg->kind() == eDBConstraintPrimaryKey )
            {
                primar_keys.insert( one.name() );
            }
            else if( arg->kind() == eDBConstraintForeignKey )
            {
                auto ptr = std::dynamic_pointer_cast< DBContraintArg >( arg );
                forein_keys.emplace_back( std::make_pair( one.name(), ptr->get() ) );
            }
            else
            {
                col_cstrs[one.name()].emplace_back( arg );
            }
        }
    }

    if( primar_keys.empty() )
    {
        DB_ERROR << "primary key not found";
        return "";
    }

    std::map< std::string, std::string > col_decl;
    for( auto& col : members )
    {
        // 列类型转换为字符串
        std::string value;
        if( !type_to_str( col->get_type(), value ) )
        {
            DB_ERROR << "column:" << col->get_name() << " type format error";
            return "";
        }

        // 列约束转换为字符串
        auto iter = col_cstrs.find( col->get_name() );
        if( iter != col_cstrs.end() )
        {
            for( size_t i = 0; i < iter->second.size(); ++i )
            {
                value += " " + constr_to_str( iter->second[i] );
            }
        }
        col_decl[col->get_name()] = value;
    }

    /// 组织SQL
    std::stringstream ss;
    ss << "create table ";
    if( constraint.create_when_no_exit.valid() )
    {
        ss << "if not exists ";
    }
    ss << cls_name << "(";

    for( auto& one : col_decl )
    {
        ss << one.first << " " << one.second << ",";
    }

    ss << "primary key(";
    size_t n = 0;
    for( auto& k : primar_keys )
    {
        ss << k;
        if( n < primar_keys.size() - 1 )
        {
            ss << ",";
        }
        ++n;
    }
    ss << ")";

    if( !forein_keys.empty() )
    {
        ss << ",";
        size_t n = 0;
        for( ; n < forein_keys.size(); ++n )
        {
            ss << "foreign key (" << forein_keys[n].first << ") references " << forein_keys[n].second;
            if( n < primar_keys.size() - 1 )
            {
                ss << ",";
            }
        }
    }

    ss << ");";
    return ss.str();
}

std::string SQLiteDB::query_tbl_sql( 
                        const char* cls_name, 
                        std::vector<ClsMemberReflectorSPtr> const& members,
                        DBQueryParam const& param )
{
    TARO_ASSERT( STRING_CHECK( cls_name ) && members.size() > 0 );

    // 组装查询列
    std::string col_str = "*";
    auto filter = DBFilterImpl::get( param.filter_ );
    if( filter && !filter->names_.empty() )
    {
        col_str = "";
        for ( auto const& one : members )
        {
            auto& name = one->get_name();
            auto iter = filter->names_.find( name );
            if ( filter->black_ )
            {
                if ( iter != filter->names_.end() )
                {
                    continue;
                }
            }
            else
            {
                if ( iter == filter->names_.end() )
                {
                    continue;
                }
            }

            if ( col_str.empty() )
            {
                col_str = name;
            }
            else
            {
                col_str += "," + name;
            }
        }
    }

    std::stringstream ss;
    ss << "select " << col_str << " from " << cls_name;

    auto cond = DBCondImpl::to_str( param.cond_ );
    if( !cond.empty() )
    {
        ss << " where " << cond;
    }

    if( param.order_.valid() )
    {
        std::string tmp = param.order_.value().second ? "asc" : "desc";
        ss << " order by " << param.order_.value().first << " " << tmp;
    }

    if ( param.limit_.valid() )
    {
        ss << " limit " << param.limit_.value().count_;
        if ( param.limit_.value().offset_ >= 0 )
        {
            ss << " offset " << param.limit_.value().offset_;
        }
    }
    ss << ";";
    return ss.str();
}

std::string SQLiteDB::insert_tbl_sql( 
                        void* obj,
                        const char* cls_name, 
                        std::vector<ClsMemberReflectorSPtr> const& members,
                        DBModifyParam const& param )
{
    TARO_ASSERT( obj && STRING_CHECK( cls_name ) && !members.empty() );

    std::string keys, values;
    for ( auto& one : members )
    {
        auto const& name = one->get_name();

        // 过滤判断
        auto filter = DBFilterImpl::get( param.filter_ );
        if ( nullptr != filter )
        {
            auto iter = filter->names_.find( name );
            if ( filter->black_ )
            {
                if ( iter != filter->names_.end() )
                {
                    continue;
                }
            }
            else
            {
                if ( iter == filter->names_.end() )
                {
                    continue;
                }
            }
        }

        // null判断
        std::string value = "null";
        if( param.null_.find( name ) == param.null_.end() )
        {
            value = one->serialize( obj );
        }
        
        if ( keys.empty() )
        {
            keys = name;
            values = value;
        }
        else
        {
            keys += "," + name;
            values += "," + value;
        }
    }

    std::stringstream ss;
    ss << "insert into " << cls_name << " (" << keys << ") values (" << values << ");";
    return ss.str();
}

std::string SQLiteDB::update_tbl_sql(
                        void* obj,
                        const char* cls_name,
                        std::vector<ClsMemberReflectorSPtr> const& members,
                        DBModifyParam const& param )
{
    TARO_ASSERT( obj && STRING_CHECK( cls_name ) && !members.empty() );

    // 组装更新信息
    std::string update_str;
    for ( auto const& one : members )
    {
        auto const& name = one->get_name();

        // 过滤判断
        auto filter = DBFilterImpl::get( param.filter_ );
        if ( nullptr != filter )
        {
            auto iter = filter->names_.find( name );
            if ( filter->black_ )
            {
                if ( iter != filter->names_.end() )
                {
                    continue;
                }
            }
            else
            {
                if ( iter == filter->names_.end() )
                {
                    continue;
                }
            }
        }

        if ( !update_str.empty() )
        {
            update_str += ",";
        }
        update_str += name + "=" + one->serialize( obj ); 
    }

    std::stringstream ss;
    ss << "update " << cls_name << " set " << update_str;
    auto cond = DBCondImpl::to_str( param.cond_ );
    if( !cond.empty() )
    {
        ss << " where " << cond;
    }

    ss << ";";
    return ss.str();
}

std::string SQLiteDB::remove_tbl_sql( const char* cls_name, DBCond const& cond )
{
    std::stringstream ss;
    ss << "delete from " << cls_name;
    auto where_str = DBCondImpl::to_str( cond );
    if( !where_str.empty() )
    {
        ss << " where " << where_str;
    }
    ss << ";";
    return ss.str();
}

NAMESPACE_TARO_DB_END
