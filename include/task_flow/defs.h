
#pragma once

#include "log/inc.h"
#include "base/error_no.h"
#include "base/utils/assert.h"
#include <string>

#define NAMESPACE_TARO_TASKFLOW_BEGIN namespace taro { namespace taskflow{
#define NAMESPACE_TARO_TASKFLOW_END }}
#define USING_NAMESPACE_TARO_TASKFLOW using namespace taro::taskflow;

#define TASKFLOW_FATAL taro::log::LogWriter( "taskflow", taro::log::eLogLevelFatal, __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
#define TASKFLOW_ERROR taro::log::LogWriter( "taskflow", taro::log::eLogLevelError, __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
#define TASKFLOW_WARN  taro::log::LogWriter( "taskflow", taro::log::eLogLevelWarn,  __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
#define TASKFLOW_TRACE taro::log::LogWriter( "taskflow", taro::log::eLogLevelTrace, __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
#define TASKFLOW_DEBUG taro::log::LogWriter( "taskflow", taro::log::eLogLevelDebug, __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
