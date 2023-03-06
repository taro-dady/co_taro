
#pragma once

#include "net/defs.h"
#include "co_routine/inc.h"

NAMESPACE_TARO_NET_BEGIN

// 网络事件类型
enum ENetEventType
{
    eNetEvtRead      = 1 << 0,
    eNetEvtWrite     = 1 << 1,
    eNetEvtException = 1 << 2,
    eNetEvtTimeout   = 1 << 3,
    eNetEvtInvalid   = 0,
};

// 事件处理器
class NetEventHandler
{
PUBLIC:

    /**
    * @brief 析构函数
    */
    virtual ~NetEventHandler() = default;

    /**
    * @brief 事件处理函数
    * 
    * @param[in] type 事件类型
    */
    virtual void on_event( ENetEventType type ) = 0;
};

// 网络协程事件
class CoNetEvent : PUBLIC NetEventHandler
{
PUBLIC:

    /**
    * @brief 构造函数
    */
    CoNetEvent( int32_t fd, ENetEventType type = eNetEvtInvalid );

    /**
    * @brief 析构函数
    */
    ~CoNetEvent();

    /**
    * @brief 等待事件
    */
    void wait();

    /**
    * @brief 等待指定事件
    */
    void wait( ENetEventType type, uint32_t ms = 0 );

    /**
    * @brief 获取事件类型
    */
    ENetEventType evt_type() const;

PRIVATE:

    /**
    * @brief 事件处理函数
    * 
    * @param[in] type 事件类型
    */
    virtual void on_event( ENetEventType type ) override final;

PRIVATE:

    int32_t           fd_;
    int32_t           evt_id_;
    ENetEventType     type_;
    rt::CoRoutineSPtr co_;
};

using CoNetEventSPtr = std::shared_ptr<CoNetEvent>;

NAMESPACE_TARO_NET_END
