
#pragma once

#include "base/serialize/json_serialize_def.h"
#include <sstream>

NAMESPACE_TARO_BEGIN

/**************************** optional *************************************/

// 可选类型
template<typename T>
typename std::enable_if< JSON_IS_NORMAL , Json& >::type
operator<<( Json& value, Optional<T> const& r )
{
    if ( r.valid() )
    {
        value = r.value();
    }
    return value;
}

template<typename T>
typename std::enable_if< JSON_IS_CLASS, Json& >::type
operator<<( Json& value, Optional<T> const& r )
{
    if ( r.valid() )
    {
        value = r.value().JsonParamSerial();
    }
    return value;
}

template<typename T>
typename std::enable_if< JSON_IS_MAP, Json& >::type
operator<<( Json& value, Optional<T> const& r )
{
    if ( r.valid() )
    {
        cls_to_json( r.value(), nullptr, value );
    }
    return value;
}

template<typename T>
typename std::enable_if< JSON_IS_ARRAY, Json& >::type
operator<<( Json& value, Optional<T> const& r )
{
    if ( r.valid() )
    {
        cls_to_json( r.value(), nullptr, value );
    }
    return value;
}

template<typename T>
typename std::enable_if< JSON_IS_TUPLE, Json& >::type
operator<<( Json& value, Optional<T> const& r )
{
    if ( r.valid() )
    {
        cls_to_json( r.value(), nullptr, value );
    }
    return value;
}

template<typename T>
typename std::enable_if< JSON_IS_NORMAL, void >::type
operator>>( Json const& value, Optional<T>& r )
{
    r = value.get<T>();
}

template<typename T>
typename std::enable_if< JSON_IS_CLASS, void >::type
operator>>( Json const& value, Optional<T>& r )
{
    typename std::decay<T>::type temp;
    temp.JsonParamDeSerial( value );
    r = temp;
}

template<typename T>
typename std::enable_if< JSON_IS_ARRAY, void >::type
operator>>( Json const& value, Optional<T>& r )
{
    typename std::decay<T>::type obj;
    json_to_cls( value, nullptr, obj );
    r = std::move( obj );
}

template<typename T>
typename std::enable_if< JSON_IS_MAP, void >::type
operator>>( Json const& value, Optional<T>& r )
{
    typename std::decay<T>::type obj;
    json_to_cls( value, nullptr, obj );
    r = std::move( obj );
}

template<typename T>
typename std::enable_if< JSON_IS_TUPLE, void >::type
operator>>( Json const& value, Optional<T>& r )
{
    typename std::decay<T>::type obj;
    json_to_cls( value, nullptr, obj );
    r = std::move( obj );
}

/**************************** class to json basic *************************************/
template<typename T>
typename std::enable_if< JSON_IS_NORMAL, void >::type
cls_to_json( T const& cls, const char* name, Json& out )
{
    if ( name != nullptr )
        out[name] = cls;
    else
        out = cls;
}

template<class T>
typename std::enable_if< JSON_IS_CLASS, void >::type
cls_to_json( T const& cls, const char* name, Json& out )
{
    if ( name == nullptr )
        out = cls.JsonParamSerial();
    else
        out[name] = cls.JsonParamSerial();
}

template<class T>
typename std::enable_if< JSON_IS_ARRAY, void >::type
cls_to_json( T const& cls, const char* name, Json& out ) 
{
    Json* a = ( ( nullptr == name ) ? &out : &out[name] );

    int32_t index = 0;
    for ( auto& one : cls )
    {
        cls_to_json( one, nullptr, (*a)[index++] );
    }
}

template<class T>
typename std::enable_if< JSON_IS_OPTIONAL, void >::type
cls_to_json( T const& cls, const char* name, Json& out ) 
{
    if ( !cls.valid() )
        return;
    
    if ( name == nullptr )
        out << cls;
    else
        out[name] << cls;
}

template<class Tuple, size_t N = std::tuple_size< Tuple >::value >
struct TupleToJson 
{
    static void transfer( Json& value, Tuple const& t ) 
    {
        cls_to_json( std::get<N - 1>( t ), nullptr, value[N - 1] );
        TupleToJson<Tuple, N - 1>::transfer( value, t );
    }
};

template<class Tuple >
struct TupleToJson<Tuple, 1> 
{
    static void transfer( Json& value, Tuple const& t ) 
    {
        cls_to_json( std::get<0>( t ), nullptr, value[0] );
    }
};

template<class T>
typename std::enable_if< JSON_IS_TUPLE, void >::type
cls_to_json( T const& cls, const char* name, Json& out ) 
{
    if ( nullptr != name )
    {
        TupleToJson<T>::transfer( out[name], cls );
    }
    else
    {
        TupleToJson<T>::transfer( out, cls );
    }
}

template<typename T>
typename std::enable_if< JSON_IS_TUPLE, void >::type
operator>>( T const& data, Json& value )
{
    TupleToJson<T>::transfer( value, data );
}

template<class T>
typename std::enable_if< JSON_IS_MAP, void >::type
cls_to_json( T const& cls, const char* name, Json& out )
{
    Json* ref = &out;
    if ( name != nullptr )
    {
        ref = &out[name];
    }

    for( auto& one : cls )
    {
        std::stringstream ss;
        ss << one.first;
        
        Json value;
        cls_to_json( one.second, nullptr, value );
        ( *ref )[ss.str()] = value;
    }
}

/**
* @brief 参数递归展开
*/
template<typename T>
void cls_to_js_param_expand( Json& js, const char* n, T const& value ) 
{
    cls_to_json( value, n, js );
}

/**
* @brief 参数递归展开
*/
template<typename T, typename... Args>
void cls_to_js_param_expand( Json& js, const char* n,  T const& value, Args... args ) 
{
    cls_to_json( value, n, js );
    cls_to_js_param_expand( js, args... );
}

/**************************** json to cls *************************************/
template<class T>
typename std::enable_if< JSON_IS_NORMAL, void >::type
json_to_cls( Json const& ss, const char* n, T& value ) 
{
    if ( nullptr == n )
    {
        value = ss.get<T>();
        return;
    }

    if ( ss.contains( n ) )
    {
        value = ss[n].get<T>();
    }
}

template<class T>
typename std::enable_if< JSON_IS_CLASS, void >::type
json_to_cls( Json const& ss, const char* n, T& value )
{
    if ( n != nullptr ) 
    {
        if ( !ss.contains( n ) )
        {
            return;
        }
        value.JsonParamDeSerial( ss[n] );
        return;
    }
    value.JsonParamDeSerial( ss );
}

template<class T>
typename std::enable_if< JSON_IS_OPTIONAL, void >::type
json_to_cls( Json const& ss, const char* n, T& value ) 
{
    if ( nullptr == n )
    {
        ss >> value;
    }
    else if ( ss.contains( n ) )
    {
        ss[n] >> value;
    }
}

template<class T>
typename std::enable_if< JSON_IS_VECTOR, void >::type
json_to_cls( Json const& ss, const char* n, T& value ) 
{
    size_t index = 0;
    if ( nullptr == n )
    {
        value.resize( ss.size() );
        for ( ; index < ss.size(); ++index ) 
        {
            json_to_cls( ss[index], nullptr, value[index] );
        }
        return;
    }

    if ( !ss.contains( n ) )
    {
        return;
    }

    value.resize( ss[n].size() );
    for ( ; index < ss[n].size(); ++index ) 
    {
        json_to_cls( ss[n][index], nullptr, value[index] );
    }
}

template<class T>
typename std::enable_if< JSON_IS_LIST, void >::type
json_to_cls( Json const& ss, const char* n, T& value ) 
{
    if ( nullptr == n )
    {
        size_t index = 0;
        for ( ; index < ss.size(); ++index ) 
        {
            typename T::value_type tmp;
            json_to_cls( ss[index], nullptr, tmp );
            value.push_back( tmp );
        }
        return;
    }

    if ( !ss.contains( n ) )
    {
        return;
    }

    size_t index = 0;
    for ( ; index < ss[n].size(); ++index ) 
    {
        typename T::value_type tmp;
        json_to_cls( ss[n][index], nullptr, tmp );
        value.push_back( tmp );
    }
}

template<class Tuple, size_t N = std::tuple_size< Tuple >::value >
struct JsonToTuple 
{
    static void transfer( Json const& value, Tuple& t ) 
    {
        json_to_cls( value[N - 1], nullptr, std::get<N - 1>( t ) );
        JsonToTuple<Tuple, N - 1>::transfer( value, t );
    }
};

template<class Tuple >
struct JsonToTuple<Tuple, 1> 
{
    static void transfer( Json const& value, Tuple& t ) 
    {
        json_to_cls( value[0], nullptr, std::get<0>( t ) );
    }
};

template<class T>
typename std::enable_if< JSON_IS_TUPLE, void >::type
json_to_cls( Json const& ss, const char* name, T& value ) 
{
    if ( nullptr != name )
    {
        JsonToTuple<T>::transfer( ss[name], value );
    }
    else
    {
        JsonToTuple<T>::transfer( ss, value );
    }
}

template<typename T>
typename std::enable_if< JSON_IS_TUPLE, void>::type
operator<<( T& data, Json const& value )
{
    JsonToTuple<T>::transfer( value, data );
}

template<class T>
typename std::enable_if< JSON_IS_MAP, void >::type
json_to_cls( Json const& ss, const char* n, T& value ) 
{
    Json* ref = ( Json* )&ss;
    if ( n != nullptr )
    {
        ref = ( Json* )&ss[n];
    }

    using RAW_T = typename std::decay<T>::type;

    for ( auto const& one : ref->items() )
    {
        std::stringstream ss;
        ss << one.key();
        typename RAW_T::key_type k;
        ss >> k;

        typename RAW_T::mapped_type v;
        json_to_cls( one.value(), nullptr, v );
        value[k] = v;
    }
}

/**
* @brief 参数递归展开
*/
template<typename T>
void json_to_cls_param_expand( Json const& ss, const char* n, T& value ) 
{
    json_to_cls( ss, n, value );
}

/**
* @brief 参数递归展开
*/
template<typename T, typename... Args>
void json_to_cls_param_expand( Json const& ss, const char* n, T& value, Args&&... args ) 
{
    json_to_cls( ss, n, value );
    json_to_cls_param_expand( ss, args... );
}

/****************************** user call api ****************************************/

template<typename T>
typename std::enable_if< !IsClsMemberJsonParamSerial<T>::value, Json >::type
json_param_serial( T const& value )
{
    Json jvalue = value;
    return jvalue;
}

template<typename T>
typename std::enable_if< IsClsMemberJsonParamSerial<T>::value, Json >::type
json_param_serial( T const& value )
{
    return value.JsonParamSerial();
}

template<typename T>
typename std::enable_if< !IsClsMemberJsonParamSerial<T>::value && !IsVector<T>::value, void >::type
json_param_deserial( Json const& j, T& value )
{
    value = j.get<T>();
}

template<typename T>
typename std::enable_if< IsClsMemberJsonParamSerial<T>::value, void >::type
json_param_deserial( Json const& j, T& value )
{
    value.JsonParamDeSerial(j);
}

template<typename T>
typename std::enable_if< IsClsMemberJsonParamSerial<T>::value, void >::type
operator<<( Json& j, T const& value )
{
    j = value.JsonParamSerial();
}

template<typename T>
typename std::enable_if< IsClsMemberJsonParamSerial<T>::value, void >::type
operator>>( T const& value, Json& j )
{
    j = value.JsonParamSerial();
}

template<typename T>
typename std::enable_if< IsClsMemberJsonParamSerial<T>::value, void >::type
operator>>( Json const& j, T& value )
{
    value.JsonParamDeSerial(j);
}

template<typename T>
typename std::enable_if< IsClsMemberJsonParamSerial<T>::value, void >::type
operator<<( T& value, Json const& j )
{
    value.JsonParamDeSerial(j);
}

NAMESPACE_TARO_END

/**************************** macro define *************************************/
#define TARO_JSON_DEFINE(...)\
taro::Json JsonParamSerial() const{\
    taro::Json value;\
    taro::cls_to_js_param_expand(value, PARAM_DEF_FOR_EACH_(EXTAND_ARGS(FL_ARG_COUNT(__VA_ARGS__)),__VA_ARGS__));\
    return value;\
}\
void JsonParamDeSerial( taro::Json const& value ){\
    taro::json_to_cls_param_expand( value, PARAM_DEF_FOR_EACH_(EXTAND_ARGS(FL_ARG_COUNT(__VA_ARGS__)),__VA_ARGS__) );\
}
