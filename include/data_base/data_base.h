
#pragma once

#include "data_base/db_helper.h"
#include "data_base/db_reflector.h"
#include "base/serialize/str_serialize.h"
#include <algorithm>

NAMESPACE_TARO_DB_BEGIN

// 查询结果对象
class TARO_DLL_EXPORT DBQueryResult
{
PUBLIC: // 公共函数

    /**
    * @brief 析构函数
    */
    virtual ~DBQueryResult() = default;

    /**
    * @brief  获取列数据，并转换为指定类型
    * 
    * @param[in] col 列
    */
    template<class T>
    Optional<T> get( int32_t col )
    {
        Optional<T> ret;
        auto* ptr = get_col_val( col );
        if( nullptr == ptr )
        {
            return ret;
        }
        return str_to_value<T>( ptr );
    }

    /**
    * @brief  指向下一行
    * 
    * @return true 有效 false 无效
    */
    virtual bool next() = 0;

    /**
    * @brief  获取列信息
    */
    virtual std::vector<std::string> get_columns() = 0;

    /**
    * @brief  获取列数据
    *
    * @param[in] col 列
    */
    virtual char* get_col_val( int32_t col ) = 0;
};

using DBQueryResultSPtr = std::shared_ptr<DBQueryResult>;

// 数据库接口
TARO_INTERFACE TARO_DLL_EXPORT DataBase
{
PUBLIC: // 公共函数

    /**
    * @brief 虚析构函数
    */
    virtual ~DataBase() = default;

    /**
    * @brief  连接数据库
    *
    * @param[in] uri 数据库资源描述
    */
    virtual int32_t connect( const char* uri ) = 0;

    /**
    * @brief  断开数据库
    */
    virtual int32_t disconnect() = 0;

    /**
    * @brief  执行SQL
    *
    * @param[in] sql sql语句
    */
    virtual int32_t excute_cmd( const char* sql ) = 0;

    /**
    * @brief  执行SQL，后获取自增id
    *
    * @param[in]  sql sql语句
    * @param[out] id  自增id
    */
    virtual int32_t exec_cmd_ret_id( const char* sql, uint64_t& id ) = 0;

    /**
    * @brief  执行获取指令
    *
    * @param[in] sql sql语句
    */
    virtual DBQueryResultSPtr query( const char* sql ) = 0;

    /**
    * @brief  开始事务
    */
    virtual int32_t begin_transaction() = 0;

    /**
    * @brief  提交事务
    */
    virtual int32_t commit_transaction() = 0;

    /**
    * @brief  回滚事务
    */
    virtual int32_t rollback_transaction() = 0;

    /**
     * @brief 建表
    */
    template<typename T, typename... Args>
    int32_t create_table( Args&&... args )
    {
        // 获取类型的所有信息
        std::string name;
        std::vector<ClsMemberReflectorSPtr> members;
        get_cls_info<T>( name, members );
        TARO_ASSERT( !name.empty() && members.size() );

        // 解析参数
        CreateTblConstraint constraint;
        expand_create_param( constraint, std::forward<Args>( args )... );

        // 转换为SQL语句
        auto cmd = create_tbl_sql( name.c_str(), members, constraint );
        if ( cmd.empty() )
        {
            DB_ERROR << "compose create table sql failed";
            return TARO_ERR_INVALID_ARG;
        }

        // 执行SQL命令
        if ( TARO_OK != excute_cmd( cmd.c_str() ) )
        {
            DB_ERROR << "create failed. sql:" << cmd;
            return TARO_ERR_FAILED;
        }
        return TARO_OK;
    }

    /**
     * @brief 删除表
    */
    template<typename T>
    int32_t drop_table()
    {
        // 查询表名
        auto type = &typeid( T );
        DBReflector& inst = DBReflector::instance();
        auto name = inst.find_class_name( type );
        if( name.empty() )
        {
            // 没有查询到映射关系，则调用建立映射函数
            T::db_cls_reflect();
            name = inst.find_class_name( type );
        }
        TARO_ASSERT( !name.empty() );

        std::string cmd = "drop table ";
        cmd += name;

        // 执行SQL命令
        if ( TARO_OK != excute_cmd( cmd.c_str() ) )
        {
            DB_ERROR << "insert failed. sql:" << cmd;
            return TARO_ERR_FAILED;
        }
        return TARO_OK;
    }

    /**
     * @brief 查询
    */
    template<typename T, typename... Args>
    std::vector<T> query( Args&&... args )
    {
        // 获取类型的所有信息
        std::string name;
        std::vector<ClsMemberReflectorSPtr> members;
        get_cls_info<T>( name, members );
        TARO_ASSERT( !name.empty() && members.size() );

        // 解析参数
        DBQueryParam param;
        expand_query_param( param, std::forward<Args>( args )... );

        // 转换为SQL语句
        std::vector<T> query_result;
        auto cmd = query_tbl_sql( name.c_str(), members, param );
        if ( cmd.empty() )
        {
            DB_ERROR << "compose insert table sql failed";
            return query_result;
        }

        // 执行SQL命令
        auto result = query( cmd.c_str() );
        if ( result == nullptr )
        {
            return query_result;
        }

        // 反序列化数据
        auto cols = result->get_columns();
        if ( cols.empty() )
        {
            DB_ERROR << "get columns failed";
            return query_result;
        }

        do{
            T element;
            for( size_t i = 0; i < cols.size(); ++i )
            {
                auto iter = std::find_if( members.begin(), members.end(), [&]( ClsMemberReflectorSPtr const& ptr )
                {
                    return ptr->get_name() == cols[i];
                } );
                TARO_ASSERT( iter != members.end() );
                ( *iter )->deserialize( ( const char* )result->get_col_val( ( int32_t )i ), ( void* )&element );
            }
            query_result.emplace_back( element );
        }while( result->next() );
        
        return query_result;
    }

    /**
     * @brief 插入数据
    */
    template<typename T, typename... Args>
    int32_t insert( T const& obj, Args&&... args )
    {
        // 获取类型的所有信息
        std::string name;
        std::vector<ClsMemberReflectorSPtr> members;
        get_cls_info<T>( name, members );
        TARO_ASSERT( !name.empty() && members.size() );

        // 解析参数
        DBModifyParam param;
        expand_modify_param( param, std::forward<Args>( args )... );

        // 转换为SQL语句
        auto cmd = insert_tbl_sql( ( void* )&obj, name.c_str(), members, param );
        if ( cmd.empty() )
        {
            DB_ERROR << "compose insert table sql failed";
            return TARO_ERR_INVALID_ARG;
        }

        // 执行SQL命令
        if ( TARO_OK != excute_cmd( cmd.c_str() ) )
        {
            DB_ERROR << "insert failed. sql:" << cmd;
            return TARO_ERR_FAILED;
        }
        return TARO_OK;
    }

    /**
     * @brief 更新数据
    */
    template<typename T, typename... Args>
    int32_t update( T const& obj, Args&&... args )
    {
        // 获取类型的所有信息
        std::string name;
        std::vector<ClsMemberReflectorSPtr> members;
        get_cls_info<T>( name, members );
        TARO_ASSERT( !name.empty() && members.size() );

        // 解析参数
        DBModifyParam param;
        expand_modify_param( param, std::forward<Args>( args )... );

        // 转换为SQL语句
        auto cmd = update_tbl_sql( ( void* )&obj, name.c_str(), members, param );
        if( cmd.empty() )
        {
            DB_ERROR << "compose insert table sql failed";
            return TARO_ERR_INVALID_ARG;
        }

        // 执行SQL命令
        if( TARO_OK != excute_cmd( cmd.c_str() ) )
        {
            DB_ERROR << "insert failed. sql:" << cmd;
            return TARO_ERR_FAILED;
        }
        return TARO_OK;
    }

    /*
    * @brief 删除指定行
    */
    template<typename T>
    int32_t remove( DBCond const& cond = DBCond() )
    {
        auto type = &typeid( T );
        DBReflector& inst = DBReflector::instance();
        auto name = inst.find_class_name( type );
        if( name.empty() )
        {
            // 没有查询到映射关系，则调用建立映射函数
            T::db_cls_reflect();
            name = inst.find_class_name( type );
        }
        TARO_ASSERT( !name.empty() );

        // 转换为SQL语句
        auto cmd = remove_tbl_sql( name.c_str(), cond );
        if( cmd.empty() )
        {
            DB_ERROR << "compose remove table sql failed";
            return TARO_ERR_INVALID_ARG;
        }

        // 执行SQL命令
        if( TARO_OK != excute_cmd( cmd.c_str() ) )
        {
            DB_ERROR << "remove failed. sql:" << cmd;
            return TARO_ERR_FAILED;
        }
        return TARO_OK;
    }

PROTECTED: // 保护函数

    /**
     * @brief 组装建表SQL
    */
    virtual std::string create_tbl_sql( 
                        const char* cls_name, 
                        std::vector<ClsMemberReflectorSPtr> const& members,
                        CreateTblConstraint const& constraint ) = 0;

    /**
     * @brief 组装查询SQL
    */
    virtual std::string query_tbl_sql( 
                        const char* cls_name, 
                        std::vector<ClsMemberReflectorSPtr> const& members,
                        DBQueryParam const& param ) = 0;

    /**
     * @brief 组装插入SQL
    */
    virtual std::string insert_tbl_sql( 
                        void* obj,
                        const char* cls_name, 
                        std::vector<ClsMemberReflectorSPtr> const& members,
                        DBModifyParam const& param ) = 0;

    /**
     * @brief 组装更新SQL
    */
    virtual std::string update_tbl_sql(
                        void* obj,
                        const char* cls_name,
                        std::vector<ClsMemberReflectorSPtr> const& members,
                        DBModifyParam const& param ) = 0;

    /**
     * @brief 组装删除SQL
    */
    virtual std::string remove_tbl_sql( const char* cls_name, DBCond const& cond ) = 0;
    
PRIVATE: // 私有函数

    /**
     * @brief 获取类型信息
     * 
     * @param[out] name    类型名称
     * @param[out] members 类成员反射对象
    */
    template<typename T>
    void get_cls_info( std::string& name, std::vector<ClsMemberReflectorSPtr>& members )
    {
        auto type = &typeid( T );
        DBReflector& inst = DBReflector::instance();
        name = inst.find_class_name( type );
        if( name.empty() )
        {
            // 没有查询到映射关系，则调用建立映射函数
            T::db_cls_reflect();
            name = inst.find_class_name( type );
        }
        TARO_ASSERT( !name.empty() );
        members = inst.get_member_reflectors( type );
    }
};

using DataBaseSPtr = std::shared_ptr<DataBase>;

// 数据库类型定义
#define DB_TYPE_SQLITE "sqlite"

/**
* @brief  创建Database
*/
extern TARO_DLL_EXPORT DataBaseSPtr create_database( const char* type = DB_TYPE_SQLITE );

NAMESPACE_TARO_DB_END
