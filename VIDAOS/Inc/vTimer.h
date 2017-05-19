#ifndef _VTIMER_H
#define _VTIMER_H

#include "vEvent.h"

typedef enum _vTimerState
{
    vTimerCreated,          // 定时器已经创建
    vTimerStarted,          // 定时器已经启动
    vTimerFuncRunning,          // 定时器正在执行回调函数
    vTimerStopped,          // 定时器已经停止
    vTimerDestroyed,        // 定时器已经销毁
}vTimerState;

typedef struct _vTimer
{
    // 链表结点
    vNode linkNode;
    
    // 初次启动延后的ticks数
    uint32_t startDelayTicks;
    
    // 周期定时时的周期tick数
    uint32_t durationTicks;
    
    // 当前定时递减计数值
    uint32_t delayTicks;
    
    // 定时回调函数
    void (*timerFunc)(void *arg);
    
    // 传递给回调函数的参数
    void *arg;
    
    // 定时器配置参数
    uint32_t config;
    
    // 定时器状态
    vTimerState state;
}vTimer;

typedef struct _vTimerInfo
{
    // 初次启动延后的ticks数
    uint32_t startDelayTicks;
    
    // 周期定时时的周期tick数
    uint32_t durationTicks;
    
    // 定时回调函数
    void (*timerFunc)(void *arg);
    
    // 传递给回调函数的参数
    void *arg;
    
    // 定时器配置参数
    uint32_t config;
    
    // 定时器状态
    vTimerState state;
}vTimerInfo;

#define TIMER_CONFIG_TYPE_HARD          (1 << 0)
#define TIMER_CONFIG_TYPE_SOFT          (0 << 0)

/**********************************************************************************************************
** Function name        :   vTimerInit
** Descriptions         :   初始化定时器
** parameters           :   timer 等待初始化的定时器
** parameters           :   delayTicks 定时器初始启动的延时ticks数。
** parameters           :   durationTicks 给周期性定时器用的周期tick数，一次性定时器无效
** parameters           :   timerFunc 定时器回调函数
** parameters           :   arg 传递给定时器回调函数的参数
** parameters           :   timerFunc 定时器回调函数
** parameters           :   config 定时器的初始配置
** Returned value       :   无
***********************************************************************************************************/
void vTimerInit(vTimer *timer, uint32_t delayTicks, uint32_t durationTicks, void (*timerFunc)(void *arg), void *arg, uint32_t config);

/**********************************************************************************************************
** Function name        :   vTimerStart
** Descriptions         :   启动定时器
** parameters           :   timer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void vTimerStart(vTimer *timer);

/**********************************************************************************************************
** Function name        :   vTimerStop
** Descriptions         :   终止定时器
** parameters           :   timer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void vTimerStop(vTimer *timer);

/**********************************************************************************************************
** Function name        :   vTimerModuleTickNotify
** Descriptions         :   通知定时模块，系统节拍tick增加
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTimerModuleTickNotify(void);

/**********************************************************************************************************
** Function name        :   vTimerModuleInit
** Descriptions         :   定时器模块初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTimerModuleInit(void);

/**********************************************************************************************************
** Function name        :   vTimerTaskInit
** Descriptions         :   初始化软定时器任务
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTimerTaskInit(void);

/**********************************************************************************************************
** Function name        :   vTimerDestroy
** Descriptions         :   销毁定时器
** parameters           :   timer 销毁的定时器
** Returned value       :   无
***********************************************************************************************************/
void vTimerDestroy (vTimer * timer);

/**********************************************************************************************************
** Function name        :   vTimerGetInfo
** Descriptions         :   查询状态信息
** parameters           :   timer 查询的定时器
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vTimerGetInfo (vTimer * timer, vTimerInfo * info);


#endif
