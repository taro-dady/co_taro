
#include "base/serialize/str_serialize.h"
#include "data_base/db_reflector.h"
#include "data_base/data_base.h"

USING_NAMESPACE_TARO

namespace aa{
    struct Employee
    {
        TARO_DB_DEFINE( Employee, age, name );

        int age = 0;
        std::string name;
    };
}

void db_test()
{
    aa::Employee::db_cls_reflect();

    aa::Employee temp;
    temp.age = 200;
    temp.name = "hello";

    auto reflector = db::DBReflector::instance().get_member_reflectors( &typeid( aa::Employee ) );
    for( auto& one : reflector )
    {
        printf( "%s %s\n", one->get_name().c_str(), one->serialize( ( void* )&temp ).c_str() );
    }

    reflector[0]->deserialize( "100", ( void* )&temp );
    reflector[1]->deserialize( "world", ( void* )&temp );
    printf( "%d %s\n", temp.age, temp.name.c_str() );
}

void db_test1()
{
    // 创建数据库对象
    auto db = db::create_database();

    // 连接数据库(SQLITE打开文件)
    db->connect( "sqlite_test.db" );

    // 建表，参数为约束条件
    db->create_table<aa::Employee>( DB_CSTR( aa::Employee::name ).primary_key().not_null(), db::create_if_not_exist() );
}

int main( int argc, char** argv )
{
    if ( argc < 2 )
    {
        perror( "parameter error\n" ); 
        exit( 0 );
    }

    switch ( atoi( argv[1] ) )
    {
    case 0:
        db_test1();
        break;
    }
    return 0;
}
