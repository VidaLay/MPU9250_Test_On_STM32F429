#ifndef _VIDAOS_H
#define _VIDAOS_H

// 每秒钟SysTick产生中断次数
#define TICKS_PER_SEC           (1000 / VIDAOS_SYSTICK_MS)

// 标准头文件，里面包含了常用的类型定义，如uint32_t
#include <stdint.h>

// VidaOS的内核库文件
#include "vLib.h"

// VidaOS的配置文件
#include "vConfig.h"

// 任务管理头文件
#include "vTask.h"

// 任务调度头文件
#include "vSched.h"

// 任务延时链表头文件
#include "vDelay.h"

// 事件控制头文件
#include "vEvent.h"

// 信号量头文件
#include "vSem.h"

// 邮箱头文件
#include "vMbox.h"

// 存储块头文件
#include "vMemBlock.h"

// 事件标志组头文件
#include "vFlagGroup.h"

// 互斥信号量头文件
#include "vMutex.h"

// 软定时器
#include "vTimer.h"

// Hooks扩展
#include "vHooks.h"

typedef enum _vError
{
    vErrorNoError = 0,                  // 没有错误
    vErrorTimeout,                      // 等待超时
    vErrorResourceUnavaliable,          // 资源不可用
    vErrorDel,                          // 被删除
    vErrorResourceFull,                 // 资源缓冲区满
    vErrorOwner,                        // 不匹配的所有者
}vError;

typedef enum _vOSState
{
    vStopped = 0,                       // OS停止
    vRunning,                           // OS运行
}vOSState;

// OS状态
extern uint8_t vOS_State;

// 当前任务：记录当前是哪个任务正在运行
extern vTask *currentTask;

// 下一个将即运行的任务：在进行任务切换前，先设置好该值，然后任务切换过程中会从中读取下一任务信息
extern vTask *nextTask;

// 任务就绪表
extern vList OSRdyTable[VIDAOS_PRO_COUNT];

// 任务延时链表
extern vList vTaskDelayedList;

/**********************************************************************************************************
** Function name        :   vTastCriticalEnter
** Descriptions         :   进入临界区
** parameters           :   无
** Returned value       :   进入之前的临界区状态值
***********************************************************************************************************/
uint32_t vTastCriticalEnter(void);

/**********************************************************************************************************
** Function name        :   vTaskCriticalExit
** Descriptions         :   退出临界区,恢复之前的临界区状态
** parameters           :   status 进入临界区之前的CPU
** Returned value       :   进入临界区之前的临界区状态值
***********************************************************************************************************/
void vTaskCriticalExit(uint32_t status);

/**********************************************************************************************************
** Function name        :   vOSStart
** Descriptions         :   启动系统
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vOSStart(void);

/**********************************************************************************************************
** Function name        :   vTaskSwitch
** Descriptions         :   进行一次任务切换，VidaOS会预先配置好currentTask和nextTask, 然后调用该函数，切换至
**                          nextTask运行
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSwitch(void);

/**********************************************************************************************************
** Function name        :   vTaskSystemTickHandler
** Descriptions         :   系统时钟节拍处理。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSystemTickHandler(void);

/**********************************************************************************************************
** Function name        :   vSetSysTickPeriod
** Descriptions         :   设定SysTick中断周期。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vSetSysTickPeriod(uint32_t ms);

/**********************************************************************************************************
** Function name        :   vInitApp
** Descriptions         :   初始化应用接口
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vInitApp(void);

/**********************************************************************************************************
** Function name        :   vCPUUsageGet
** Descriptions         :   获得当前任务CPU占用率
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
float vCPUUsageGet(void);

#endif
