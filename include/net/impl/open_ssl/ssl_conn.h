
#pragma once

#include "net/defs.h"
#include "base/memory/dyn_packet.h"
#include <map>
#include <list>
#include <mutex>
#include <tuple>
#include <atomic>
#include <functional>
#include <openssl/ssl.h>

NAMESPACE_TARO_NET_BEGIN

// 写数据回调
using NetWriteCallback = std::function< int32_t( char*, uint32_t ) >;

// SSL适配模块
class SSLAdaptor
{
PUBLIC:

    /**
    * @brief 网络收到数据后向BIO写数据
    * 
    * @param[in] packet 网络接收到的数据
    */
    void net_read( DynPacketSPtr const& packet );

    /**
    * @brief 设置BIO输出数据到网络的回调
    * 
    * @param[in] cb 回调函数
    */
    void set_net_write( NetWriteCallback const& cb );

    /**
    * @brief 加密完成后输出数据
    * 
    * @param[in] data  数据指针
    * @param[in] bytes 数据长度
    */
    int32_t ssl_write( const void* data, int32_t bytes );

    /**
    * @brief 解密需要读入的数据
    * 
    * @param[in] buffer 数据缓冲
    * @param[in] bytes  需要数据的长度
    */
    int32_t ssl_read( void* buffer, int32_t bytes );

    /**
    * @brief 是否有数据待读
    */
    bool empty() const;

PRIVATE:

    // 写数据对象
    struct PacketItem
    {
        PacketItem( DynPacketSPtr const& p )
            : offset( 0 )
            , packet( p )
        {

        }

        uint32_t      offset;
        DynPacketSPtr packet;
    };

PRIVATE:

    NetWriteCallback      write_callback_;
    std::list<PacketItem> read_packets_;
};

// SSL上下文工厂 
class SSLCtxFactory
{
PUBLIC:

    /**
    * @brief 获取SSL工厂单例
    */
    static SSLCtxFactory& instance();

    /**
    * @brief 查找对应的SSL_CTX, 若没有则创建
    * 
    * @param[in] ctx SSL上下文参数 
    */
    SSL_CTX* find_or_create( SSLContext const& ctx );

PRIVATE:

    /**
    * @brief 构造函数
    */
    SSLCtxFactory() = default;

    /**
    * @brief 析构函数
    */
    ~SSLCtxFactory() = default;

    /**
    * @brief 加载SSL文件
    * 
    * @param[in] ctx  SSL上下文参数
    * @param[in] path 文件路径
    */
    bool load_ssl_files( SSL_CTX* ctx, SSLContext* path );

PRIVATE:

    using SSLMap = std::map< std::tuple< std::string, std::string, std::string >, SSL_CTX* >;
    
    SSLMap     factory_;
    std::mutex mutex_;
};

// SSL连接对象
class SSLConn
{
PUBLIC:

    /**
    * @brief 构造函数(客户端)
    * 
    * @param[in] ctx SSL上下文参数
    */
    SSLConn( SSLContext const& ctx );

    /**
    * @brief 构造函数(服务端)
    * 
    * @param[in] ctx SSL上下文
    */
    SSLConn( SSL_CTX* ctx );

    /**
    * @brief 析构函数
    */
    ~SSLConn();

    /**
    * @brief 设置BIO输出数据到网络的回调
    * 
    * @param[in] cb 回调函数
    */
    void set_write_cb( NetWriteCallback const& func );

    /**
    * @brief 获取状态
    * 
    * @return true 正常 false 握手中
    */
    bool state() const;

    /**
    * @brief 执行握手
    */
    int32_t handshake();

    /**
    * @brief 处理网络读事件
    * 
    * @param[in] packet 读到的数据
    */
    void handle_read_event( DynPacketSPtr const& packet );

    /**
    * @brief 解密后读到的数据
    * 
    * @param[in] buffer 数据缓冲
    * @param[in] bytes  数据缓冲的长度
    */
    int32_t read( char* buffer, int32_t bytes );

    /**
    * @brief 加密输出
    * 
    * @param[in] buffer 数据缓冲
    * @param[in] bytes  数据缓冲的长度
    */
    bool write( char* buffer, int32_t bytes );

PRIVATE:

    SSL_CTX*         ssl_ctx_;
    SSL*             ssl_;
    std::atomic_bool handshaked_;
    SSLAdaptor       bio_;
};

NAMESPACE_TARO_NET_END

