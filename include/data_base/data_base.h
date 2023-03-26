
#pragma once

#include "data_base/db_helper.h"
#include "data_base/db_reflector.h"
#include "base/serialize/str_serialize.h"

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

PROTECTED: // 保护函数

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
    bool create_table( Args... args )
    {
        std::string name;
        std::vector<ClsMemberReflectorSPtr> members;
        get_cls_info( name, members );
        TARO_ASSERT( !name.empty() && members.size() );

        std::string cmd;

        if ( TARO_OK != excute_cmd( cmd.c_str() ) )
        {
            DB_ERROR << "create failed. sql:" << cmd;
            return false;
        }
        return true;
    }

PROTECTED: // 保护函数

    /**
     * @brief 组装建表的SQL
    */
    virtual std::string create_tbl_sql( 
                        const char* cls_name, 
                        std::vector<ClsMemberReflectorSPtr> const& members,
                        CreateTblConstraint const& constraint ) = 0;
    
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
            T::db_cls_reflect();
        }
        name = inst.find_class_name( type );
        members = inst.get_member_reflectors( type );
    }
};

using DataBaseSPtr = std::shared_ptr<DataBase>;

// 数据库类型定义
#define DB_TYPE_SQLITE "sqlite"

/**
* @brief  创建Database
*/
extern TARO_DLL_EXPORT DataBaseSPtr create_database( const char* type );

NAMESPACE_TARO_DB_END
