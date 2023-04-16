
#include "data_base/impl/db_sql.h"
#include "base/utils/string_tool.h"
#include "data_base/impl/db_helper_impl.h"

NAMESPACE_TARO_DB_BEGIN

std::string DataBaseSQL::create_tbl_sql( const char* cls_name,
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

    std::string one_primary;
    std::map< std::string, std::string > col_decl;
    for( auto& col : members )
    {
        // 列类型转换为字符串
        std::string type;
        if( !type_to_str( col->get_type(), type ) )
        {
            DB_ERROR << "column:" << col->get_name() << " type format error";
            return "";
        }

        bool primary = false;
        std::string value = type;
        if( primar_keys.size() == 1 && ( *primar_keys.begin() ) == col->get_name() )
        {
            value += " ";
            value += "primary key";
            primary = true;
        }

        // 列约束转换为字符串
        auto iter = col_cstrs.find( col->get_name() );
        if( iter != col_cstrs.end() )
        {
            for( size_t i = 0; i < iter->second.size(); ++i )
            {
                if ( iter->second[i]->kind() == eDBConstraintAutoInc )
                {
                    TARO_ASSERT( primary && type == int_str_, "sqlite auto inc must be primary key and int" );
                }
                value += " " + constr_to_str( iter->second[i] );
            }
        }

        if( !primary )
            col_decl[col->get_name()] = value;
        else
            one_primary += col->get_name() + " " + value;
    }

    /// 组织SQL
    std::stringstream ss;
    ss << "create table ";
    if( constraint.create_when_no_exit.valid() )
    {
        ss << "if not exists ";
    }
    ss << cls_name << "(";

    // 组装列信息
    std::string col_str;
    if ( !one_primary.empty() )
    {
        col_str += one_primary + ",";
    }

    for( auto& one : col_decl )
    {
        col_str += one.first + " " + one.second + ",";
    }
    ss << string_trim_back( col_str, "," );
    
    // 多主键
    if ( primar_keys.size() >= 2 )
    {
        ss << ",primary key(";
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
    }
    
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

std::string DataBaseSQL::query_tbl_sql( 
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

std::string DataBaseSQL::insert_tbl_sql( 
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

std::string DataBaseSQL::update_tbl_sql(
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

std::string DataBaseSQL::remove_tbl_sql( const char* cls_name, DBCond const& cond )
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

std::string DataBaseSQL::average_tbl_sql( const char* cls_name, const char* col, DBCond const& cond )
{
    std::stringstream ss;
    ss << "select avg(" << col << ") from " << cls_name;
    auto where_str = DBCondImpl::to_str( cond );
    if( !where_str.empty() )
    {
        ss << " where " << where_str;
    }
    ss << ";";
    return ss.str();
}

std::string DataBaseSQL::sum_tbl_sql( const char* cls_name, const char* col, DBCond const& cond )
{
    std::stringstream ss;
    ss << "select sum(" << col << ") from " << cls_name;
    auto where_str = DBCondImpl::to_str( cond );
    if( !where_str.empty() )
    {
        ss << " where " << where_str;
    }
    ss << ";";
    return ss.str();
}

std::string DataBaseSQL::count_tbl_sql( const char* cls_name, DBCond const& cond )
{
    std::stringstream ss;
    ss << "select count(*) from " << cls_name;
    auto where_str = DBCondImpl::to_str( cond );
    if( !where_str.empty() )
    {
        ss << " where " << where_str;
    }
    ss << ";";
    return ss.str();
}

std::string DataBaseSQL::constr_to_str( DBContraintArgBaseSPtr const& base_ptr )
{
    TARO_ASSERT( base_ptr != nullptr );

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
        return auto_inc_str_;
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

bool DataBaseSQL::is_string_type( const std::type_info* type )
{
    return ( *type == typeid( std::string ) || *type == typeid( const char* ) || *type == typeid( char* ) );
}

bool DataBaseSQL::type_to_str( const std::type_info* p, std::string& tstr )
{
    TARO_ASSERT( p != nullptr );

    if( is_string_type( p ) )
    {
        tstr = var_char_str_;
    }
    else if( *p == typeid( int32_t )
        || *p == typeid( int16_t )
        || *p == typeid( int8_t )
        || *p == typeid( uint32_t )
        || *p == typeid( uint16_t )
        || *p == typeid( uint8_t ) )
    {
        tstr = int_str_;
    }
    else if( *p == typeid( int64_t ) )
    {
        tstr = "bigint";
    }
    else if( *p == typeid( uint64_t ) )
    {
        tstr = "unsigned big int";
    }
    else if( *p == typeid( float ) )
    {
        tstr = float_str_;
    }
    else if( *p == typeid( double ) )
    {
        tstr = double_str_;
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

NAMESPACE_TARO_DB_END
