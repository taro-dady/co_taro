
#pragma once

#include "data_base/defs.h"
#include "base/memory/optional.h"
#include "base/serialize/type_traits.h"
#include <set>
#include <vector>
#include <string>

NAMESPACE_TARO_DB_BEGIN

// URI类型
enum EDBUriType
{
    eDBUriHost,
    eDBUriUser,
    eDBUriPwd,
    eDBUriDB,
    eDBUriFile,
    eDBUriPort,
};

template<typename ValueType, EDBUriType Type>
struct DBUriOpt
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    DBUriOpt( ValueType const& value )
        : value_( value )
        , type_( Type )
    {}

    /**
    * @brief 获取值
    */
    ValueType const& value() const
    {
        return value_;
    }

PRIVATE: // 私有变量

    ValueType  value_;
    EDBUriType type_;
};

using DBUriOptHost = DBUriOpt<std::string, eDBUriHost>;
using DBUriOptUser = DBUriOpt<std::string, eDBUriUser>;
using DBUriOptPwd  = DBUriOpt<std::string, eDBUriPwd>;
using DBUriOptDB   = DBUriOpt<std::string, eDBUriDB>;
using DBUriOptFile = DBUriOpt<std::string, eDBUriFile>;
using DBUriOptPort = DBUriOpt<uint16_t, eDBUriPort>;

#define db_opt_host db::DBUriOptHost
#define db_opt_port db::DBUriOptPort
#define db_opt_user db::DBUriOptUser
#define db_opt_pwd  db::DBUriOptPwd
#define db_opt_db   db::DBUriOptDB
#define db_opt_file db::DBUriOptFile

// 数据库URI
class TARO_DLL_EXPORT DBUri
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    DBUri();

    /**
    * @brief 构造函数
    */
    DBUri( const char* value, EDBUriType const& type = eDBUriFile );

    /**
    * @brief 析构函数
    */
    ~DBUri();

    /**
    * @brief 设置host
    */
    DBUri& operator<<( DBUriOptHost const& arg );

    /**
    * @brief 设置user
    */
    DBUri& operator<<( DBUriOptUser const& arg );

    /**
    * @brief 设置pwd
    */
    DBUri& operator<<( DBUriOptPwd const& arg );

    /**
    * @brief 设置database
    */
    DBUri& operator<<( DBUriOptDB const& arg );

    /**
    * @brief 设置file
    */
    DBUri& operator<<( DBUriOptFile const& arg );

    /**
    * @brief 设置端口
    */
    DBUri& operator<<( DBUriOptPort const& arg );

    /**
    * @brief 获取参数引用
    */
    std::string operator[]( EDBUriType const& type ) const;
    
    /**
    * @brief 获取端口引用
    */
    Optional<uint16_t>& port() const;

PRIVATE: // 私有函数

    TARO_NO_COPY( DBUri );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

// 数据库格式变换
struct TARO_DLL_EXPORT DBFormat
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    DBFormat( const char* sql );

    /**
    * @brief 替换?
    */
    template<typename T>
    DBFormat& operator<<( T value )
    {
        replace( value_to_str( value ) );
        return *this;
    }

    /**
    * @brief 获取SQL语句
    */
    operator std::string() const;

PRIVATE: // 私有函数    

    TARO_NO_COPY( DBFormat );

    /**
    * @brief 替换?
    */
    void replace( std::string const& value );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

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

// 条件的内部变量
struct DBCondImpl;

// 条件描述
struct TARO_DLL_EXPORT DBCond
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    DBCond();

    /**
    * @brief 构造函数
    * 
    * @param[in] name 变量名称
    */
    DBCond( const char* name );

    /**
    * @brief 构造函数
    * 
    * @param[in] other 其他对象
    */
    DBCond( DBCond&& other );

    /**
    * @brief 析构函数
    */
    ~DBCond();

    /**
    * @brief 移动赋值
    * 
    * @param[in] other 其他对象
    */
    DBCond& operator=( DBCond&& other );

    /**
    * @brief 是否为空
    * 
    * @param[in] null true 为空 false 不为空
    */
    DBCond& is_null( bool null = true );

    /**
    * @brief 相等条件
    * 
    * @param[in] a 比较的值 参数必须传值否则在字符串时会导致模板特化错误，比如 = "jack" 就会被特化为<char [5]>
    */
    template<typename A>
    DBCond& operator=( A a )
    {
        set_value( "=", value_to_str( a ) );
        return *this;
    }

    /**
    * @brief 小于比较
    * 
    * @param[in] a 比较的值
    */
    template<typename A>
    DBCond& operator<( A a )
    {
        set_value( "<", value_to_str( a ) );
        return *this;
    }

    /**
    * @brief 小于等于比较
    * 
    * @param[in] a 比较的值
    */
    template<typename A>
    DBCond& operator<=( A a )
    {
        set_value( "<=", value_to_str( a ) );
        return *this;
    }

    /**
    * @brief 大于比较
    * 
    * @param[in] a 比较的值
    */
    template<typename A>
    DBCond& operator>( A a )
    {
        set_value( ">", value_to_str( a ) );
        return *this;
    }

    /**
    * @brief 大于等于比较
    * 
    * @param[in] a 比较的值
    */
    template<typename A>
    DBCond& operator>=( A a )
    {
        set_value( ">=", value_to_str( a ) );
        return *this;
    }

    /**
    * @brief 条件的与关系
    * 
    * @param[in] c 条件对象
    */
    DBCond& operator&&( DBCond const& c );

    /**
    * @brief 条件的或关系
    * 
    * @param[in] c 条件对象
    */
    DBCond& operator||( DBCond const& c );

PRIVATE: // 私有类型

    friend struct DBCondImpl;

PRIVATE: // 私有函数

    TARO_NO_COPY( DBCond );

    /**
    * @brief 设置操作符与值
    * 
    * @param[in] op  操作符
    * @param[in] val 值
    */
    void set_value( std::string const& op, std::string const& val );

PRIVATE: // 私有变量

    DBCondImpl* impl_;
};

struct DBFilterImpl;

// 过滤器，用于操作结构体的部分数据
struct TARO_DLL_EXPORT DBFilter
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    DBFilter();

    /**
    * @brief 构造函数
    * 
    * @param[in] black true 表示黑名单 false 表示白名单
    */
    DBFilter( bool black );

    /**
    * @brief 构造函数
    * 
    * @param[in] other 其他对象
    */
    DBFilter( DBFilter&& other );

    /**
    * @brief 析构函数
    */
    ~DBFilter();

    /**
    * @brief 移动赋值
    * 
    * @param[in] other 其他对象
    */
    DBFilter& operator=( DBFilter&& other );

    /**
    * @brief 添加参数
    * 
    * @param[in] name 参数名称
    */
    DBFilter& operator<<( std::string const& name );

PRIVATE: // 私有类型

    friend struct DBFilterImpl;

PRIVATE: // 私有函数

    TARO_NO_COPY( DBFilter );

PRIVATE: // 私有变量

    DBFilterImpl* impl_;
};

// 指定操作范围
struct TARO_DLL_EXPORT DBLimit
{
    /**
    * @brief 构造函数
    * 
    * @param[in] cnt    数量
    * @param[in] offset 开始位置
    */
    DBLimit( int32_t cnt = 0, int32_t offset = -1 )
        : offset_( offset )
        , count_( cnt )
    {}

    int32_t offset_; // 开始位置
    int32_t count_;  // 数量
};

struct TARO_DLL_EXPORT DBNull
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] name 参数名称
    */
    DBNull( const char* name );

    /**
    * @brief 析构函数
    */
    ~DBNull();

    /**
    * @brief 获取名称
    */
    const char* name();

PRIVATE: // 私有函数

    TARO_NO_COPY( DBNull );

PRIVATE: // 私有类型

    struct Impl;
    Impl* impl_;
};

struct TARO_DLL_EXPORT DBOrder
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] name 参数名称
    * @param[in] asc  升序
    */
    DBOrder( const char* name, bool asc = true );

    /**
    * @brief 析构函数
    */
    ~DBOrder();

    /**
    * @brief 获取名称
    */
    const char* name();

    /**
    * @brief 是否为升序
    */
    bool ascend() const;

PRIVATE: // 私有函数

    TARO_NO_COPY( DBOrder );

PRIVATE: // 私有类型

    struct Impl;
    Impl* impl_;
};

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

/************************* create table ********************************************/
// 建表约束
struct CreateTblConstraint
{
    Optional<bool> create_when_no_exit;    // 若表存在则不建新表
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

/************************* modify table ********************************************/
struct DBModifyParam
{
    DBCond cond_; // 条件变量
    DBFilter filter_; // 过滤器
    std::set<std::string> null_; // 空值
};

template<typename T>
struct set_modify_tbl_param {};

template<>
struct set_modify_tbl_param<DBCond>
{
    static void set( DBModifyParam& params, DBCond&& c )
    {
        params.cond_ = std::move( c );
    }
};

template<>
struct set_modify_tbl_param<DBFilter>
{
    static void set( DBModifyParam& params, DBFilter&& filter )
    {
        params.filter_ = std::move( filter );
    }
};

template<>
struct set_modify_tbl_param<DBNull>
{
    static void set( DBModifyParam& params, DBNull&& n )
    {
        params.null_.insert( n.name() );
    }
};

inline void expand_modify_param( DBModifyParam& params )
{

}

template< typename T >
void expand_modify_param( DBModifyParam& params, T&& t )
{
    set_modify_tbl_param< typename UnWrapperParam<T>::type >::set( params, std::move( t ) );
}

template< typename T, typename... Args >
void expand_modify_param( DBModifyParam& params, T&& t, Args&&... args )
{
    set_modify_tbl_param< typename UnWrapperParam<T>::type >::set( params, std::move( t ) );
    expand_modify_param( params, std::forward<Args>( args )... );
}

/************************* query table ********************************************/
struct DBQueryParam
{
    DBCond cond_; // 条件
    DBFilter filter_; // 过滤器
    Optional<DBLimit> limit_; // 查询范围
    Optional< std::pair<std::string, bool> > order_; // 升序降序参数
};

template<typename T>
struct set_query_param {};

template<>
struct set_query_param<DBCond>
{
    static void set( DBQueryParam& params, DBCond&& c )
    {
        params.cond_ = std::move( c );
    }
};

template<>
struct set_query_param<DBOrder>
{
    static void set( DBQueryParam& params, DBOrder&& order )
    {
        params.order_ = std::make_pair( order.name(), order.ascend() );
    }
};

template<>
struct set_query_param<DBLimit>
{
    static void set( DBQueryParam& params, DBLimit&& limit )
    {
        params.limit_ = limit;
    }
};

template<>
struct set_query_param<DBFilter>
{
    static void set( DBQueryParam& params, DBFilter&& filter )
    {
        params.filter_ = std::move( filter );
    }
};

inline void expand_query_param( DBQueryParam& params )
{

}

template< typename T >
void expand_query_param( DBQueryParam& params, T&& t )
{
    set_query_param< typename UnWrapperParam<T>::type >::set( params, std::move( t ) );
}

template< typename T, typename... Args >
void expand_query_param( DBQueryParam& params, T&& t, Args&&... args )
{
    set_query_param< typename UnWrapperParam<T>::type >::set( params, std::move( t ) );
    expand_query_param( params, std::forward<Args>( args )... );
}

NAMESPACE_TARO_DB_END

// 获取名称
#define DB_MEM( x ) \
    taro::db::get_class_member( #x )

// 约束
#define DB_CSTR( x ) \
    taro::db::DBContraint( DB_MEM( x ) )

// 条件
#define DB_COND( x ) \
    taro::db::DBCond( DB_MEM( x ) )

// 空值
#define DB_NULL( x ) \
    taro::db::DBNull( DB_MEM( x ) )

// 查询序列
#define DB_ASCEND( x ) \
    taro::db::DBOrder( DB_MEM( x ), true )

#define DB_DESCEND( x ) \
    taro::db::DBOrder( DB_MEM( x ), false )

// 查询范围
#define DB_LIMIT( ... ) \
    taro::db::DBLimit( __VA_ARGS__ )

// 黑名单
#define DB_BLACK \
    taro::db::DBFilter( true )

// 白名单
#define DB_WHITE \
    taro::db::DBFilter( false )
