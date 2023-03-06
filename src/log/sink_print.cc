
#include "log/sink_print.h"
#include <stdio.h>

#if defined _WIN32 || defined _WIN64
#include <Winsock2.h>
#include <windows.h>
    #define    BLUE         FOREGROUND_BLUE
    #define    RED          FOREGROUND_RED
    #define    WHITE        7
    #define    LIGHT_GREEN  FOREGROUND_GREEN
#else
    #define    BLUE         "\033[34m"
    #define    RED          "\033[31m"
    #define    WHITE        "\033[37m"
    #define    LIGHT_GREEN  "\033[1;32m"
    #define    ENDATTRI     "\033[0m"
#endif

NAMESPACE_TARO_LOG_BEGIN

PrintSink::PrintSink( const char* name )
    : Sink( name )
{

}

const char* PrintSink::type() const
{
    return "PrintSink";
}

void PrintSink::write( const char* log_msg, ELogLevel const& level )
{
#if defined _WIN32 || _WIN64
    unsigned short forecolor = WHITE;
#else
    const char* forecolor = WHITE;
#endif
    forecolor = level >= eLogLevelTrace ? LIGHT_GREEN : ( forecolor );
    forecolor = level >  eLogLevelWarn  ? RED         : ( forecolor );

#if defined _WIN32 || _WIN64
    unsigned short backgroundcolor = 0;
    HANDLE con = GetStdHandle( STD_OUTPUT_HANDLE );
    SetConsoleTextAttribute( con, forecolor | backgroundcolor );
    printf( "%s", log_msg );
    SetConsoleTextAttribute( con, WHITE | backgroundcolor );
#else
    printf("%s%s\033[0m", forecolor, log_msg );
#endif
}

NAMESPACE_TARO_LOG_END
