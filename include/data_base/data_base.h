
#pragma once

#include "data_base/defs.h"
#include "base/memory/optional.h"
#include "base/serialize/str_serialize.h"

NAMESPACE_TARO_DB_BEGIN

struct DBQueryResultImpl;

// 查询结果对象
class TARO_DLL_EXPORT DBQueryResult
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    DBQueryResult();

    /**
    * @brief 析构函数
    */
    ~DBQueryResult();

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
    bool next();

    /**
    * @brief  获取列头信息
    */
    std::vector<std::string> get_col_header();

PRIVATE: // 私有类型
    
    friend DBQueryResultImpl;

PRIVATE: // 私有函数

    TARO_NO_COPY( DBQueryResult );

    /**
    * @brief  获取列数据
    *
    * @param[in] col 列
    */
    char* get_col_val( int32_t col );

PRIVATE: // 私有变量

    DBQueryResultImpl* impl_;
};

// 数据库接口
TARO_INTERFACE TARO_DLL_EXPORT DataBase
{
PUBLIC: // 公共函数

    /**
    * @brief 虚析构函数
    */
    virtual ~DataBase() = default;

    /**
    * @brief  是否有效
    */
    virtual bool valid() const = 0;

    /**
    * @brief  连接数据库
    *
    * @param[in] uri 数据库资源描述
    */
    virtual int32_t connect( const char* uri ) const = 0;

    /**
    * @brief  断开数据库
    */
    virtual int32_t disconnect() const = 0;

    /**
    * @brief  执行SQL
    *
    * @param[in] sql sql语句
    */
    virtual int32_t excute_cmd( const char* sql ) const = 0;

    /**
    * @brief  执行SQL，后获取自增id
    *
    * @param[in]  sql sql语句
    * @param[out] id  自增id
    */
    virtual int32_t exec_cmd_ret_id( const char* sql, uint64_t& id ) const = 0;

    /**
    * @brief  执行获取指令
    *
    * @param[in] sql sql语句
    */
    virtual Optional<DBQueryResult> query( const char* sql ) = 0;

    /**
    * @brief  开始事务
    */
    virtual int32_t begin_transaction() const = 0;

    /**
    * @brief  提交事务
    */
    virtual int32_t commit_transaction() const = 0;

    /**
    * @brief  回滚事务
    */
    virtual int32_t rollback_transaction() const = 0;
};

NAMESPACE_TARO_DB_END
