
#include "data_base/impl/db_helper_impl.h"

NAMESPACE_TARO_DB_BEGIN


DBContraint::DBContraint( const char* name )
    : impl_( new DBContraintImpl )
{
    TARO_ASSERT( STRING_CHECK( name ) );
    impl_->name_ = name;
}

DBContraint::DBContraint( DBContraint&& other )
    : impl_( other.impl_ )
{
    other.impl_ = nullptr;
}

DBContraint::~DBContraint()
{
    if( nullptr == impl_ )
    {
        return;
    }

    delete impl_;
    impl_ = nullptr;
}

DBContraint& DBContraint::not_null()
{
    impl_->kinds_.insert( make_constraint( eDBConstraintNotNull ) );
    return *this;
}

DBContraint& DBContraint::unique()
{
    impl_->kinds_.insert( make_constraint( eDBConstraintUnique ) );
    return *this;
}

DBContraint& DBContraint::primary_key()
{
    impl_->kinds_.insert( make_constraint( eDBConstraintPrimaryKey ) );
    return *this;
}

DBContraint& DBContraint::foreign_key( std::string const& tbl, std::string const& col )
{
    std::stringstream ss;
    ss << tbl << "(" << col << ")";
    impl_->kinds_.insert( make_constraint( eDBConstraintForeignKey, ss.str() ) );
    return *this;
}

DBContraint& DBContraint::auto_inc()
{
    impl_->kinds_.insert( make_constraint( eDBConstraintAutoInc ) );
    return *this;
}

DBContraint& DBContraint::default_val()
{
    impl_->kinds_.insert( make_constraint( eDBConstraintDefault ) );
    return *this;
}

void DBContraint::add_constraint( DBContraintArgBaseSPtr const& arg )
{
    TARO_ASSERT( nullptr != arg );
    impl_->kinds_.insert( arg );
}

std::string const& DBContraint::name() const
{
    return impl_->name_;
}

NAMESPACE_TARO_DB_END
