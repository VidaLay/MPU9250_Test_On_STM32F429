#ifndef _VEVENT_H
#define _VEVENT_H

#include "vLib.h"
#include "vTask.h"

// Event类型
typedef enum _vEventType
{
    vEventTypeUnknown   = (1 << 16), 				// 未知类型
    vEventTypeSem   	= (2 << 16), 				// 信号量类型
    vEventTypeMbox  	= (3 << 16), 				// 邮箱类型
	vEventTypeMemBlock  = (4 << 16),				// 存储块类型
	vEventTypeFlagGroup = (5 << 16),				// 事件标志组
	vEventTypeMutex     = (6 << 16),				// 互斥信号量类型
}vEventType;

// Event控制结构
typedef struct _vEvent
{
    vEventType type;						// Event类型

    vList waitList;							// 任务等待列表
}vEvent;

/**********************************************************************************************************
** Function name        :   vEventInit
** Descriptions         :   初始化事件控制块
** parameters           :   event 事件控制块
** parameters           :   type 事件控制块的类型
** Returned value       :   无
***********************************************************************************************************/
void vEventInit(vEvent *event, vEventType type);

/**********************************************************************************************************
** Function name        :   vEventWait
** Descriptions         :   让指定在事件控制块上等待事件发生
** parameters           :   event 事件控制块
** parameters           :   task 等待事件发生的任务
** parameters           :   msg 事件消息存储的具体位置
** parameters           :   state 消息类型
** parameters           :   timeout 等待多长时间
** Returned value       :   优先级最高的且可运行的任务
***********************************************************************************************************/
void vEventWait(vEvent *event, vTask *task, void *msg, uint32_t state, uint32_t timeout);

/**********************************************************************************************************
** Function name        :   vEventWakeupFirst
** Descriptions         :   从事件控制块中唤醒首个等待的任务
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   首个等待的任务，如果没有任务等待，则返回0
***********************************************************************************************************/
vTask *vEventWakeupFirst(vEvent *event, void *msg, uint32_t result);

/**********************************************************************************************************
** Function name        :   vEventWakeup
** Descriptions         :   从事件控制块中唤醒指定任务
** parameters           :   event 事件控制块
** parameters           :   task 等待唤醒的任务
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   首个等待的任务，如果没有任务等待，则返回0
***********************************************************************************************************/
void vEventWakeup(vEvent *event, vTask *task, void *msg, uint32_t result);

/**********************************************************************************************************
** Function name        :   vEventRemoveTask
** Descriptions         :   将任务从其等待队列中强制移除
** parameters           :   task 待移除的任务
** parameters           :   result 告知事件的等待结果
** Returned value       :   无
***********************************************************************************************************/
void vEventRemoveTask(vTask *task, void *msg, uint32_t result);

/**********************************************************************************************************
** Function name        :   vEventWakeupAll
** Descriptions         :   清除所有等待中的任务，将事件发送给所有任务
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
uint32_t vEventWakeupAll(vEvent *event, void *msg, uint32_t result);

/**********************************************************************************************************
** Function name        :   vEventWaitCount
** Descriptions         :   事件控制块中等待的任务数量
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
uint32_t vEventWaitCount(vEvent *event);

#endif
