
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

NAMESPACE_TARO_DB_END
