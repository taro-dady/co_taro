
#pragma once

#include "base/base.h"

#define TARO_OK               0
#define TARO_ERR_FAILED      -1    // 执行失败
#define TARO_ERR_INVALID_ARG -2    // 无效参数
#define TARO_ERR_INVALID_RES -3    // 无效资源
#define TARO_ERR_DISCONNECT  -4    // 断开连接
#define TARO_ERR_MULTI_OP    -5    // 重复的操作
#define TARO_ERR_NOT_SUPPORT -6    // 不支持的操作
#define TARO_ERR_TIMEOUT     -7    // 超时
#define TARO_ERR_STATE       -8    // 状态错误 
#define TARO_ERR_FILE_OP     -9    // 文件操作错误
#define TARO_ERR_CONTINUE    -10   // 继续等待
#define TARO_ERR_FORMAT      -11   // 格式错误

NAMESPACE_TARO_BEGIN

/**
 * @brief 设置错误号
 * 
 * @param[in] no 错误号
*/
TARO_DLL_EXPORT void set_errno( int32_t no );

/**
 * @brief 获取错误号
 * 
 * @return 错误号
*/
TARO_DLL_EXPORT int32_t get_errno();

NAMESPACE_TARO_END
