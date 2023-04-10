
#pragma once

#include "base/base.h"
#include "base/system/time.h"
#include <memory>
#include <vector>

NAMESPACE_TARO_BEGIN

struct GraphyNodeImpl;
class  GraphyNode;
using  GraphyNodeSPtr = std::shared_ptr<GraphyNode>;

// 图节点
class TARO_DLL_EXPORT GraphyNode
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    * 
    * @param[in] name 节点名称
    */
    GraphyNode( const char* name );

    /**
    * @brief 析构函数
    */
    virtual ~GraphyNode();

    /**
    * @brief 获取名称
    */
    const char* name() const;

    /**
    * @brief 获取下游节点
    * 
    * @return 下游节点集合
    */
    std::vector<GraphyNodeSPtr> next() const;

PRIVATE: // 私有类型

    friend struct GraphyNodeImpl;

PRIVATE: // 私有函数

    TARO_NO_COPY( GraphyNode );

PRIVATE: // 私有变量

    GraphyNodeImpl* impl_;
};

// 图对象
class TARO_DLL_EXPORT Graph
{
PUBLIC: // 公共函数

    /**
    * @brief 构造函数
    */
    Graph();

    /**
    * @brief 析构函数
    */
    ~Graph();

    /**
    * @brief 添加节点
    * 
    * @param[in] node 节点对象
    */
    int32_t add_node( GraphyNodeSPtr const& node );

    /**
    * @brief 删除节点
    * 
    * @param[in] name 节点名称
    */
    int32_t remove_node( const char* name );

    /**
    * @brief 连接节点
    * 
    * @param[in] 源节点
    * @param[in] 目的节点
    */
    int32_t add_edge( const char* src, const char* dst );

    /**
    * @brief 断开节点
    * 
    * @param[in] 源节点
    * @param[in] 目的节点
    */
    int32_t remove_edge( const char* src, const char* dst );

    /**
    * @brief 获取图的入口节点
    * 
    * @return 入口节点集合
    */
    std::vector<GraphyNodeSPtr> entry() const;

    /**
    * @brief 成环判断
    */
    bool has_circle() const;

    /**
    * @brief 获取所有节点
    */
    std::vector<GraphyNodeSPtr> all_nodes() const;

PRIVATE: // 私有函数

    TARO_NO_COPY( Graph );

PRIVATE: // 私有变量

    struct Impl;
    Impl* impl_;
};

NAMESPACE_TARO_END
