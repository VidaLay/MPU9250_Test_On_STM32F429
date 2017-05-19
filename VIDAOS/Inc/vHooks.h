#ifndef _VHOOKS_H
#define _VHOOKS_H

#include "VidaOS.h"

/**********************************************************************************************************
** Function name        :   vHooksCPUIdle
** Descriptions         :   CPU空闲时的hooks
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vHooksCPUIdle(void);

/**********************************************************************************************************
** Function name        :   vHooksSysTick
** Descriptions         :   时钟节拍Hooks
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vHooksSysTick(void);

/**********************************************************************************************************
** Function name        :   vHooksTaskSwitch
** Descriptions         :   任务切换hooks
** parameters           :   from 从哪个任务开始切换
** parameters           :   to 切换至哪个任务
** Returned value       :   无
***********************************************************************************************************/
void vHooksTaskSwitch(vTask *from, vTask *to);

/**********************************************************************************************************
** Function name        :   vHooksTaskInit
** Descriptions         :   任务初始化的Hooks
** parameters           :   task 等待初始化的任务
** Returned value       :   无
***********************************************************************************************************/
void vHooksTaskInit(vTask *task);

#endif
