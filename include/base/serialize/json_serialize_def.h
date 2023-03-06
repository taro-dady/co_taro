
#pragma once

#include "json/inc.h"
#include "base/memory/optional.h"
#include "base/serialize/type_traits.h"
#include "base/serialize/param_expand_macro.h"

NAMESPACE_TARO_BEGIN

IS_CLASS_MEMBER( JsonParamSerial )

// 模板类型判断
#define JSON_IS_NORMAL\
    !IsClsMemberJsonParamSerial< typename std::decay<T>::type >::value\
    && !IsArray< typename std::decay<T>::type >::value\
    && !IsOptional< typename std::decay<T>::type >::value\
    && !IsTuple< typename std::decay<T>::type >::value\
    && !IsMap<typename std::decay<T>::type>::value

// 包括 vector list
#define JSON_IS_ARRAY\
    !IsClsMemberJsonParamSerial< typename std::decay<T>::type >::value\
    && IsArray< typename std::decay<T>::type >::value\
    && !IsOptional< typename std::decay<T>::type >::value\
    && !IsTuple< typename std::decay<T>::type >::value\
    && !IsMap<typename std::decay<T>::type>::value

#define JSON_IS_CLASS\
    IsClsMemberJsonParamSerial< typename std::decay<T>::type >::value\
    && !IsArray< typename std::decay<T>::type >::value\
    && !IsOptional< typename std::decay<T>::type >::value\
    && !IsTuple< typename std::decay<T>::type >::value\
    && !IsMap<typename std::decay<T>::type>::value

#define JSON_IS_OPTIONAL\
    !IsClsMemberJsonParamSerial< typename std::decay<T>::type >::value\
    && !IsArray< typename std::decay<T>::type >::value\
    && IsOptional< typename std::decay<T>::type >::value\
    && !IsTuple< typename std::decay<T>::type >::value\
    && !IsMap<typename std::decay<T>::type>::value

#define JSON_IS_TUPLE\
    !IsClsMemberJsonParamSerial< typename std::decay<T>::type >::value\
    && !IsArray< typename std::decay<T>::type >::value\
    && !IsOptional< typename std::decay<T>::type >::value\
    && IsTuple< typename std::decay<T>::type >::value\
    && !IsMap<typename std::decay<T>::type>::value

#define JSON_IS_MAP\
    !IsClsMemberJsonParamSerial< typename std::decay<T>::type >::value\
    && !IsArray< typename std::decay<T>::type >::value\
    && !IsOptional< typename std::decay<T>::type >::value\
    && !IsTuple< typename std::decay<T>::type >::value\
    && IsMap<typename std::decay<T>::type>::value

#define JSON_IS_VECTOR\
    !IsClsMemberJsonParamSerial< typename std::decay<T>::type >::value\
    && !IsList< typename std::decay<T>::type >::value\
    && IsVector< typename std::decay<T>::type >::value\
    && !IsOptional< typename std::decay<T>::type >::value\
    && !IsTuple< typename std::decay<T>::type >::value\
    && !IsMap<typename std::decay<T>::type>::value

#define JSON_IS_LIST\
    !IsClsMemberJsonParamSerial< typename std::decay<T>::type >::value\
    && IsList< typename std::decay<T>::type >::value\
    && !IsVector< typename std::decay<T>::type >::value\
    && !IsOptional< typename std::decay<T>::type >::value\
    && !IsTuple< typename std::decay<T>::type >::value\
    && !IsMap<typename std::decay<T>::type>::value

/********************************* optional *************************************************/
// optional 转换为json
template<typename T>
typename std::enable_if< JSON_IS_NORMAL, Json& >::type
operator<<( Json& value, Optional<T> const& r );

template<typename T>
typename std::enable_if< JSON_IS_CLASS, Json& >::type
operator<<( Json& value, Optional<T> const& r );

template<typename T>
typename std::enable_if< JSON_IS_MAP, Json& >::type
operator<<( Json& value, Optional<T> const& r );

template<typename T>
typename std::enable_if< JSON_IS_ARRAY, Json& >::type
operator<<( Json& value, Optional<T> const& r );

template<typename T>
typename std::enable_if< JSON_IS_TUPLE, Json& >::type
operator<<( Json& value, Optional<T> const& r );

// json 转换为 optional
template<typename T>
typename std::enable_if< JSON_IS_NORMAL, void >::type
operator>>( Json const& value, Optional<T>& r );

template<typename T>
typename std::enable_if< JSON_IS_CLASS, void >::type
operator>>( Json const& value, Optional<T>& r );

template<typename T>
typename std::enable_if< JSON_IS_ARRAY, void >::type
operator>>( Json const& value, Optional<T>& r );

template<typename T>
typename std::enable_if< JSON_IS_MAP, void >::type
operator>>( Json const& value, Optional<T>& r );

template<typename T>
typename std::enable_if< JSON_IS_TUPLE, void >::type
operator>>( Json const& value, Optional<T>& r );

/********************************* json to cls *************************************************/
template<typename T>
typename std::enable_if< JSON_IS_NORMAL, void >::type
cls_to_json( T const& cls, const char* name, Json& out );

template<class T>
typename std::enable_if< JSON_IS_CLASS, void >::type
cls_to_json( T const& cls, const char* name, Json& out );

template<class T>
typename std::enable_if< JSON_IS_ARRAY, void >::type
cls_to_json( T const& cls, const char* name, Json& out );

template<class T>
typename std::enable_if< JSON_IS_OPTIONAL, void >::type
cls_to_json( T const& cls, const char* name, Json& out );

template<class T>
typename std::enable_if< JSON_IS_TUPLE, void>::type
cls_to_json( T const& cls, const char* name, Json& out );

template<class T>
typename std::enable_if< JSON_IS_MAP, void >::type
cls_to_json( T const& cls, const char* name, Json& out );

/********************************* cls to json *************************************************/
template<class T>
typename std::enable_if< JSON_IS_NORMAL, void >::type
json_to_cls( Json const& in, const char* name, T& value );

template<class T>
typename std::enable_if< JSON_IS_CLASS, void >::type
json_to_cls( Json const& in, const char* name, T& value );

template<class T>
typename std::enable_if< JSON_IS_OPTIONAL, void >::type
json_to_cls( Json const& in, const char* name, T& value );

template<class T>
typename std::enable_if< JSON_IS_VECTOR, void >::type
json_to_cls( Json const& in, const char* name, T& value );

template<class T>
typename std::enable_if< JSON_IS_LIST, void >::type
json_to_cls( Json const& in, const char* name, T& value );

template<class T>
typename std::enable_if< JSON_IS_MAP, void >::type
json_to_cls( Json const& in, const char* name, T& value );

template<class T>
typename std::enable_if< JSON_IS_TUPLE,  void>::type
json_to_cls( Json const& in, const char* name, T& value );

NAMESPACE_TARO_END
