
#pragma once

#include "data_base/defs.h"
#include "base/utils/arg_expander.h"
#include "base/serialize/type_traits.h"
#include "base/serialize/str_serialize.h"
#include "base/serialize/param_expand_macro.h"
#include <map>
#include <vector>
#include <memory>
#include <typeinfo>

NAMESPACE_TARO_DB_BEGIN

// 变量反射基类
class TARO_DLL_EXPORT ClsMemberReflectorBase
{
public: // 公共函数

    /**
    * @brief  虚析构
    */
    virtual ~ClsMemberReflectorBase() = default;

    /**
    * @brief  获取名称
    */
    virtual std::string const& get_name() const = 0;

    /**
    * @brief  获取类型
    *
    * @return 成员类型
    */
    virtual const std::type_info* get_type() const = 0;

    /**
    * @brief  将成员变量转换为字符串
    *
    * @param[in] class_obj_ptr 类对象指针
    * @return    成员变量序列化结果
    */
    virtual std::string serialize( void* class_obj_ptr ) const = 0;

    /**
    * @brief  将字符串转换为成员变量的值
    *
    * @param[in] text 成员对象
    * @param[in] class_obj_ptr 类对象指针
    * @return    成员变量序列化结果
    */
    virtual bool deserialize( const char* text, void* class_obj_ptr ) const = 0;
};
using ClsMemberReflectorSPtr = std::shared_ptr<ClsMemberReflectorBase>;

// 类成员变量的反射
template<typename Class, typename MemberPtr>
class ClsMemberReflector : PUBLIC ClsMemberReflectorBase
{
PUBLIC: // 公共函数

    /**
    * @brief  构造函数
    * 
    * @param[in] name 成员名称
    * @param[in] mptr 成员变量指针
    */
    ClsMemberReflector( const char* name, MemberPtr const& mptr )
        : name_( name )
        , mptr_( mptr )
    {

    }

    /**
    * @brief  获取名称
    * 
    * @return 成员名称
    */
    virtual std::string const& get_name() const override final
    {
        return name_;
    }

    /**
    * @brief  获取类型
    * 
    * @return 成员类型
    */
    virtual const std::type_info* get_type() const override final
    {
        return &typeid( decltype( ( ( Class* )0 )->*mptr_ ) );
    }

    /**
    * @brief  将变量转换为字符串
    * 
    * @return 序列化后的字符串
    */
    virtual std::string serialize( void* class_obj_ptr ) const override final
    {
        Class* obj = static_cast< Class* >( class_obj_ptr );
        if( nullptr == obj )
        {
            return "";
        }
        return value_to_str< typename UnWrapperParam< decltype( obj->*mptr_ ) >::type >( obj->*mptr_ );
    }

    /**
    * @brief  将字符串转换为变量的值
    * 
    * @param[in] text          成员值
    * @param[in] class_obj_ptr 类对象指针
    */
    virtual bool deserialize( const char* text, void* class_obj_ptr ) const override final
    {
        Class* obj = static_cast< Class* >( class_obj_ptr );
        if( nullptr == obj || !STRING_CHECK( text ) )
        {
            return false;
        }
        obj->*mptr_ = str_to_value< typename UnWrapperParam< decltype( obj->*mptr_ ) >::type >( text );
        return true;
    }

PRIVATE: // 私有变量

    std::string name_;    // 类成员名称
    MemberPtr   mptr_;    // 类的成员指针
};

// 类型与的反射关系管理对象
struct TARO_DLL_EXPORT DBReflector
{
PUBLIC: // 公共函数
    
    /**
    * @brief  获取反射管理单例
    */
    static DBReflector& instance();

    /**
    * @brief  注册类型信息
    */
    bool register_class( const char* cls, const std::type_info* info );

    /**
    * @brief 查询类名称
    *
    * @param[in]  info    类型描述
    * @return     类名称
    */
    std::string find_class_name( const std::type_info* info ) const;

    /**
    * @brief  注册类名与类的序列化方法
    */
    bool register_member( const std::type_info* cls, const char* mb, ClsMemberReflectorSPtr const& reflector );

    /**
    * @brief  获取该类所有成员的反射对象
    */
    std::vector<ClsMemberReflectorSPtr> get_member_reflectors( const std::type_info* cls );

PRIVATE: // 私有函数

    /**
    * @brief  构造函数
    */
    DBReflector();

    /**
    * @brief  析构函数
    */
    ~DBReflector();

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

/**
* @brief 类型注册
*/
template<typename C, typename M>
inline void register_cls_member( const std::type_info* cls_n, std::string const& member_n, M const& member_o )
{
    ClsMemberReflectorSPtr reflector = std::make_shared< ClsMemberReflector<C, M> >( member_n.c_str(), member_o );
    ( void )DBReflector::instance().register_member( cls_n, member_n.c_str(), reflector );
}

/**
* @brief 注册类以及类的成员变量，建立反射关系
* 
* @param[in] cls_n    类名称
* @param[in] i        成员序号，用于获取成员名称
* @param[in] member_n 类成员名称
* @param[in] member_o 类成员对象
*/
template<typename C, typename T>
void reflect_cls_and_member( std::vector<std::string> const& names, int32_t i, const std::type_info* cls_n, T const& member_o )
{
    register_cls_member<C, T>( cls_n, names[i], member_o );
}

/**
* @brief  注册类以及类的成员变量，建立反射关系
*
* @param[in] cls_n    类名称
* @param[in] i        成员序号，用于获取成员名称
* @param[in] member_n 类成员名称
* @param[in] member_o 类成员对象
*/ 
template<typename C, typename T, typename... Args>
void reflect_cls_and_member( std::vector<std::string> const& names, int32_t i, const std::type_info* cls_n, T const& member_o, Args... args )
{
    register_cls_member<C, T>( cls_n, names[i++], member_o );
    reflect_cls_and_member<C>( names, i, cls_n, args... );
}

NAMESPACE_TARO_DB_END

#define FIELD( t ) t
#define BD_DEF_FOR_EACH_( C, N, ... ) EXTAND_ARGS( CONCATENATE( FOR_EACH_, N )( &C::FIELD, __VA_ARGS__ ) )

#define DB_NAME_DEF(Param) #Param
#define DB_NAME_DEF_FOR_EACH_(N, ...) EXTAND_ARGS(CONCATENATE(FOR_EACH_, N)(DB_NAME_DEF,__VA_ARGS__))

#define TARO_DB_DEFINE( C, ...) \
static void db_cls_reflect(){\
    int32_t index = 0;\
    ( void )taro::db::DBReflector::instance().register_class( #C, &typeid( C ) );\
    std::vector<std::string> names = { DB_NAME_DEF_FOR_EACH_( EXTAND_ARGS(FL_ARG_COUNT(__VA_ARGS__)),__VA_ARGS__) };\
    taro::db::reflect_cls_and_member<C>( names, index, &typeid( C ), BD_DEF_FOR_EACH_( C, EXTAND_ARGS(FL_ARG_COUNT(__VA_ARGS__)),__VA_ARGS__) );\
}
