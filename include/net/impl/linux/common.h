
#pragma once

#include "net/defs.h"

NAMESPACE_TARO_NET_BEGIN

/**
* @brief 设置文件描述符阻塞属性
*/
extern bool set_fdnoblock( int32_t fd, bool noblock = true );

/**
* @brief 根据返回值判断是否为断线事件
*/
bool is_disconnect( int32_t ret );

NAMESPACE_TARO_NET_END
