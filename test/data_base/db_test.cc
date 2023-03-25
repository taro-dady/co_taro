
#include "base/serialize/str_serialize.h"
#include "data_base/db_reflector.h"

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
        db_test();
        break;
    }
    return 0;
}
