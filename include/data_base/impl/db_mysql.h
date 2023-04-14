
#pragma once

#include "mysql/mysql.h"
#include "data_base/impl/db_sql.h"

NAMESPACE_TARO_DB_BEGIN

class MySQLDBResult : PUBLIC DBQueryResult
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] result 查询结果对象
    */
    MySQLDBResult( MYSQL_RES* result );

    /**
    * @brief 析构函数
    */
    ~MySQLDBResult();

    /**
    * @brief 是否有效
    */
    bool valid() const;

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

    MYSQL_RES* result_;
    MYSQL_ROW  row_;
    int32_t    max_col_;
};

class MySQLDB : PUBLIC DataBaseSQL
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    MySQLDB();

    /**
    * @brief 析构函数
    */
    ~MySQLDB();

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
    
    MYSQL handler_;
    bool  connect_;
};

NAMESPACE_TARO_DB_END
