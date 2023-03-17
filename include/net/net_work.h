
#pragma once

#include "net/defs.h"
#include <vector>

NAMESPACE_TARO_NET_BEGIN

/**
* @brief 启动网络
*/
extern TARO_DLL_EXPORT void start_network();

/**
* @brief 启动带有SSL的网络库
*/
extern TARO_DLL_EXPORT void start_network_ssl();

/**
* @brief 停止网络
*/
extern TARO_DLL_EXPORT void stop_network();

/**
* @brief 网络接口信息
*/
struct NetInterface
{
    std::string              name;     
    std::string              mac;   
    std::vector<std::string> ip;       
    std::vector<std::string> mask;    
    std::vector<std::string> gateway;
};

/**
* @brief 获取本机的网络接口
*/
extern TARO_DLL_EXPORT std::vector<NetInterface> get_host_interfaces();

// DNS解析后的信息
struct DnsItem
{
    std::string domain;
    std::string ip;
};

/**
* @brief 域名解析
*
* @param[in] host 域名
* @param[in] dns  服务器ip
*/
extern TARO_DLL_EXPORT std::vector< DnsItem > get_host_by_name( const char* name, const char* dns = "114.114.114.114" );

NAMESPACE_TARO_NET_END
