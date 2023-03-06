
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

NAMESPACE_TARO_NET_END
