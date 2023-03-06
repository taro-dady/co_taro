
#pragma once

#include "base/base.h"

NAMESPACE_TARO_BEGIN

// 共享内存
struct TARO_DLL_EXPORT ShmMemory
{
PUBLIC: // 公共函数

	/**
	* @brief 构造函数
	*/
	ShmMemory();

	/**
	* @brief 构造函数
	*/
	ShmMemory( ShmMemory&& other );

	/**
	* @brief 析构函数
	*/
	~ShmMemory();

	/**
	* @brief 创建新的共享内存
	*
	* @param[in] name  共享内存名称
	* @param[in] bytes 共享内存大小
	* @return 见错误码
	*/
	int32_t create( const char* name, uint64_t bytes );

	/**
	* @brief 打开已有的共享内存
	*
	* @param[in] name  共享内存名称
	* @return 见错误码
	*/
	int32_t open( const char* name );

	/**
	* @brief 获取数据指针
	* 
	* @return 内存指针 
	*/
	void* get() const;

PRIVATE: // 私有函数

	TARO_NO_COPY( ShmMemory );

PRIVATE: // 私有变量

	struct Impl;
	Impl* impl_;
};

NAMESPACE_TARO_END
