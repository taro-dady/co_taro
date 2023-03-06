
#pragma once

#include "yaml_serialize_def.h"

NAMESPACE_TARO_BEGIN

/**************************** tuple ****************************/
template<class Tuple, size_t N = std::tuple_size< Tuple >::value >
struct TupleToYaml 
{
    static void transfer( YAML::Node value, Tuple const& t ) 
    {
        auto tmp = value[std::tuple_size< Tuple >::value - N];
        cls_to_yaml( std::get<std::tuple_size< Tuple >::value - N>( t ), nullptr, tmp );
        TupleToYaml<Tuple, N - 1>::transfer( value, t );
    }
};

template<class Tuple >
struct TupleToYaml<Tuple, 1> 
{
    static void transfer( YAML::Node value, Tuple const& t ) 
    {
        auto tmp = value[std::tuple_size< Tuple >::value - 1];
        cls_to_yaml( std::get<std::tuple_size< Tuple >::value - 1>( t ), nullptr, tmp );
    }
};

template<class Tuple, size_t N = std::tuple_size< Tuple >::value >
struct YamlToTuple 
{
    static void transfer( YAML::Node const& value, Tuple& t ) 
    {
        yaml_to_cls( value[N - 1], nullptr, std::get<N - 1>( t ) );
        YamlToTuple<Tuple, N - 1>::transfer( value, t );
    }
};

template<class Tuple >
struct YamlToTuple<Tuple, 1> 
{
    static void transfer( YAML::Node const& value, Tuple& t ) 
    {
        yaml_to_cls( value[0], nullptr, std::get<0>( t ) );
    }
};

/********************************* optional ************************/
template<typename T>
typename std::enable_if< YAML_IS_NORMAL, void >::type
operator<<( YAML::Node& value, Optional<T> const& r )
{
    if ( r.valid() )
    {
        value = r.value();
    }
}

template<typename T>
typename std::enable_if< YAML_IS_CLASS, void >::type
operator<<( YAML::Node& value, Optional<T> const& r )
{
    if ( r.valid() )
    {
        value = r.value().YamlParamSerial();
    }
}

template<typename T>
typename std::enable_if< YAML_IS_MAP, void >::type
operator<<( YAML::Node& value, Optional<T> const& r )
{
    if ( r.valid() )
    {
        cls_to_yaml( r.value(), nullptr, value );
    }
}

template<typename T>
typename std::enable_if< YAML_IS_ARRAY, void >::type
operator<<( YAML::Node& value, Optional<T> const& r )
{
    if ( r.valid() )
    {
        cls_to_yaml( r.value(), nullptr, value );
    }
}

template<typename T>
typename std::enable_if< YAML_IS_TUPLE, void >::type
operator<<( YAML::Node& value, Optional<T> const& r )
{
    if ( r.valid() )
    {
        cls_to_yaml( r.value(), nullptr, value );
    }
}

template<typename T>
typename std::enable_if< YAML_IS_NORMAL, void >::type
operator>>( YAML::Node const& value, Optional<T>& r )
{
    r = value.as<T>();
}

template<typename T>
typename std::enable_if< YAML_IS_CLASS, void >::type
operator>>( YAML::Node const& value, Optional<T>& r )
{
    typename std::decay<T>::type temp;
    temp.YamlParamDeSerial( value );
    r = temp;
}

template<typename T>
typename std::enable_if< YAML_IS_ARRAY, void >::type
operator>>( YAML::Node const& value, Optional<T>& r )
{
    typename std::decay<T>::type obj;
    yaml_to_cls( value, nullptr, obj );
    r = std::move( obj );
}

template<typename T>
typename std::enable_if< YAML_IS_MAP, void >::type
operator>>( YAML::Node const& value, Optional<T>& r )
{
    typename std::decay<T>::type obj;
    yaml_to_cls( value, nullptr, obj );
    r = std::move( obj );
}

template<typename T>
typename std::enable_if< YAML_IS_TUPLE, void >::type
operator>>( YAML::Node const& value, Optional<T>& r )
{
    typename std::decay<T>::type obj;
    yaml_to_cls( value, nullptr, obj );
    r = std::move( obj );
}

/**************************** class to yaml *************************************/
template<typename T>
typename std::enable_if< YAML_IS_NORMAL, void >::type
cls_to_yaml( T const& cls, const char* name, YAML::Node& node )
{
    if ( nullptr != name )
    {
        node[name] = cls;
    }
    else
    {
        node = cls;
    }
}

template<typename T>
typename std::enable_if< YAML_IS_ARRAY, void >::type
cls_to_yaml( T const& cls, const char* name, YAML::Node& node )
{
    YAML::Node temp;
    if ( nullptr != name )
    {
        temp = node[name];
    }
    YAML::Node* n = ( nullptr != name ) ? &temp : &node;

    int32_t index = 0;
    for ( auto const& one : cls )
    {
        auto new_node = ( *n )[index++];
        cls_to_yaml( one, nullptr, new_node );
    }
}

template<typename T>
typename std::enable_if< YAML_IS_CLASS, void >::type
cls_to_yaml( T const& cls, const char* name, YAML::Node& node )
{
    if ( nullptr != name )
    {
        node[name] = cls.YamlParamSerial();
    }
    else
    {
        node = cls.YamlParamSerial();
    }
}

template<typename T>
typename std::enable_if< YAML_IS_MAP, void >::type
cls_to_yaml( T const& cls, const char* name, YAML::Node& node )
{
    YAML::Node temp;
    if ( nullptr != name )
    {
        temp = node[name];
    }

    YAML::Node* n = ( nullptr != name ) ? &temp : &node;
    for( auto& one : cls )
    {
        std::stringstream ss;
        ss << one.first;
        auto new_node = ( *n )[ss.str()];
        cls_to_yaml( one.second, nullptr, new_node );
    }
}

template<typename T>
typename std::enable_if< YAML_IS_OPTIONAL, void >::type
cls_to_yaml( T const& cls, const char* name, YAML::Node& node )
{
    if ( !cls.valid() )
    {
        return;
    }
    
    if ( name == nullptr )
    {
        node << cls;
    }
    else
    {
        auto temp = node[name];
        temp << cls;
    }
}

template<class T>
typename std::enable_if< YAML_IS_TUPLE, void>::type
cls_to_yaml( T const& cls, const char* name, YAML::Node& out )
{
    if ( nullptr != name )
    {
        TupleToYaml<T>::transfer( out[name], cls );
    }
    else
    {
        TupleToYaml<T>::transfer( out, cls );
    }
}

template<typename T>
void cls_to_yaml_param_expand( YAML::Node& node, const char* n, T const& value ) 
{
    cls_to_yaml( value, n, node );
}

template<typename T, typename... Args>
void cls_to_yaml_param_expand( YAML::Node& node, const char* n,  T const& value, Args... args ) 
{
    cls_to_yaml( value, n, node );
    cls_to_yaml_param_expand( node, args... );
}

/**************************** yaml to class *************************************/
template<class T>
typename std::enable_if< YAML_IS_NORMAL, void >::type
yaml_to_cls( YAML::Node node, const char* name, T& value )
{
    if ( nullptr != name )
    {
        value = node[name].as<T>();
    }
    else
    {
        value = node.as<T>();
    }
}

template<class T>
typename std::enable_if< YAML_IS_ARRAY, void >::type
yaml_to_cls( YAML::Node node, const char* name, T& value )
{
    YAML::Node temp;
    if ( nullptr != name )
    {
        temp = node[name];
    }

    using VALUE_TYPE = typename std::decay<typename T::value_type>::type;
    YAML::Node const* n = ( nullptr != name ) ? &temp : &node;
    for ( size_t i = 0; i < n->size(); ++i )
    {
        VALUE_TYPE temp;
        yaml_to_cls( ( *n )[i], nullptr, temp );
        value.emplace_back( temp );
    }
}

template<class T>
typename std::enable_if< YAML_IS_CLASS, void >::type
yaml_to_cls( YAML::Node node, const char* name, T& value )
{
    YAML::Node temp;
    if ( nullptr != name )
    {
        temp = node[name];
    }
    YAML::Node const* n = ( nullptr != name ) ? &temp : &node;
    value.YamlParamDeSerial( *n );
}

template<class T>
typename std::enable_if< YAML_IS_MAP, void >::type
yaml_to_cls( YAML::Node node, const char* name, T& value )
{
    YAML::Node temp;
    if ( nullptr != name )
    {
        temp = node[name];
    }
    YAML::Node const* n = ( nullptr != name ) ? &temp : &node;
    using RAW_T = typename std::decay<T>::type;

    for ( auto it = n->begin(); it != n->end(); ++it )
    {
        std::stringstream ss;
        ss << it->first;
        typename RAW_T::key_type k;
        ss >> k;
        yaml_to_cls( it->second, nullptr, value[k] );
    }
}

template<class T>
typename std::enable_if< YAML_IS_OPTIONAL, void >::type
yaml_to_cls( YAML::Node node, const char* name, T& value )
{
    if ( nullptr == name )
    {
        node >> value;
    }
    else if ( node[name] )
    {
        node[name] >> value;
    }
}

template<class T>
typename std::enable_if< YAML_IS_TUPLE, void >::type
yaml_to_cls( YAML::Node node, const char* name, T& value )
{
    if ( nullptr == name )
    {
        YamlToTuple<T>::transfer( node, value );
    }
    else if ( node[name] )
    {
        YamlToTuple<T>::transfer( node[name], value );
    }
}

template<typename T>
void yaml_to_cls_param_expand( YAML::Node node, const char* n, T& value ) 
{
    yaml_to_cls( node, n, value );
}

template<typename T, typename... Args>
void yaml_to_cls_param_expand( YAML::Node node, const char* n, T& value, Args&&... args ) 
{
    yaml_to_cls( node, n, value );
    yaml_to_cls_param_expand( node, args... );
}

NAMESPACE_TARO_END

/**************************** macro define *************************************/
#define TARO_YAML_DEFINE(...)\
YAML::Node YamlParamSerial() const{\
    YAML::Node node;\
    taro::cls_to_yaml_param_expand(node, PARAM_DEF_FOR_EACH_(EXTAND_ARGS(FL_ARG_COUNT(__VA_ARGS__)),__VA_ARGS__));\
    return node;\
}\
void YamlParamDeSerial( YAML::Node const& node ){\
    taro::yaml_to_cls_param_expand( node, PARAM_DEF_FOR_EACH_(EXTAND_ARGS(FL_ARG_COUNT(__VA_ARGS__)),__VA_ARGS__) );\
}
