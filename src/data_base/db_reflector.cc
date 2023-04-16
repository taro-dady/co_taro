
#include "base/system/spin_locker.h"
#include "data_base/db_reflector.h"
#include <map>
#include <mutex>

NAMESPACE_TARO_DB_BEGIN

using ReflectorMap = std::map< std::string, ClsMemberReflectorSPtr >;

// 反射对象
struct DBReflector::Impl
{
    SpinLocker locker_;
    std::map< const std::type_info*, std::string > cls_;
    std::map< const std::type_info*, ReflectorMap > reflectors_;
};

DBReflector& DBReflector::instance()
{
    static DBReflector inst;
    return inst;
}

DBReflector::DBReflector()
    : impl_( new Impl )
{

}

DBReflector::~DBReflector()
{
    delete impl_;
}

bool DBReflector::register_class( const char* cls, const std::type_info* info )
{
    TARO_ASSERT( STRING_CHECK( cls ) && info );

    std::unique_lock<SpinLocker> g( impl_->locker_ );
    auto ret = impl_->cls_.insert( std::make_pair( info, cls ) ).second;
    TARO_ASSERT( ret, "mutiple register", cls );
    return ret;
}

std::string DBReflector::find_class_name( const std::type_info* info ) const
{
    TARO_ASSERT( nullptr != info );

    std::unique_lock<SpinLocker> g( impl_->locker_ );
    auto iter = impl_->cls_.find( info );
    if( iter == impl_->cls_.end() )
    {
        return "";
    }
    return iter->second;
}

bool DBReflector::register_member( const std::type_info* cls, const char* mb, ClsMemberReflectorSPtr const& reflector )
{
    TARO_ASSERT( ( nullptr != cls ) && STRING_CHECK( mb ) && reflector );

    std::unique_lock<SpinLocker> g( impl_->locker_ );
    auto iter = impl_->reflectors_.find( cls );
    if( iter == impl_->reflectors_.end() )
    {
        impl_->reflectors_[cls][mb] = reflector;
        return true;
    }
    return iter->second.insert( std::make_pair( mb, reflector ) ).second;
}

std::vector<ClsMemberReflectorSPtr> DBReflector::get_member_reflectors( const std::type_info* cls )
{
    TARO_ASSERT( cls );

    std::vector<ClsMemberReflectorSPtr> res;
    std::unique_lock<SpinLocker> g( impl_->locker_ );
    auto iter = impl_->reflectors_.find( cls );
    if( iter == impl_->reflectors_.end() )
    {
        return res;
    }

    for( auto& one : iter->second )
    {
        res.push_back( one.second );
    }
    return res;
}

NAMESPACE_TARO_DB_END
