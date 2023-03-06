
#pragma once

#include "net/ud_client.h"

NAMESPACE_TARO_NET_BEGIN

// unix domain 服务端
class UDServer
{
PUBLIC:

    /**
    * @brief 构造函数
    */
    UDServer();

    /**
    * @brief 析构函数
    */
    ~UDServer();
    
    /**
    * @brief 监听客户端
    * 
    * @param[in] local 服务名称
    */
    bool listen( std::string const& local );
    
    /*
    * @brief 接受对端连接
    */
    UDClientSPtr accept();

PRIVATE:

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

using UDServerSPtr = std::shared_ptr<UDServer>;

NAMESPACE_TARO_NET_END
