
#pragma once

#include "yaml-cpp/yaml.h"
#include "base/memory/optional.h"
#include "base/serialize/type_traits.h"
#include "base/serialize/param_expand_macro.h"

NAMESPACE_TARO_BEGIN

IS_CLASS_MEMBER( YamlParamSerial )

// 模板类型判断
#define YAML_IS_NORMAL\
    !IsClsMemberYamlParamSerial< typename std::decay<T>::type >::value\
    && !IsArray< typename std::decay<T>::type >::value\
    && !IsOptional< typename std::decay<T>::type >::value\
    && !IsTuple< typename std::decay<T>::type >::value\
    && !IsMap<typename std::decay<T>::type>::value

// 包括 vector list
#define YAML_IS_ARRAY\
    !IsClsMemberYamlParamSerial< typename std::decay<T>::type >::value\
    && IsArray< typename std::decay<T>::type >::value\
    && !IsOptional< typename std::decay<T>::type >::value\
    && !IsTuple< typename std::decay<T>::type >::value\
    && !IsMap<typename std::decay<T>::type>::value

#define YAML_IS_CLASS\
    IsClsMemberYamlParamSerial< typename std::decay<T>::type >::value\
    && !IsArray< typename std::decay<T>::type >::value\
    && !IsOptional< typename std::decay<T>::type >::value\
    && !IsTuple< typename std::decay<T>::type >::value\
    && !IsMap<typename std::decay<T>::type>::value

#define YAML_IS_OPTIONAL\
    !IsClsMemberYamlParamSerial< typename std::decay<T>::type >::value\
    && !IsArray< typename std::decay<T>::type >::value\
    && IsOptional< typename std::decay<T>::type >::value\
    && !IsTuple< typename std::decay<T>::type >::value\
    && !IsMap<typename std::decay<T>::type>::value

#define YAML_IS_TUPLE\
    !IsClsMemberYamlParamSerial< typename std::decay<T>::type >::value\
    && !IsArray< typename std::decay<T>::type >::value\
    && !IsOptional< typename std::decay<T>::type >::value\
    && IsTuple< typename std::decay<T>::type >::value\
    && !IsMap<typename std::decay<T>::type>::value

#define YAML_IS_MAP\
    !IsClsMemberYamlParamSerial< typename std::decay<T>::type >::value\
    && !IsArray< typename std::decay<T>::type >::value\
    && !IsOptional< typename std::decay<T>::type >::value\
    && !IsTuple< typename std::decay<T>::type >::value\
    && IsMap<typename std::decay<T>::type>::value

/********************************* optional *************************************************/
template<typename T>
typename std::enable_if< YAML_IS_NORMAL, void >::type
operator<<( YAML::Node& value, Optional<T> const& r );

template<typename T>
typename std::enable_if< YAML_IS_CLASS, void >::type
operator<<( YAML::Node& value, Optional<T> const& r );

template<typename T>
typename std::enable_if< YAML_IS_MAP, void >::type
operator<<( YAML::Node& value, Optional<T> const& r );

template<typename T>
typename std::enable_if< YAML_IS_ARRAY, void >::type
operator<<( YAML::Node& value, Optional<T> const& r );

template<typename T>
typename std::enable_if< YAML_IS_TUPLE, void >::type
operator<<( YAML::Node& value, Optional<T> const& r );

template<typename T>
typename std::enable_if< YAML_IS_NORMAL, void >::type
operator>>( YAML::Node const& value, Optional<T>& r );

template<typename T>
typename std::enable_if< YAML_IS_CLASS, void >::type
operator>>( YAML::Node const& value, Optional<T>& r );

template<typename T>
typename std::enable_if< YAML_IS_ARRAY, void >::type
operator>>( YAML::Node const& value, Optional<T>& r );

template<typename T>
typename std::enable_if< YAML_IS_MAP, void >::type
operator>>( YAML::Node const& value, Optional<T>& r );

template<typename T>
typename std::enable_if< YAML_IS_TUPLE, void >::type
operator>>( YAML::Node const& value, Optional<T>& r );

/********************************* class to yaml *************************************************/
template<typename T>
typename std::enable_if< YAML_IS_NORMAL, void >::type
cls_to_yaml( T const& cls, const char* name, YAML::Node& node );

template<typename T>
typename std::enable_if< YAML_IS_ARRAY, void >::type
cls_to_yaml( T const& cls, const char* name, YAML::Node& node );

template<typename T>
typename std::enable_if< YAML_IS_CLASS, void >::type
cls_to_yaml( T const& cls, const char* name, YAML::Node& node );

template<typename T>
typename std::enable_if< YAML_IS_MAP, void >::type
cls_to_yaml( T const& cls, const char* name, YAML::Node& node );

template<typename T>
typename std::enable_if< YAML_IS_OPTIONAL, void >::type
cls_to_yaml( T const& cls, const char* name, YAML::Node& node );

template<class T>
typename std::enable_if< YAML_IS_TUPLE, void>::type
cls_to_yaml( T const& cls, const char* name, YAML::Node& out );

/********************************* yaml to class *************************************************/
template<class T>
typename std::enable_if< YAML_IS_NORMAL, void >::type
yaml_to_cls( YAML::Node node, const char* name, T& value );

template<class T>
typename std::enable_if< YAML_IS_ARRAY, void >::type
yaml_to_cls( YAML::Node node, const char* name, T& value );

template<class T>
typename std::enable_if< YAML_IS_CLASS, void >::type
yaml_to_cls( YAML::Node node, const char* name, T& value );

template<class T>
typename std::enable_if< YAML_IS_MAP, void >::type
yaml_to_cls( YAML::Node node, const char* name, T& value );

template<class T>
typename std::enable_if< YAML_IS_OPTIONAL, void >::type
yaml_to_cls( YAML::Node node, const char* name, T& value );

template<class T>
typename std::enable_if< YAML_IS_TUPLE, void >::type
yaml_to_cls( YAML::Node node, const char* name, T& value );

NAMESPACE_TARO_END
