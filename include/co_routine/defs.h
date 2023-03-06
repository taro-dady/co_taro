
#pragma once

#include "log/inc.h"

#define NAMESPACE_TARO_RT_BEGIN namespace taro { namespace rt{
#define NAMESPACE_TARO_RT_END } }
#define USING_NAMESPACE_TARO_RT using namespace taro::rt;

#define RT_FATAL taro::log::LogWriter( "co_routine", taro::log::eLogLevelFatal, __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
#define RT_ERROR taro::log::LogWriter( "co_routine", taro::log::eLogLevelError, __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
#define RT_WARN  taro::log::LogWriter( "co_routine", taro::log::eLogLevelWarn,  __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
#define RT_TRACE taro::log::LogWriter( "co_routine", taro::log::eLogLevelTrace, __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
#define RT_DEBUG taro::log::LogWriter( "co_routine", taro::log::eLogLevelDebug, __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
