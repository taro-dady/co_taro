
#pragma once

#include "base/memory/graph.h"
#include <map>

NAMESPACE_TARO_BEGIN

struct GraphyNodeImpl
{
    static GraphyNodeImpl* get( GraphyNode& node )
    {
        return node.impl_;
    }

    std::string name_; // 节点名称
    std::vector<GraphyNodeSPtr> in_nodes_;  // 上游节点
    std::vector<GraphyNodeSPtr> out_nodes_; // 下游节点
};

struct Graph::Impl
{
    std::map< std::string, GraphyNodeSPtr > nodes_;
};

NAMESPACE_TARO_END
