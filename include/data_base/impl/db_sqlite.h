﻿
#pragma once

#include "sqlite/sqlite3.h"
#include "data_base/impl/db_sql.h"

NAMESPACE_TARO_DB_BEGIN

class SQLiteDBResult : PUBLIC DBQueryResult
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] stmt 查询结果对象
    */
    SQLiteDBResult( sqlite3_stmt* stmt );

    /**
    * @brief 析构函数
    */
    ~SQLiteDBResult();

PRIVATE: // 私有函数

    /**
    * @brief  指向下一行
    * 
    * @return true 有效 false 无效
    */
    virtual bool next() override;

    /**
    * @brief  获取列数据
    *
    * @param[in] col 列
    */
    virtual char* get_col_val( int32_t col ) override;

    /**
    * @brief  获取列信息
    */
    virtual std::vector<std::string> get_columns() override;

PRIVATE: // 私有变量

    sqlite3_stmt* stmt_;
};

class SQLiteDB : PUBLIC DataBaseSQL
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    SQLiteDB();

    /**
    * @brief 析构函数
    */
    ~SQLiteDB();

    /**
    * @brief  连接数据库
    *
    * @param[in] uri 数据库资源描述
    */
    virtual int32_t connect( DBUri const& uri ) override;

    /**
    * @brief  执行SQL
    *
    * @param[in] sql sql语句
    */
    virtual int32_t excute_cmd( const char* sql ) override;

    /**
    * @brief  执行SQL，后获取自增id
    *
    * @param[in]  sql sql语句
    * @param[out] id  自增id
    */
    virtual int32_t exec_cmd_ret_id( const char* sql, uint64_t& id ) override;

    /**
    * @brief  执行获取指令
    *
    * @param[in] sql sql语句
    */
    virtual DBQueryResultSPtr query( const char* sql ) override;

    /**
    * @brief  开始事务
    */
    virtual int32_t begin_transaction() override;

    /**
    * @brief  提交事务
    */
    virtual int32_t commit_transaction() override;

    /**
    * @brief  回滚事务
    */
    virtual int32_t rollback_transaction() override;

PRIVATE: // 私有函数

    /**
     * @brief 初始化SQL相关参数
    */
    void init_sql_param();

PRIVATE: // 私有变量

    sqlite3* handler_;
};

NAMESPACE_TARO_DB_END
