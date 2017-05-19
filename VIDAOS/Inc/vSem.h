#ifndef _VSEM_H
#define _VSEM_H

#include "vEvent.h"

// 信号量类型
typedef struct _vSem
{
    // 事件控制块
	// 该结构被特意放到起始处，以实现tSem同时是一个tEvent的目的
    vEvent event;
    
    // 当前的计数
    uint32_t count;
    
    // 最大计数
    uint32_t maxCount;
}vSem;

// 信号量的信息类型
typedef struct _vSemInfo
{
    // 当前信号量的计数
    uint32_t count;
    
    // 信号量允许的最大计数
    uint32_t maxCount;
    
    // 当前等待的任务计数
    uint32_t taskCount;
}vSemInfo;

/**********************************************************************************************************
** Function name        :   vSemInit
** Descriptions         :   初始化信号量
** parameters           :   startCount 初始的计数
** parameters           :   maxCount 最大计数，如果为0，则不限数量
** Returned value       :   无
***********************************************************************************************************/
void vSemInit(vSem *sem, uint32_t startCount, uint32_t maxCount);

/**********************************************************************************************************
** Function name        :   vSemWait
** Descriptions         :   等待信号量
** parameters           :   sem 等待的信号量
** parameters           :   waitTicks 当信号量计数为0时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果, vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vSemWait(vSem *sem, uint32_t waitTicks);

/**********************************************************************************************************
** Function name        :   vSemNoWaitGet
** Descriptions         :   获取信号量，如果信号量计数不可用，则立即退回
** parameters           :   sem 等待的信号量
** Returned value       :   获取结果, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vSemNoWaitGet(vSem *sem);

/**********************************************************************************************************
** Function name        :   vSemNotify
** Descriptions         :   通知信号量可用，唤醒等待队列中的一个任务，或者将计数+1
** parameters           :   sem 操作的信号量
** Returned value       :   无
***********************************************************************************************************/
void vSemNotify(vSem *sem);

/**********************************************************************************************************
** Function name        :   vSemGetInfo
** Descriptions         :   查询信号量的状态信息
** parameters           :   sem 查询的信号量
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vSemGetInfo(vSem *sem, vSemInfo *info);

/**********************************************************************************************************
** Function name        :   vSemDestroy
** Descriptions         :   销毁信号量
** parameters           :   sem 需要销毁的信号量
** Returned value       :   因销毁该信号量而唤醒的任务数量
***********************************************************************************************************/
uint32_t vSemDestroy(vSem *sem);

#endif
