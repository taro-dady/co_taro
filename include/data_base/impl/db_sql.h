
#pragma once

#include "data_base/data_base.h"

NAMESPACE_TARO_DB_BEGIN

class DataBaseSQL : PUBLIC DataBase
{
PUBLIC: // 公共函数

    /**
     * @brief 析构函数
    */
    virtual ~DataBaseSQL() = default;

PROTECTED: // 保护函数

    /**
     * @brief 组装建表的SQL
    */
    virtual std::string create_tbl_sql( 
                        const char* cls_name, 
                        std::vector<ClsMemberReflectorSPtr> const& members,
                        CreateTblConstraint const& constraint ) override;

    /**
     * @brief 组装查询SQL
    */
    virtual std::string query_tbl_sql( 
                        const char* cls_name, 
                        std::vector<ClsMemberReflectorSPtr> const& members,
                        DBQueryParam const& param ) override;

    /**
     * @brief 组装插入的SQL
    */
    virtual std::string insert_tbl_sql( 
                        void* obj,
                        const char* cls_name, 
                        std::vector<ClsMemberReflectorSPtr> const& members,
                        DBModifyParam const& param ) override;

    /**
     * @brief 组装更新SQL
    */
    virtual std::string update_tbl_sql(
                        void* obj,
                        const char* cls_name,
                        std::vector<ClsMemberReflectorSPtr> const& members,
                        DBModifyParam const& param ) override;

    /**
     * @brief 组装删除SQL
    */
    virtual std::string remove_tbl_sql( const char* cls_name, DBCond const& cond ) override;

    /**
     * @brief 计算平均值
    */
    virtual std::string average_tbl_sql( const char* cls_name, const char* col, DBCond const& cond ) override;

    /**
     * @brief 计算总值
    */
    virtual std::string sum_tbl_sql( const char* cls_name, const char* col, DBCond const& cond ) override;

    /**
     * @brief 计算数量
    */
    virtual std::string count_tbl_sql( const char* cls_name, DBCond const& cond ) override;
    
    /**
     * @brief 约束条件转换为字符
    */
    virtual std::string constr_to_str( DBContraintArgBaseSPtr const& base_ptr );

    /**
     * @brief 判断是否为字符型
    */
    virtual bool is_string_type( const std::type_info* type );

    /**
     * @brief 类型转换为数据库对应的字符
    */
    virtual bool type_to_str( const std::type_info* p, std::string& tstr );

PROTECTED: // 保护变量

    std::string auto_inc_str_;
    std::string var_char_str_;
    std::string int_str_;
    std::string float_str_;
    std::string double_str_;
};

NAMESPACE_TARO_DB_END
