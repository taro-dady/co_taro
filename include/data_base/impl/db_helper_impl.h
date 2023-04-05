
#pragma once

#include "data_base/db_helper.h"
#include <set>

NAMESPACE_TARO_DB_BEGIN

struct DBContraintImpl
{
PUBLIC: // 公共函数

    static DBContraintImpl* get( DBContraint const& contraint )
    {
        return contraint.impl_;
    }

PUBLIC: // 公共类型
    
    std::string name_; // 约束条件名称
    std::set<DBContraintArgBaseSPtr> kinds_; // 约束条件集合
};

struct DBCondElement
{
    std::string name_;
    std::string op_;
    std::string value_;
};

static std::string element_str( DBCondElement const& e )
{
    return e.name_ + " " + e.op_ + " " + e.value_;
} 

using DBCondContainer = std::vector< DBCondElement >;

struct DBCondImpl
{
    static std::string to_str( DBCond const& cond )
    {
        if( cond.impl_ == nullptr 
        || ( cond.impl_->curr_.name_.empty() && cond.impl_->conds_.empty() ) )
        {
            return "";
        }

        if ( !cond.impl_->curr_.name_.empty() )
        {
            TARO_ASSERT( cond.impl_->conds_.empty() );
            return element_str( cond.impl_->curr_ );
        }

        TARO_ASSERT( cond.impl_->conds_.size() == ( cond.impl_->relationship_.size() + 1 ) );
        std::stringstream ss;
        ss << element_str( cond.impl_->conds_.front() );
        for ( size_t i = 0; i < cond.impl_->relationship_.size(); ++i )
        {
            ss << " " << cond.impl_->relationship_[i] << " " << element_str( cond.impl_->conds_[i + 1] );
        }
        return ss.str();
    }

    void clear()
    {
        curr_.name_  = "";
        curr_.op_    = "";
        curr_.value_ = "";
    }

    DBCondElement   curr_; // 当前条件
    DBCondContainer conds_; // 条件集合
    std::vector<std::string> relationship_; // 条件间的关系 
};

struct DBFilterImpl
{
    static DBFilterImpl* get( DBFilter const& filter )
    {
        return filter.impl_;
    }

    bool black_;
    std::set<std::string> names_;
};

NAMESPACE_TARO_DB_END
