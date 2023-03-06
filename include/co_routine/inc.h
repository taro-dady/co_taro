
#pragma once

#include "co_routine/co_timer.h"
#include "co_routine/co_routine.h"
#include "co_routine/co_wrapper.h"
#include "co_routine/co_channel.h"
#include "co_routine/co_condition.h"
#include "co_routine/co_async.h"

NAMESPACE_TARO_RT_BEGIN

/**
 * @brief 等待调度
*/
extern TARO_DLL_EXPORT void co_wait( uint32_t ms = 0 );

/**
 * @brief 调度协程
*/
extern TARO_DLL_EXPORT void co_loop();

/**
 * @brief 调度协程直至没有运行中的协程
*/
extern TARO_DLL_EXPORT void co_loop_until_no_tasks();

NAMESPACE_TARO_RT_END
