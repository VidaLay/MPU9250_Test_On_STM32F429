#ifndef _VMUTEX_H
#define _VMUTEX_H

#include "vEvent.h"

typedef struct _vMutex
{
    // 事件控制块
    vEvent event;
    
    // 已被锁定的次数
    uint32_t lockedCount;
    
    // 拥有者
    vTask *owner;
    
    // 拥有者原始的优先级
    uint32_t ownerOriginalPrio;
}vMutex;

typedef struct _vMutexInfo
{
    // 等待的任务数量
    uint32_t taskCount;
    
    // 拥有者任务的优先级
    uint32_t ownerPrio;
    
    // 继承优先级
    uint32_t inheritedPrio;
    
    // 当前信号量的拥有者
    vTask *owner;
    
    // 锁定次数
    uint32_t lockedCount;
}vMutexInfo;

/**********************************************************************************************************
** Function name        :   vMutexInit
** Descriptions         :   初始化互斥信号量
** parameters           :   mutex 等待初始化的互斥信号量
** Returned value       :   无
***********************************************************************************************************/
void vMutexInit(vMutex *mutex);

/**********************************************************************************************************
** Function name        :   vMutexWait
** Descriptions         :   等待信号量
** parameters           :   mutex 等待的信号量
** parameters           :   waitTicks 最大等待的ticks数，为0表示无限等待
** Returned value       :   等待结果, vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vMutexWait(vMutex *mutex, uint32_t waitTicks);

/**********************************************************************************************************
** Function name        :   vMutexNoWaitGet
** Descriptions         :   获取信号量，如果已经被锁定，立即返回
** parameters           :   mutex 等待的信号量
** Returned value       :   获取结果, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vMutexNoWaitGet(vMutex *mutex);

/**********************************************************************************************************
** Function name        :   vMutexNotify
** Descriptions         :   通知互斥信号量可用
** parameters           :   mbox 操作的信号量
** parameters           :   msg 发送的消息
** parameters           :   notifyOption 发送的选项
** Returned value       :   vErrorResourceFull
***********************************************************************************************************/
uint32_t vMutexNotify(vMutex *mutex);

/**********************************************************************************************************
** Function name        :   vMutexDestroy
** Descriptions         :   销毁信号量
** parameters           :   mutex 待销毁互的斥信号量
** Returned value       :   因销毁该信号量而唤醒的任务数量
***********************************************************************************************************/
uint32_t vMutexDestroy(vMutex *mutex);

/**********************************************************************************************************
** Function name        :   vMutexGetInfo
** Descriptions         :   查询状态信息
** parameters           :   mutex 查询的互斥信号量
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vMutexGetInfo(vMutex *mutex, vMutexInfo *info);

#endif
