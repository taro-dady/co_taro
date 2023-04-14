
#pragma once

#include "base/serialize/yaml_serialize.h"

NAMESPACE_TARO_BEGIN

// yaml格式的配置
class TARO_DLL_EXPORT YamlConfig
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    YamlConfig();

    /**
    * @brief 析构函数
    */
    ~YamlConfig();

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
        YAML::Node node = cfg.YamlParamSerial();
        return set_config( &node );
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
        YAML::Node node = cfg.YamlParamSerial();
        return set_config( path, &node );
    }

    /**
    * @brief 获取配置
    * 
    * @param[out] config  配置数据
    */
    template<typename T>
    bool get_config( T& cfg )
    {
        YAML::Node* node = nullptr;
        ( void )get_config( &node );
        if ( nullptr == node )
        {
            return false;
        }
        cfg.YamlParamDeSerial( *node );
        return true;
    }

    /**
    * @brief 获取配置
    * 
    * @param[in] path     路径 如: "net.[2].ip" json["net"][2]["ip"] "net.2.ip" json["net"]["2"]["ip"]
    * @param[out] config  配置数据
    */
    template<typename T>
    bool get_config( const char* path, T& cfg )
    {
        YAML::Node* node = nullptr;
        ( void )get_config( path, &node );
        if ( nullptr == node )
        {
            return false;
        }
        cfg.YamlParamDeSerial( *node );
        delete_node( node );
        return true;
    }

PRIVATE: // 私有函数

    TARO_NO_COPY( YamlConfig );

    /**
    * @brief 设置配置
    * 
    * @param[in] config 配置数据
    */
    bool set_config( YAML::Node* config );

    /**
    * @brief 设置配置
    * 
    * @param[in] config 配置数据
    */
    bool set_config( const char* path, YAML::Node* config );

    /**
    * @brief 获取配置
    * 
    * @param[out] config      配置数据
    */
    void get_config( YAML::Node** config );

    /**
    * @brief 获取配置
    * 
    * @param[out] config      配置数据
    */
    void get_config( const char* path, YAML::Node** config );

    /**
    * @brief 删除节点
    */
    void delete_node( YAML::Node* node );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

using YamlConfigSPtr = std::shared_ptr<YamlConfig>;

NAMESPACE_TARO_END
