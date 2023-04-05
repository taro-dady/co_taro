
#include "base/serialize/str_serialize.h"
#include "data_base/db_reflector.h"
#include "data_base/data_base.h"

USING_NAMESPACE_TARO

namespace aa{
    struct Employee
    {
        // 申明映射关系
        TARO_DB_DEFINE( Employee, age, name );

        Employee() = default;
        Employee( int a, const char* n )
            : age( a )
            , name( n )
        {}

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
    auto ret = db->connect( "sqlite_test.db" );
    TARO_ASSERT( ret == TARO_OK );

    // 删除表
    db->drop_table<aa::Employee>();

    // 建表，参数为约束条件
    ret = db->create_table<aa::Employee>( DB_CSTR( aa::Employee::name ).primary_key().not_null(), db::create_if_not_exist() );
    TARO_ASSERT( ret == TARO_OK );

    // 插入数据
    ret = db->insert( aa::Employee( 20, "jack" ) );
    TARO_ASSERT( ret == TARO_OK );
    ret = db->insert( aa::Employee( 9,  "jack1" ) );
    TARO_ASSERT( ret == TARO_OK );
    ret = db->insert( aa::Employee( 23, "jack2" ) );
    TARO_ASSERT( ret == TARO_OK );
    ret = db->insert( aa::Employee( 17, "jack3" ) );
    TARO_ASSERT( ret == TARO_OK );

    {
        // 查询全部数据
        auto result = db->query<aa::Employee>();
        TARO_ASSERT( 4 == result.size() );
        for( auto& one : result )
        {
            std::cout << one.name << " " << one.age << std::endl;
        }
    }

    {
        // 条件查询
        auto result = db->query<aa::Employee>(
            DB_BLACK << DB_MEM( aa::Employee::name ), // 不查询name
            DB_COND( aa::Employee::age ) > 10 && DB_COND( aa::Employee::age ) <= 20 && DB_COND( aa::Employee::name ).is_null( false ), // 查询条件
            DB_LIMIT( 3, 0 ),  // 查询范围
            DB_DESCEND( aa::Employee::age ) ); // 升序还是降序DB_ASCEND
        std::cout << "query 2 --------------" << std::endl;
        TARO_ASSERT( 2 == result.size() );
        for( auto& one : result )
        {
            std::cout << one.name << " " << one.age << std::endl;
        }
    }

    // jack3 年龄修改为30，DB_WHITE 表示只更新age 如果有其他选项可以继续以 << DB_MEM() 的方式添加 
    ret = db->update( aa::Employee( 30, "" ), DB_WHITE << DB_MEM( aa::Employee::age ), DB_COND( aa::Employee::name ) = "jack3" );
    TARO_ASSERT( ret == TARO_OK );
    {
        auto result = db->query<aa::Employee>( DB_COND( aa::Employee::name ) = "jack3" );
        TARO_ASSERT( 1 == result.size() );
        for( auto& one : result )
        {
            std::cout << one.name << " " << one.age << std::endl;
        }
    }
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
