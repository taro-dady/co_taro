
#include "base/task_que.h"
#include <chrono>
#include <ctime>
#include <list>

USING_NAMESPACE_TARO

#define TEST_NUMBER 5000000

struct DlTest : PUBLIC DoubleListHead, PUBLIC RefObject {
    DlTest( uint32_t i ) : index(i){}
    uint32_t index;
};

void dl_test() {
    DoubleList<LockFreeMutex, false> dl;
    uint32_t count = TEST_NUMBER;
    while( count > 0 ) {
        dl.push( new DlTest( count ) );
        --count;
    }

    while( !dl.empty() ) {
        DlTest* tmp;
        dl.pop( ( DoubleListHead** )&tmp );
        decrement_ref(tmp);
    }
}

void dl_move() {
    DoubleList<std::mutex> dl;
    uint32_t count = 20;
    while( count > 0 ) {
        dl.push( new DlTest( count ) );
        --count;
    }
    uint32_t size = 0;
    DoubleListHead *head, *tail;
    dl.remove_all( &head, &tail, size );
    
    DoubleList<std::mutex> dl1;
    dl1.append( head, tail, size );
    while( !dl1.empty() ) {
        DlTest* tmp;
        dl1.pop( ( DoubleListHead** )&tmp );
        decrement_ref(tmp);
        printf("tmp %d\n", tmp->index);
    }
    printf( "dl size %d\n", dl.size() );
}

void list_test() {
    std::list<uint32_t> ll;
    uint32_t count = TEST_NUMBER;
    while( count > 0 ) {
        ll.push_back( count );
        --count;
    }

    while( !ll.empty() ) {
        auto i = ll.front();
        ll.pop_front();
    }
}

void compare_test() {
    uint32_t total = 20, bigger = 0, less = 0;
    int64_t avg = 0;
    while( total-- > 0 ) {
        int64_t cost = 0;
        {
            auto now = std::chrono::steady_clock::now();
            list_test();
            cost = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - now ).count();
            //printf("cost %ld\n", std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - now ).count() );
        }

        {
            auto now = std::chrono::steady_clock::now();
            dl_test();
            auto tmp = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - now ).count();
            if ( tmp < cost ) {
                ++less;
                avg += ( cost - tmp );
            } else 
                ++bigger;
            
            //printf("cost %ld\n", std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - now ).count() );
        }
    }
    printf( "bigger %d, less %d avg %lf\n", bigger, less, (double)avg / ( less ) );
}

int main() {
    dl_move();
    return 0;
}
