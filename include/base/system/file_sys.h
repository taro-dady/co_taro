
#pragma once

#include "base/base.h"
#include <list>
#include <string>

NAMESPACE_TARO_BEGIN

// 文件节点类型
enum ENodeType
{
    eNodeTypeDir  = 1 << 0,  // 文件夹
    eNodeTypeFile = 1 << 1,  // 普通文件
};

// 文件系统节点信息
struct FileSysNode
{
    ENodeType   type;   // 节点类型
    std::string name;   // 节点名称
    int64_t     mtime;  // 上一次修改时间
    uint64_t    size;   // 文件大小
};

// 文件系统接口
class TARO_DLL_EXPORT FileSystem
{
PUBLIC: // 公共函数

    /**
    * @brief 检查文件夹是否存在
    * 
    * @param[in] dir 文件夹
    * @return 见错误码
    */
    static int32_t check_dir( const char* dir );

    /**
    * @brief 递归创建文件夹
    * 
    * @param[in] dir 文件夹
    * @return 见错误码
    */
    static int32_t create_dir( const char* dir );
    
    /**
    * @brief 检查文件是否存在
    * 
    * @param[in] path 文件路径
    * @return 见错误码
    */
    static int32_t check_file( const char* path );

    /**
    * @brief 文件拷贝
    * 
    * @param[in] src 原文件
    * @param[in] dst 目标文件
    * @return 见错误码
    */
    static int32_t copy_file( const char* src, const char* dst );

    /**
    * @brief 获取文件夹下的文件以及文件夹信息
    * 
    * @param[in] path 文件夹
    */
    static std::list<FileSysNode> get_nodes( const char* dir );
};

NAMESPACE_TARO_END
