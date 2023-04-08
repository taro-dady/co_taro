
#include "base/error_no.h"
#include "base/utils/assert.h"
#include "base/memory/impl/graph_impl.h"
#include <algorithm>

NAMESPACE_TARO_BEGIN

static void circle_check( GraphyNodeSPtr node, std::map< std::string, int32_t >& visited, std::string& node_name )
{
    auto it = visited.find( node->name() );
    TARO_ASSERT( it != visited.end(), "node not found" );
    ++it->second;

    if ( it->second > 1 )
    {
        node_name = node->name();
        return;
    }

    auto& out_nodes = GraphyNodeImpl::get( *node )->out_nodes_;
    if ( out_nodes.empty() )
    {
        return;
    }

    for ( auto const& one : out_nodes )
    {
        circle_check( one, visited, node_name );
        if ( !node_name.empty() )
        {
            return;
        }
        else
        {
            --visited[one->name()];
        }
    }
}

GraphyNode::GraphyNode( const char* name )
    : impl_( new GraphyNodeImpl )
{
    TARO_ASSERT( STRING_CHECK( name ) );
    impl_->name_ = name;
}

GraphyNode::~GraphyNode()
{
    delete impl_;
}

const char* GraphyNode::name() const
{
    return impl_->name_.c_str();
}

std::vector<GraphyNodeSPtr> GraphyNode::next() const
{
    return impl_->out_nodes_;
}

Graph::Graph()
    : impl_( new Impl )
{

}

Graph::~Graph()
{
    delete impl_;
}

int32_t Graph::add_node( GraphyNodeSPtr const& node )
{
    if ( nullptr == node || !STRING_CHECK( node->name() ) )
    {
        return TARO_ERR_INVALID_ARG;
    }

    if ( !impl_->nodes_.insert( std::make_pair( node->name(), node ) ).second )
    {
        return TARO_ERR_MULTI_OP;
    }
    return TARO_OK;
}

int32_t Graph::remove_node( const char* name )
{
    if ( !STRING_CHECK( name ) )
    {
        return TARO_ERR_INVALID_ARG;
    }
    return impl_->nodes_.erase( name ) > 0 ? TARO_OK : TARO_ERR_FAILED;
}

int32_t Graph::add_edge( const char* src, const char* dst )
{
    if ( !STRING_CHECK( src, dst ) )
    {
        return TARO_ERR_INVALID_ARG;
    }

    auto src_it = impl_->nodes_.find( src );
    auto dst_it = impl_->nodes_.find( dst );
    if ( src_it == impl_->nodes_.end() || dst_it == impl_->nodes_.end() )
    {
        return TARO_ERR_INVALID_RES;
    }

    auto& out = GraphyNodeImpl::get( *src_it->second )->out_nodes_;
    auto out_it = std::find_if( std::begin( out ), std::end( out ), [&]( GraphyNodeSPtr const& n )
    {
        return std::string( n->name() ) == dst;
    } );

    if ( out_it != std::end( out ) )
    {
        return TARO_ERR_INVALID_RES;
    }

    auto& in = GraphyNodeImpl::get( *dst_it->second )->in_nodes_;
    auto in_it = std::find_if( std::begin( in ), std::end( in ), [&]( GraphyNodeSPtr const& n )
    {
        return std::string( n->name() ) == src;
    } );

    if ( in_it != std::end( in ) )
    {
        return TARO_ERR_INVALID_RES;
    }

    out.push_back( dst_it->second );
    in.push_back( src_it->second );
    return TARO_OK;
}

int32_t Graph::remove_edge( const char* src, const char* dst )
{
    if ( !STRING_CHECK( src, dst ) )
    {
        return TARO_ERR_INVALID_ARG;
    }

    auto src_it = impl_->nodes_.find( src );
    auto dst_it = impl_->nodes_.find( dst );
    if ( src_it == impl_->nodes_.end() || dst_it == impl_->nodes_.end() )
    {
        return TARO_ERR_INVALID_RES;
    }

    auto& out = GraphyNodeImpl::get( *src_it->second )->out_nodes_;
    auto out_it = std::find_if( std::begin( out ), std::end( out ), [&]( GraphyNodeSPtr const& n )
    {
        return std::string( n->name() ) == dst;
    } );

    if ( out_it == std::end( out ) )
    {
        return TARO_ERR_INVALID_RES;
    }

    auto& in = GraphyNodeImpl::get( *dst_it->second )->in_nodes_;
    auto in_it = std::find_if( std::begin( in ), std::end( in ), [&]( GraphyNodeSPtr const& n )
    {
        return std::string( n->name() ) == src;
    } );

    if ( in_it == std::end( in ) )
    {
        return TARO_ERR_INVALID_RES;
    }

    out.erase( out_it );
    in.erase( in_it );
    return TARO_OK;
}

std::vector<GraphyNodeSPtr> Graph::entry() const
{
    std::vector<GraphyNodeSPtr> entry;
    for ( auto const& one : impl_->nodes_ )
    {
        if ( GraphyNodeImpl::get( *one.second )->in_nodes_.empty() )
        {
            entry.push_back( one.second );
        }
    }
    return entry;
}

bool Graph::has_circle() const
{
    if ( impl_->nodes_.empty() )
    {
        return false;
    }

    auto entry = Graph::entry();
    if ( entry.empty() )
    {
        return true;
    }

    std::map< std::string, int32_t > visited;
    for ( auto const& one : entry )
    {
        for ( auto const& one : impl_->nodes_ )
        {
            visited[one.first] = 0;
        }

        std::string error_node;
        circle_check( one, visited, error_node );
        if ( !error_node.empty() )
        {
            return true;
        }
        else
        {
            --visited[one->name()];
        }
    }
    return false;
}

NAMESPACE_TARO_END
