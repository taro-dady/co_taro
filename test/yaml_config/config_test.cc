
#include "base/config/yaml_config.h"
#include "base/utils/assert.h"
#include "base/error_no.h"
#include "base/utils/arg_parser.h"

USING_NAMESPACE_TARO

TARO_ARG_DEF_DECL( int, test_type, -1 );

struct TestCfg1
{
    TARO_YAML_DEFINE(name, ip, port)

    std::string name;
    std::string ip;
    uint16_t    port;
};

bool operator==( TestCfg1 const& one, TestCfg1 const& one1 ){
    if( one1.name != one.name || one1.ip != one.ip || one1.port != one.port )
        return false;
    return true;
}

struct TestCfg2
{
    TARO_YAML_DEFINE( count, cfgs1, cfgs2, cfgs3, m1, m2, m3, t1, t2, o1, o2, o3, o4, o5 )

    int32_t count;
    std::list<TestCfg1> cfgs1;
    std::vector<TestCfg1> cfgs2;
    std::vector< Optional<TestCfg1> > cfgs3;
    std::map<int, std::string> m1;
    std::map<std::string, TestCfg1> m2;
    std::map<std::string, Optional<std::string>> m3;
    std::tuple<int, float, TestCfg1> t1;
    std::tuple<int, float, Optional<TestCfg1> > t2;
    Optional<int> o1;
    Optional<TestCfg1> o2;
    Optional< std::map<std::string, TestCfg1> > o3;
    Optional< std::vector<TestCfg1> > o4;
    Optional< std::tuple<int, float, TestCfg1> > o5;
};

void test1()
{
    ::remove( "cfg.yml" );
    ::remove( "cfg_back.yml" );

    {
        auto cfg = std::make_shared<YamlConfig>();
        cfg->set_param( "cfg.yml", "cfg_back.yml" );
        TestCfg1 test;
        test.name = "test1";
        test.ip = "127.0.0.1";
        test.port = 20000;
        cfg->set_config( test );
    }
    
    {
        auto cfg = std::make_shared<YamlConfig>();
        cfg->set_param( "cfg.yml", "cfg_back.yml" );
        TestCfg1 test;
        TARO_ASSERT( cfg->load() == TARO_OK, "load failed" );
        TARO_ASSERT( cfg->get_config( test ), "get config failed" );
        TARO_ASSERT( test.name == "test1", "test name" );
        TARO_ASSERT( test.ip == "127.0.0.1", "test ip" );
        TARO_ASSERT( test.port == 20000, "test port" );
    }
}

void test2()
{
    std::cout << "test2"<< std::endl;

    ::remove( "cfg.yml" );
    ::remove( "cfg_back.yml" );

    TestCfg1 test;
    test.name = "test1";
    test.ip = "127.0.0.1";
    test.port = 20000;
    {
        auto cfg = std::make_shared<YamlConfig>();
        cfg->set_param( "cfg.yml", "cfg_back.yml" );
        
        TestCfg2 t;
        t.count = 10;
        t.cfgs1.emplace_back( test );
        t.cfgs2.emplace_back( test );
        t.cfgs3.emplace_back( test );
        t.m1[-100] = "hello";
        t.m2["test"] = test;
        t.m3["test"] = "hello";
        t.t1 = std::make_tuple(2, ( float )-100.99, test);
        t.t2 = std::make_tuple( 2, ( float )-100.99, test );
        t.o1 = -290;
        t.o2 = test;
        t.o3 = t.m2;
        t.o4 = t.cfgs2;
        t.o5 = t.t1;
        cfg->set_config( t );
    }

    {
        auto cfg = std::make_shared<YamlConfig>();
        cfg->set_param( "cfg.yml", "cfg_back.yml" );
        TestCfg2 test2;
        TARO_ASSERT( cfg->load() == TARO_OK, "load failed" );
        TARO_ASSERT( cfg->get_config( test2 ), "get config failed" );
        TARO_ASSERT( test2.count == 10 );
        TARO_ASSERT( test2.cfgs1.size() == 1, test2.cfgs1.size() );
        TARO_ASSERT( test2.cfgs1.front() == test );
        TARO_ASSERT( test2.cfgs2.size() == 1, test2.cfgs2.size() );
        TARO_ASSERT( test2.cfgs2.front() == test );
        TARO_ASSERT( test2.cfgs3.front().value() == test );
        TARO_ASSERT( test2.m1.size() == 1 );
        TARO_ASSERT( test2.m1[-100] == "hello" );
        TARO_ASSERT( test2.m2.size() == 1 );
        TARO_ASSERT( test2.m2["test"] == test );
        TARO_ASSERT( test2.m3["test"].value() == "hello");
        TARO_ASSERT( std::get<0>(test2.t1) == 2 && std::get<1>( test2.t1 ) == ( float )-100.99 && std::get<2>( test2.t1 ) == test );
        TARO_ASSERT( std::get<0>( test2.t2 ) == 2 && std::get<1>( test2.t2 ) == ( float )-100.99 && std::get<2>( test2.t2 ).value() == test );
        TARO_ASSERT( test2.o1.value() == -290 );
        TARO_ASSERT( test2.o2.value() == test );
        TARO_ASSERT( test2.o3.valid() && test2.o3.value()["test"] == test);
        TARO_ASSERT( test2.o4.valid() && test2.o4.value()[0] == test);
        TARO_ASSERT( std::get<0>( test2.o5.value() ) == 2 && std::get<1>( test2.o5.value() ) == ( float )-100.99 && std::get<2>( test2.o5.value() ) == test );
    }

    std::cout << "test2 ok" << std::endl;
}

int main( int argc, char* argv[] )
{
    parse_args( argc, argv );
    if( taro_arg_test_type == 1 )
        test1();
    else if( taro_arg_test_type == 2 )
        test2();
    else
        printf( "type unsupport %d\n", taro_arg_test_type );
    return 0;
}