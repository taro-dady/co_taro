
#pragma once

#include "log/inc.h"
#include "base/error_no.h"
#include "base/utils/assert.h"
#include <string>

#define NAMESPACE_TARO_NET_BEGIN namespace taro { namespace net{
#define NAMESPACE_TARO_NET_END }}
#define USING_NAMESPACE_TARO_NET using namespace taro::net;

#define NET_FATAL taro::log::LogWriter( "net", taro::log::eLogLevelFatal, __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
#define NET_ERROR taro::log::LogWriter( "net", taro::log::eLogLevelError, __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
#define NET_WARN  taro::log::LogWriter( "net", taro::log::eLogLevelWarn,  __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
#define NET_TRACE taro::log::LogWriter( "net", taro::log::eLogLevelTrace, __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()
#define NET_DEBUG taro::log::LogWriter( "net", taro::log::eLogLevelDebug, __FILE__, __FUNCTION__, __LINE__ ).stream<std::stringstream>()

#define TARO_INVALID_SOCKET -1

NAMESPACE_TARO_NET_BEGIN

// ip地址
struct IpAddr
{
    /**
    * @brief 构造函数
    * 
    * @param[in] addr ip地址
    * @param[in] p    端口
    */
    IpAddr( std::string const& addr = "", uint16_t p = 0 )
        : ip( addr )
        , port( p )
    {

    }

    /**
    * @brief 有效性判断
    */
    bool valid() const
    {
        return ip != "";
    }

    /**
    * @brief 等于判断
    */
    bool operator==( IpAddr const& other ) const
    {
        return ip == other.ip && port == other.port;
    }

    /**
    * @brief 小于判断
    */
    bool operator<( IpAddr const& other ) const
    {
        return ip < other.ip && port < other.port;
    }

    std::string ip;
    uint16_t    port;
};

// 加密传输参数
struct SSLContext
{
    std::string cert;  // 证书文件
    std::string key;   // 密钥文件
    std::string ca;    // CA文件
};

NAMESPACE_TARO_NET_END

/**
* @brief 控制台输出
*/
inline std::ostream& operator<<( std::ostream& s, taro::net::IpAddr const& addr )
{
    if ( addr.valid() )
    {
        s << addr.ip << ":" << addr.port;
    }
    else
    {
        s << "invalid ip address";
    }
    return s;
}

