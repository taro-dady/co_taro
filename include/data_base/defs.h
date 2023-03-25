
#pragma once

#include "log/inc.h"
#include "base/error_no.h"
#include "base/utils/assert.h"

#define NAMESPACE_TARO_DB_BEGIN namespace taro { namespace db{
#define NAMESPACE_TARO_DB_END }}
#define USING_NAMESPACE_TRAO_DB using namespace taro::db;

#define DB_FATAL taro::log::LogWriter( "database", taro::log::eLogLevelFatal, __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
#define DB_ERROR taro::log::LogWriter( "database", taro::log::eLogLevelError, __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
#define DB_WARN  taro::log::LogWriter( "database", taro::log::eLogLevelWarn,  __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
#define DB_TRACE taro::log::LogWriter( "database", taro::log::eLogLevelTrace, __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
#define DB_DEBUG taro::log::LogWriter( "database", taro::log::eLogLevelDebug, __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
