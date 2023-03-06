
#include "base/error_no.h"

NAMESPACE_TARO_BEGIN

thread_local int32_t err_no = TARO_OK;

TARO_DLL_EXPORT void set_errno( int32_t no )
{
    err_no = no;
}

TARO_DLL_EXPORT int32_t get_errno()
{
    return err_no;
}

NAMESPACE_TARO_END
