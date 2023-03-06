

#include "log/log_sys.h"
#include "base/error_no.h"
#include "base/utils/assert.h"
#include "base/system/thread.h"
#include "log/sink_rotate_file.h"
#include <functional>

NAMESPACE_TARO_LOG_BEGIN

extern Controller& get_global();

// 异步输出回调
std::function<void( SinkSPtr, std::shared_ptr<std::string>, ELogLevel )> async_output;

TARO_DLL_EXPORT bool get_sys_enable()
{
    return get_global().enabled();
}

TARO_DLL_EXPORT void set_sys_enable( bool enable )
{
    get_global().set_enable( enable );
}

TARO_DLL_EXPORT ELogLevel get_sys_level()
{
    return get_global().level();
}

TARO_DLL_EXPORT void set_sys_level( ELogLevel level )
{
    get_global().set_level( level );
}

TARO_DLL_EXPORT void set_sys_rotate_cfg( const char* out_dir, uint32_t max_file_num, uint32_t file_size, bool async )
{
    static const char* name = "RotateFileSinkGlobal";

    ( void )get_global().remove_sink( name );
    auto sink = std::make_shared< RotateFileSink >( name );
    sink->set_async_mode( async );
    auto ret = sink->set_param( out_dir, max_file_num, file_size );
    TARO_ASSERT( ret == TARO_OK );
    get_global().add_sink( sink );
}

TARO_DLL_EXPORT void init_async_logger( std::function<void( SinkSPtr, std::shared_ptr<std::string>, ELogLevel)> const& func )
{
    async_output = func;
}

NAMESPACE_TARO_LOG_END
