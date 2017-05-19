#ifndef _VDELAY_H
#define _VDELAY_H

#include "vTask.h"

/**********************************************************************************************************
** Function name        :   vTaskDelayedInit
** Descriptions         :   初始化任务延时机制
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskDelayedInit(void);

/**********************************************************************************************************
** Function name        :   vTDlistTaskWait
** Descriptions         :   将任务加入延时队列中
** parameters           :   task    需要延时的任务
** parameters           :   ticks   延时的ticks
** Returned value       :   无
***********************************************************************************************************/
void vTDlistTaskWait(vTask *task, uint32_t ticks);

/**********************************************************************************************************
** Function name        :   vTDlistTaskWakeup
** Descriptions         :   将延时的任务从延时队列中唤醒
** parameters           :   task  需要唤醒的任务
** Returned value       :   无
***********************************************************************************************************/
void vTDlistTaskWakeup(vTask *task);;

/**********************************************************************************************************
** Function name        :   vTDlistTaskRemove
** Descriptions         :   将延时的任务从延时队列中移除
** parameters           :   task  需要移除的任务
** Returned value       :   无
***********************************************************************************************************/
void vTDlistTaskRemove(vTask *task);

/**********************************************************************************************************
** Function name        :   vTaskDelay
** Descriptions         :   使当前任务进入延时状态。
** parameters           :   delay 延时多少个ticks
** Returned value       :   无
***********************************************************************************************************/
void vTaskDelay(uint32_t delay);

/**********************************************************************************************************
** Function name        :   vTaskDelay_ms
** Descriptions         :   使当前任务进入延时状态。
** parameters           :   ms 延时的毫秒数
** Returned value       :   无
***********************************************************************************************************/
void vTaskDelay_ms(uint32_t ms);

/**********************************************************************************************************
** Function name        :   vTaskDelayHumanTime
** Descriptions         :   使当前任务进入延时状态。
** parameters           :   hour 延时的小时数
** parameters           :   minute 延时的分钟数
** parameters           :   second 延时的秒数
** parameters           :   ms 延时的毫秒数
** Returned value       :   无
***********************************************************************************************************/
void vTaskDelayHumanTime(uint32_t hour, uint32_t minute, uint32_t second, uint16_t ms);

#endif
