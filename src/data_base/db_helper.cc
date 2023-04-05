
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

DBCond::DBCond()
    : impl_( nullptr )
{

}

DBCond::DBCond( const char* name )
    : impl_( new DBCondImpl )
{
    impl_->curr_.name_ = name;
}

DBCond::DBCond( DBCond&& other )
    : impl_( other.impl_ )
{
    other.impl_ = nullptr;
}

DBCond::~DBCond()
{
    if ( nullptr != impl_ )
    {
        delete impl_;
        impl_ = nullptr;
    }
}

DBCond& DBCond::operator=( DBCond&& other )
{
    if ( impl_ != nullptr )
    {
        delete impl_;
    }
    impl_ = other.impl_;
    other.impl_ = nullptr;
    return *this;
}

DBCond& DBCond::is_null( bool null )
{
    impl_->curr_.op_ = ( null ? "is" : "is not" );
    impl_->curr_.value_ = "null";
    return *this;
}

DBCond& DBCond::operator&&( DBCond const& c )
{
    TARO_ASSERT( !c.impl_->curr_.name_.empty() &&!c.impl_->curr_.op_.empty() && !c.impl_->curr_.value_.empty() );

    if ( !impl_->curr_.name_.empty() )
    {
        TARO_ASSERT( !impl_->curr_.op_.empty() && !impl_->curr_.value_.empty() );
        impl_->conds_.push_back( impl_->curr_ );
        impl_->clear();
    }
    impl_->relationship_.push_back( "and" );
    impl_->conds_.push_back( c.impl_->curr_ );
    return *this;
}

DBCond& DBCond::operator||( DBCond const& c )
{
    TARO_ASSERT( !c.impl_->curr_.name_.empty() &&!c.impl_->curr_.op_.empty() && !c.impl_->curr_.value_.empty() );
    
    if ( !impl_->curr_.name_.empty() )
    {
        TARO_ASSERT( !impl_->curr_.op_.empty() && !impl_->curr_.value_.empty() );
        impl_->conds_.push_back( impl_->curr_ );
        impl_->clear();
    }
    impl_->relationship_.push_back( "or" );
    impl_->conds_.push_back( c.impl_->curr_ );
    return *this;
}

void DBCond::set_value( std::string const& op, std::string const& val )
{
    TARO_ASSERT( !op.empty() && !val.empty(), "set value invalid argument" );

    impl_->curr_.op_ = op;
    impl_->curr_.value_ = val;
}

DBFilter::DBFilter()
    : impl_( nullptr )
{

}

DBFilter::DBFilter( bool black )
    : impl_( new DBFilterImpl )
{
    impl_->black_ = black;
}

DBFilter::DBFilter( DBFilter&& other )
    : impl_( other.impl_ )
{
    other.impl_ = nullptr;
}

DBFilter::~DBFilter()
{
    if ( nullptr != impl_ )
    {
        delete impl_;
        impl_ = nullptr;
    }
}

DBFilter& DBFilter::operator=( DBFilter&& other )
{
    if ( impl_ != nullptr )
    {
        delete impl_;
    }
    impl_ = other.impl_;
    other.impl_ = nullptr;
    return *this;
}

DBFilter& DBFilter::operator<<( std::string const& name )
{
    TARO_ASSERT( !name.empty() );
    impl_->names_.insert( name );
    return *this;
}

struct DBNull::Impl
{
    std::string name_;
};

DBNull::DBNull( const char* name )
    : impl_( new Impl )
{
    impl_->name_ = name;
}

DBNull::~DBNull()
{
    delete impl_;
}

const char* DBNull::name()
{
    return impl_->name_.c_str();
}

struct DBOrder::Impl
{
    bool ascend_;
    std::string name_;
};

DBOrder::DBOrder( const char* name, bool asc )
    : impl_( new Impl )
{
    impl_->name_   = name;
    impl_->ascend_ = asc;
}

DBOrder::~DBOrder()
{
    delete impl_;
}

const char* DBOrder::name()
{
    return impl_->name_.c_str();
}

bool DBOrder::ascend() const
{
    return impl_->ascend_;
}

NAMESPACE_TARO_DB_END
