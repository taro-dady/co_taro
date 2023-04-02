
#pragma once

#include "data_base/defs.h"
#include "base/memory/optional.h"
#include "base/serialize/type_traits.h"
#include <vector>
#include <string>

NAMESPACE_TARO_DB_BEGIN

// 数据库的约束条件类型
enum EDBConstraint
{
    eDBConstraintNotNull,
    eDBConstraintUnique,
    eDBConstraintPrimaryKey,
    eDBConstraintForeignKey,
    eDBConstraintDefault,
    eDBConstraintDefaultValue,
    eDBConstraintAutoInc,
    eDBConstraintCheck,
};

// 约束条件的参数
struct TARO_DLL_EXPORT DBContraintArgBase
{
PUBLIC: // 公共函数

    /**
     * @brief 构造函数
    */
    DBContraintArgBase( EDBConstraint const& kind )
        : kind_( kind )
    {

    }

    /**
     * @brief 析构函数
    */
    virtual ~DBContraintArgBase() = default;

    /**
     * @brief 获取类型
    */
    EDBConstraint const& kind() const
    {
        return kind_;
    }

PRIVATE: // 私有变量

    EDBConstraint kind_; // 约束类型
};
using DBContraintArgBaseSPtr = std::shared_ptr< DBContraintArgBase >;

// 带有参数的约束对象
struct DBContraintArg : PUBLIC DBContraintArgBase
{
PUBLIC: // 公共函数

    /**
     * @brief 构造函数
    */
    DBContraintArg( EDBConstraint const& kind, std::string const& val )
        : DBContraintArgBase( kind )
        , val_( val )
    {

    }

    /**
     * @brief 获取参数值
    */
    std::string const& get() const
    {
        return val_;
    }

PRIVATE: // 私有变量

    std::string val_;
};

// 构造带有参数的约束对象
template<typename... Args>
typename std::enable_if< sizeof...( Args ) != 0, DBContraintArgBaseSPtr >::type
make_constraint( EDBConstraint const& kind, Args&&... args )
{
    return std::make_shared< DBContraintArg >( kind, std::forward<Args>( args )... );
}

// 构造无参数的约束对象
template<typename... Args>
typename std::enable_if< sizeof...( Args ) == 0, DBContraintArgBaseSPtr >::type
make_constraint( EDBConstraint const& kind, Args&&... args )
{
    return std::make_shared<DBContraintArgBase>( kind );
}

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
    * @brief 获取约束名称
    */
    std::string const& name() const;

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
    DBContraint& foreign_key( std::string const& tbl, std::string const& col );

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
    DBContraint& default_val( T&& val )
    {
        std::stringstream ss;
        ss << val;
        add_constraint( make_constraint( eDBConstraintDefaultValue, ss.str() ) );
        return *this;
    }

PRIVATE: // 私有类型

    friend struct DBContraintImpl;

PRIVATE: // 私有函数

    TARO_NO_COPY( DBContraint );

    /**
    * @brief 添加约束
    */
    void add_constraint( DBContraintArgBaseSPtr const& arg );

PRIVATE: // 私有变量

    DBContraintImpl* impl_;
};

// 建表约束
struct CreateTblConstraint
{
    Optional<bool> create_when_no_exit; // 若表存在则不建新表
    std::vector< DBContraint > contrains;  // 列的约束条件集合
};

struct create_if_not_exist {};

template<typename T>
struct set_create_tbl_param {};

template<>
struct set_create_tbl_param<DBContraint>
{
    static void set( CreateTblConstraint& params, DBContraint&& cstr )
    {
        params.contrains.emplace_back( std::move( cstr ) );
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
    set_create_tbl_param< typename UnWrapperParam<T>::type >::set( params, std::move( t ) );
}

template< typename T, typename... Args >
void expand_create_param( CreateTblConstraint& params, T&& t, Args&&... args )
{
    set_create_tbl_param< typename UnWrapperParam<T>::type >::set( params, std::move( t ) );
    expand_create_param( params, std::forward<Args>( args )... );
}

// 获取成员名称 如A::member 获取member
inline const char* get_class_member( const char* n )
{
    const char *ptr = nullptr, *tmp = n;
    while( ( tmp = strstr( tmp, "::" ) ) != nullptr )
    {
        ptr = tmp;
        tmp = tmp + 2;
    }
    TARO_ASSERT( ptr != nullptr );
    return ptr + 2;
}

NAMESPACE_TARO_DB_END

// 获取名称
#define DB_MEM( x ) \
    taro::db::get_class_member( #x )

// 约束
#define DB_CSTR( x ) \
    taro::db::DBContraint( DB_MEM( x ) )
