
#pragma once

#include <sstream>
#include "base/system/time.h"

#define NAMESPACE_TARO_LOG_BEGIN namespace taro { namespace log{
#define NAMESPACE_TARO_LOG_END }}
#define USING_NAMESPACE_TARO_LOG using namespace taro::log;

NAMESPACE_TARO_LOG_BEGIN

// 日志等级
enum ELogLevel
{
    eLogLevelDebug = 0,
    eLogLevelTrace = 1,
    eLogLevelWarn  = 2,
    eLogLevelError = 3,
    eLogLevelFatal = 4,
};

NAMESPACE_TARO_LOG_END
