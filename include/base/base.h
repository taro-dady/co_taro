
#pragma once

#include <stdint.h>
#include <assert.h>
#include <iostream>

#ifndef  __UNIT_TEST__
    #define PUBLIC    public
    #define PROTECTED protected
    #define PRIVATE   private
#else
    #define PUBLIC    public
    #define PROTECTED public
    #define PRIVATE   public
#endif

#define NAMESPACE_TARO_BEGIN namespace taro {
#define NAMESPACE_TARO_END }
#define USING_NAMESPACE_TARO using namespace taro;

#define TARO_FOREVER 0xFFFFFFFF

#ifndef TARO_ALIGN
#define TARO_ALIGN( x, o ) ( ( x + (o - 1) )&~( o - 1 ) )
#endif // !TARO_ALIGN

#if __GNUC__ >= 3
    #define taro_likely(x) (__builtin_expect((x), 1))
    #define taro_unlikely(x) (__builtin_expect((x), 0))
#else
    #define taro_likely(x) (x)
    #define taro_unlikely(x) (x)
#endif

#define CHECK_ZERO_RET( exp, ret ) if ( 0 != exp ) return ret
#define CHECK_TRUE_RET( exp, ret ) if ( !exp ) return ret

#define TARO_INTERFACE class

#define TARO_NO_COPY(T)\
T( T const& );\
T& operator=( T const& )

#define TARO_CLS_DECLARE(T)\
T() = default;\
~T() = default

#define TARO_INST_DECLARE(T)\
T() = default;\
~T() = default;\
T( T const& );\
T& operator=( T const& )

#if defined _WIN32 || defined _WIN64
    #ifdef TARO_USE_DLL
        #define TARO_DLL_EXPORT __declspec(dllexport)
    #else 
        #define TARO_DLL_EXPORT 
    #endif
#else
    #define TARO_DLL_EXPORT 
#endif
