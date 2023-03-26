
#pragma once

#include "data_base/defs.h"
#include "base/memory/optional.h"
#include <string>

NAMESPACE_TARO_DB_BEGIN

// 数据库的约束条件类型
enum EDBConstraint
{
    eDBConstraintUnique,
    eDBConstraintPrimaryKey,
    eDBConstraintForeignKey,
    eDBConstraintDefault,
    eDBConstraintDefaultValue,
    eDBConstraintAutoInc,
    eDBConstraintCheck,
};

struct DBContraintImpl;

// 数据库列的约束条件
struct TARO_DLL_EXPORT DBContraint
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] name 变量名称
    */
    DBContraint( const char* name );

    /**
    * @brief 构造函数
    */
    DBContraint( DBContraint&& other );

    /**
    * @brief 虚析构函数
    */
    ~DBContraint();

    /**
    * @brief 非空约束
    */
    DBContraint& not_null();

    /**
    * @brief 唯一约束
    */
    DBContraint& unique();

    /**
    * @brief 主键
    */
    DBContraint& primary_key();

    /**
    * @brief 外键
    */
    DBContraint& foreign_key( const char* tbl, const char* col );

    /**
    * @brief 序号自动加一
    */
    DBContraint& auto_inc();

    /**
    * @brief 默认值
    */
    DBContraint& default_val();

    /**
    * @brief 默认值
    */
    template< typename T >
    DBContraint& default_val( T&& val );

PRIVATE: // 私有类型

    friend struct DBContraintImpl;

PRIVATE: // 私有函数

    TARO_NO_COPY( DBContraint );

PRIVATE: // 私有变量

    DBContraintImpl* impl_;
};
using DBContraintSPtr = std::shared_ptr< DBContraint >;

// 建表约束
struct CreateTblConstraint
{
    Optional<bool> create_when_no_exit; // 若表存在则不建新表
    std::vector< DBContraintSPtr > contrains;  // 列的约束条件集合
};

struct create_if_not_exist {}; 

template<typename T>
struct set_create_tbl_param {};

template<>
struct set_create_tbl_param<DBContraint>
{
    static void set( CreateTblConstraint& params, DBContraint&& cstr )
    {
        params.contrains.emplace_back( cstr );
    }
};

template<>
struct set_create_tbl_param<create_if_not_exist>
{
    static void set( CreateTblConstraint& params, create_if_not_exist&& )
    {
        params.create_when_no_exit = true;
    }
};

inline void expand_create_param( CreateTblConstraint& params )
{

}

template< typename T >
void expand_create_param( CreateTblConstraint& params, T&& t )
{
    set_create_tbl_param< typename UnWrapperParam<T>::type >::set( params, std::forward<T>( t ) );
}

template< typename T, typename... Args >
void expand_create_param( CreateTblConstraint& params, T&& t, Args&&... args )
{
    set_create_tbl_param< typename UnWrapperParam<T>::type >::set( params, std::forward<T>( t ) );
    expand_create_param( params, std::forward<Args>( args )... );
}

NAMESPACE_TARO_DB_END
