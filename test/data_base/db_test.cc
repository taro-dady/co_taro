
#include "data_base/inc.h"

USING_NAMESPACE_TARO

namespace company{
    struct Employee
    {
        // 申明映射关系
        TARO_DB_DEFINE( Employee, identity, age, name, salary, sex, floor );

        uint32_t identity;
        int32_t age;
        std::string name;
        double salary;
        bool sex;
        int64_t floor;
    };
}

inline std::ostream& operator<<(std::ostream& os, company::Employee const& one)
{
    os << "id:" << one.identity 
       << " name:" << one.name 
       << " age:" << one.age 
       << " salary:" << one.salary 
       << " floor:" << one.floor 
       << " sex:" << ( one.sex ? "male" : "female" )
       << std::endl;
    return os;
}

void init_employee( std::vector<company::Employee>& employee )
{
    {
        company::Employee one;
        one.name = "Jack";
        one.age = 20;
        one.salary = 3000.033;
        one.sex = true;
        one.floor = 22;
        employee.push_back(one);
    }

    {
        company::Employee one;
        one.name = "Henry";
        one.age = 35;
        one.salary = 25000.89;
        one.sex = true;
        one.floor = 23;
        employee.push_back(one);
    }

    {
        company::Employee one;
        one.name = "Mayer";
        one.age = 28;
        one.salary = 8500;
        one.sex = false;
        one.floor = 22;
        employee.push_back(one);
    }

    {
        company::Employee one;
        one.name = "Angelia";
        one.age = 38;
        one.salary = 58123.86;
        one.sex = false;
        one.floor = 22;
        employee.push_back(one);
    }
}

db::DataBaseSPtr db_mysql_create()
{
    /* 
    mysql -u root -p
    select user from mysql.user;
    create user runoon@localhost identified by 'runoon12345';
    GRANT ALL PRIVILEGES ON *.* TO runoon@localhost;
    FLUSH PRIVILEGES;
    create database test;
    */

#if defined(_WIN32) || defined(_WIN64)
    // dll https://downloads.mysql.com/archives/c-c/  Windows(x86, 64-bit), ZIP Archive
    TARO_ASSERT( TARO_OK == db::load_mysql_library( "D:\\WorkSpace\\mysql-connector-c-6.1.11-winx64\\lib\\libmysql.dll" ) );
#else
    // 加载动态库，减少MySQL的依赖，使用时安装即可
    TARO_ASSERT( TARO_OK == db::load_mysql_library( "/usr/lib/x86_64-linux-gnu/libmysqlclient.so" ) );
#endif

    // 创建数据库对象
    auto db = db::create_database( DB_TYPE_MYSQL );

    db::DBUri uri;
    uri << db_opt_host( "192.168.65.3" )   // mysql server host address
        << db_opt_port( 3306 ) 
        << db_opt_user( "runoon" ) 
        << db_opt_pwd( "runoon12345" )
        << db_opt_db( "test" );
    
    auto ret = db->connect( uri );
    TARO_ASSERT( ret == TARO_OK );
    return db;
}

db::DataBaseSPtr db_sqlite_create()
{
    // 创建数据库对象
    auto db = db::create_database( DB_TYPE_SQLITE );

    // 连接数据库(SQLITE打开文件)
    TARO_ASSERT( db->connect( db::DBUri( "sqlite_test.db" ) ) == TARO_OK );
    return db;
}

void db_mysql_test_by_sql()
{
    auto db = db_mysql_create();

    std::string sql = 
       "create table if not exists Employee(identity int primary key auto_increment,age int,name text,salary double,sex boolean);";
    auto ret = db->excute_cmd( sql.c_str() );
    TARO_ASSERT( ret == TARO_OK );

    sql = db::DBFormat( "insert into Employee (age,identity,name,salary,sex) values (?,null,?,?,?);" ) << 50 << "Jack" << 3000.03 << 1;
    ret = db->excute_cmd( sql.c_str() );
    TARO_ASSERT( ret == TARO_OK );

    auto query_res = db->query( "select * from Employee;" );
    TARO_ASSERT( query_res != nullptr );

    auto cols = query_res->get_columns();
    TARO_ASSERT( cols.size() == 5 );
    do{
        for(size_t i = 0; i < cols.size(); ++i)
            std::cout << cols[i] << ":" << query_res->get_col_val( ( int )i ) << ",";
        std::cout << std::endl;
    }while ( query_res->next() );

    printf("update --------------\n");

    sql = db::DBFormat( "update Employee set age=? where name = ?;" ) << 20 << "Jack";
    ret = db->excute_cmd( sql.c_str() );
    TARO_ASSERT( ret == TARO_OK );

    query_res = db->query( "select * from Employee;" );
    TARO_ASSERT( query_res != nullptr );
    cols = query_res->get_columns();
    TARO_ASSERT( cols.size() == 5 );
    do{
        for(size_t i = 0; i < cols.size(); ++i)
            std::cout << cols[i] <<":"<< query_res->get_col_val( ( int )i) << ",";
        std::cout << std::endl;
    }while ( query_res->next() );

    printf("remove --------------\n");
    ret = db->excute_cmd( "delete from Employee;" );
    TARO_ASSERT( ret == TARO_OK );

    printf("drop --------------\n");
    ret = db->excute_cmd( "drop table Employee;" );
    TARO_ASSERT( ret == TARO_OK );
}

void db_test( db::DataBaseSPtr db )
{
    // 删除表
    db->drop_table<company::Employee>();

    // 建表，参数为约束条件
    auto ret = db->create_table<company::Employee>( DB_CSTR( company::Employee::identity ).primary_key().auto_inc(), db::create_if_not_exist() );
    TARO_ASSERT( ret == TARO_OK );

    // 插入数据
    std::vector<company::Employee> employee;
    init_employee( employee );

    uint64_t index = 1;
    for( auto const& one : employee )
    {
        auto ret = db->insert_ret_id( one, DB_NULL( company::Employee::identity ) ); // identity auto inc
        TARO_ASSERT( ret.valid() && index == ret.value() );
        ++index;
    }

    {
        // 查询全部数据
        auto result = db->query<company::Employee>();
        TARO_ASSERT( 4 == result.size() );

        std::cout << "----query test-----" << std::endl;
        for( auto const& one : result )
        {
            std::cout << one;
        }
    }

    {
        // 条件查询
        auto result = db->query<company::Employee>(
            //DB_BLACK << DB_MEM( company::Employee::name ), // 不查询name
            DB_COND( company::Employee::age ) > 20 && DB_COND( company::Employee::age ) <= 35 && DB_COND( company::Employee::name ).is_null( false ), // 查询条件
            DB_LIMIT( 3, 0 ),  // 查询范围 3行数据，起始位置为0
            DB_DESCEND( company::Employee::age ) ); // 升序还是降序DB_ASCEND
        std::cout << "----condition query test-----" << std::endl;
        TARO_ASSERT( 2 == result.size() );
        for( auto& one : result )
        {
            std::cout << one;
        }
    }

    {
        std::cout << "----caculate test-----" << std::endl;

        // 计算各项值
        auto avg = db->average<company::Employee>( DB_MEM( company::Employee::salary ) );
        TARO_ASSERT( avg.valid() );
        std::cout << "avg:" << avg.value() << std::endl;

        auto sum = db->sum<company::Employee>( DB_MEM( company::Employee::salary ) );
        TARO_ASSERT( sum.valid() );
        std::cout << "sum:" << sum.value() << std::endl;

        auto cnt = db->count<company::Employee>();
        TARO_ASSERT( cnt.valid() );
        std::cout << "cnt:" << cnt.value() << std::endl;
    }

    {
        std::cout << "----condition caculate test-----" << std::endl;

        // 计算各项值
        auto avg = db->average<company::Employee>( DB_MEM( company::Employee::salary ), DB_COND( company::Employee::age ) > 25 );
        TARO_ASSERT( avg.valid() );
        std::cout << "avg:" << avg.value() << std::endl;

        auto sum = db->sum<company::Employee>( DB_MEM( company::Employee::salary ), DB_COND( company::Employee::age ) > 25 );
        TARO_ASSERT( sum.valid() );
        std::cout << "sum:" << sum.value() << std::endl;

        auto cnt = db->count<company::Employee>( DB_COND( company::Employee::salary ) > 10000  && DB_COND( company::Employee::age ) > 25 );
        TARO_ASSERT( cnt.valid() );
        std::cout << "cnt:" << cnt.value() << std::endl;
    }

    // Mayer 年龄修改为30，DB_WHITE 表示只更新age 如果有其他选项可以继续以 << DB_MEM() 的方式添加 
    company::Employee mayer;
    mayer.age = 30;
    mayer.name = "Mayer";
    ret = db->update( mayer, DB_WHITE << DB_MEM( company::Employee::age ), DB_COND( company::Employee::name ) = mayer.name );
    TARO_ASSERT( ret == TARO_OK );
    {
        std::cout << "----update test-----" << std::endl;
        auto result = db->query<company::Employee>( DB_COND( company::Employee::name ) = mayer.name );
        TARO_ASSERT( 1 == result.size() );
        for( auto& one : result )
        {
            std::cout << one;
        }
    }

    ret = db->remove<company::Employee>( DB_COND( company::Employee::name ) = "Jack");
    TARO_ASSERT( ret == TARO_OK );
    {
        std::cout << "----remove test-----" << std::endl;
        auto result = db->query<company::Employee>();
        TARO_ASSERT( 3 == result.size() );
        for( auto& one : result )
        {
            std::cout << one;
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

    log::set_sys_level( log::eLogLevelDebug );
    switch ( atoi( argv[1] ) )
    {
    case 0:
        db_test( db_sqlite_create() );
        break;
    case 1:
        db_mysql_test_by_sql();
        break;
    case 2:
        db_test( db_mysql_create() );
        break;
    }
    return 0;
}
