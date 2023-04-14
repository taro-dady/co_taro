
#pragma once

#include "base/serialize/json_serialize.h"

NAMESPACE_TARO_BEGIN

class TARO_DLL_EXPORT JsonConfig
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    JsonConfig();

    /**
    * @brief 析构函数
    */
    ~JsonConfig();

    /**
    * @brief 设置参数
    * 
    * @param user        配置文件
    * @param user_backup 备份配置文件
    */
    int32_t set_param( const char* user, const char* user_backup = nullptr );

    /**
    * @brief 加载配置
    */
    int32_t load();

    /**
    * @brief 设置配置
    * 
    * @param[in] cfg  配置数据
    */
    template<typename T>
    bool set_config( T const& cfg )
    {
        Json jcfg = cfg.JsonParamSerial();
        return set_config( &jcfg );
    }

    /**
    * @brief 设置配置
    * 
    * @param[in] path 路径 如: "net.[2].ip" json["net"][2]["ip"] "net.2.ip" json["net"]["2"]["ip"]
    * @param[in] cfg  配置数据
    */
    template<typename T>
    bool set_config( const char* path, T const& cfg )
    {
        Json jcfg = cfg.JsonParamSerial();
        return set_config( path, &jcfg );
    }

    /**
    * @brief 获取配置
    * 
    * @param[out] config  配置数据
    */
    template<typename T>
    bool get_config( T& cfg )
    {
        Json* jcfg = nullptr;
        ( void )get_config( &jcfg );
        if ( nullptr == jcfg )
        {
            return false;
        }

        cfg << ( *jcfg );
        return true;
    }

    /**
    * @brief 获取配置
    * 
    * @param[in]  path    路径 如: "net.[2].ip" json["net"][2]["ip"] "net.2.ip" json["net"]["2"]["ip"]
    * @param[out] config  配置数据
    */
    template<typename T>
    bool get_config( const char* path, T& cfg )
    {
        Json* jcfg = nullptr;
        ( void )get_config( path, &jcfg );
        if ( nullptr == jcfg )
        {
            return false;
        }

        cfg << ( *jcfg );
        return true;
    }

PRIVATE: // 私有函数

    TARO_NO_COPY( JsonConfig );

    /**
    * @brief 设置配置
    * 
    * @param[in] config 配置数据
    */
    bool set_config( Json* config );

    /**
    * @brief 设置配置
    * 
    * @param[in] path   路径 如: "net.2.ip" json["net"][2]["ip"]
    * @param[in] config 配置数据
    */
    bool set_config( const char* path, Json* config );

    /**
    * @brief 获取配置
    * 
    * @param[out] config      配置数据
    */
    void get_config( Json** config );

    /**
    * @brief 获取配置
    * 
    * @param[in]  path    路径 如: "net.2.ip" json["net"][2]["ip"]
    * @param[out] config  配置数据
    */
    void get_config( const char* path, Json** config );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

using JsonConfigSPtr = std::shared_ptr<JsonConfig>;

NAMESPACE_TARO_END
