
#include "task_flow/inc.h"
#include "co_routine/inc.h"
#include "base/system/thread.h"
#include <thread>

USING_NAMESPACE_TARO
USING_NAMESPACE_TARO_TASKFLOW

static std::atomic_uint32_t msg_seq( 0 );

struct SourceData : PUBLIC TaskMessage
{
    SourceData( const char* src, int32_t cnt )
        : TaskMessage( src, "source", ++msg_seq )
        , count_( cnt )
    {}

    int32_t count_;
};  

struct AddData : PUBLIC TaskMessage
{
    AddData( const char* src, int32_t cnt, uint32_t seq )
        : TaskMessage( src, "add", seq )
        , count_( cnt )
    {}

    int32_t count_;
};  

void source_tigger( TaskNodeSPtr node )
{ 
    static int32_t out_put_cnt = 0;
    std::thread( [node]()
    {
        while(1)
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
            node->feed( std::make_shared<SourceData>( node->name(), ++out_put_cnt) ); 
        }
    } ).detach();
}

struct AddNode : PUBLIC TaskNode
{
    AddNode( const char* name ) 
        : TaskNode( name )
    {}

PRIVATE:

    virtual TaskMessageSPtr on_message( TaskMessageSPtr const& msg ) override final
    {
        if ( std::string( "source" ) == msg->type() )
        {
            auto arg = std::dynamic_pointer_cast<SourceData>( msg );
            //printf( "%d %d %ld\n", arg->count_, arg->sequence(), arg->time_stamp() );
            return std::make_shared<AddData>( name(), arg->count_ + arg->count_, msg->sequence() );
        }
        return nullptr;
    }
};

struct MergeNode : PUBLIC TaskNode
{
    MergeNode( const char* name ) 
        : TaskNode( name )
    {}

PRIVATE:

    virtual TaskMessageSPtr on_message( TaskMessageSPtr const& msg ) override final
    {
        auto show = [](std::pair<TaskMessageSPtr, TaskMessageSPtr>& in)
        {
            auto arg1 = std::dynamic_pointer_cast<AddData>( in.first );
            auto arg2 = std::dynamic_pointer_cast<AddData>( in.second );
            printf( "merge ok %d %d %d\n", arg1->count_, arg2->count_, arg1->sequence() );
        };

        if ( std::string( "add1" ) == msg->source() )
        {
            std::lock_guard<std::mutex> locker( mutex_ );
            sync_buff_[msg->sequence()].first = msg;
            if( sync_buff_[msg->sequence()].second )
            {
                show( sync_buff_[msg->sequence()] );
                sync_buff_.erase( msg->sequence() );
            }
        }
        else if( std::string( "add2" ) == msg->source() )
        {
            std::lock_guard<std::mutex> locker( mutex_ );
            sync_buff_[msg->sequence()].second = msg;
            if( sync_buff_[msg->sequence()].first )
            {
                show( sync_buff_[msg->sequence()] );
                sync_buff_.erase( msg->sequence() );
            }
        }
        return nullptr;
    }

    std::mutex mutex_;
    std::map< uint32_t, std::pair<TaskMessageSPtr, TaskMessageSPtr> > sync_buff_;
};

void task_flow_test()
{
    auto source_node = std::make_shared<TaskNode>( "source" );

    TaskScheduler sched;
    sched.add_node( source_node );
    sched.add_node( std::make_shared<AddNode>( "add1" ) );
    sched.add_node( std::make_shared<AddNode>( "add2" ) );
    sched.add_node( std::make_shared<MergeNode>( "merge" ) );

    sched.add_edge( source_node->name(), "add1" );
    sched.add_edge( source_node->name(), "add2" );
    sched.add_edge( "add1", "merge" );
    sched.add_edge( "add2", "merge" );

    sched.start( 2 );

    source_tigger( source_node );
    Thread::sleep( 3000 );
    sched.stop();

    TASKFLOW_WARN << "test over";
}

int main( int argc, char** argv )
{
    if ( argc < 2 )
    {
        perror( "parameter error\n" ); 
        exit( 0 );
    }

    rt::CoRoutine::init();
    switch ( atoi( argv[1] ) )
    {
    case 0:
        task_flow_test();
        break;
    }
    return 0;
}
