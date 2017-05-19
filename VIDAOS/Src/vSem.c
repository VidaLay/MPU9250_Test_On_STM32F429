#include "VidaOS.h"

#if (VIDAOS_ENABLE_SEM == 1)

/**********************************************************************************************************
** Function name        :   vSemInit
** Descriptions         :   初始化信号量
** parameters           :   startCount 初始的计数
** parameters           :   maxCount 最大计数，如果为0，则不限数量
** Returned value       :   无
***********************************************************************************************************/
void vSemInit(vSem *sem, uint32_t startCount, uint32_t maxCount)
{
    vEventInit(&sem->event, vEventTypeSem);
    
    sem->maxCount = maxCount;
    
    if (maxCount == 0)
    {
        sem->count = startCount;                // maxCount == 0 表示没有限制
    }
    else
    {
        sem->count = (startCount > maxCount) ? maxCount : startCount;
    }
}

/**********************************************************************************************************
** Function name        :   vSemWait
** Descriptions         :   等待信号量
** parameters           :   sem 等待的信号量
** parameters           :   waitTicks 当信号量计数为0时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果, vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vSemWait(vSem *sem, uint32_t waitTicks)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 首先检查信号量计数是否大于0
    if (sem->count > 0)
    {
        // 如果大于0的话，消耗掉一个，然后正常退出
        --sem->count;
        
        // 退出临界区
        vTaskCriticalExit(status);
        
        return vErrorNoError;
    }
    else
    {
        // 否则，将任务插入事件队列中
        vEventWait(&sem->event, currentTask, (void *)0, vEventTypeSem, waitTicks);
        
        // 退出临界区
        vTaskCriticalExit(status);
        
        // 最后再执行一次事件调度，以便于切换到其它任务
        vTaskSched();
        
        // 当由于等待超时或者计数可用时，执行会返回到这里，然后取出等待结果
        return currentTask->waitEventResult;
    }
}

/**********************************************************************************************************
** Function name        :   vSemNoWaitGet
** Descriptions         :   获取信号量，如果信号量计数不可用，则立即退回
** parameters           :   sem 等待的信号量
** Returned value       :   获取结果, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vSemNoWaitGet(vSem *sem)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 首先检查信号量计数是否大于0
    if (sem->count > 0)
    {
        // 如果大于0的话，消耗掉一个，然后正常退出
        --sem->count;
        
        // 退出临界区
        vTaskCriticalExit(status);
        
        return vErrorNoError;
    }
    else
    {
        // 否则，退出临界区返回资源不可用
        vTaskCriticalExit(status);
        
        return vErrorResourceUnavaliable;
    }
}

/**********************************************************************************************************
** Function name        :   vSemNotify
** Descriptions         :   通知信号量可用，唤醒等待队列中的一个任务，或者将计数+1
** parameters           :   sem 操作的信号量
** Returned value       :   无
***********************************************************************************************************/
void vSemNotify(vSem *sem)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 检查是否有任务等待
    if (vEventWaitCount(&sem->event) > 0)
    {
        // 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
        vTask *task = vEventWakeupFirst(&sem->event, (void *)0, vErrorNoError);
        
        // 如果这个任务的优先级更高，就执行调度，切换过去
        if (task->prio < currentTask->prio)
        {
            vTaskSched();
        }
    }
    else
    {
        // 如果没有任务等待的话，增加计数
        if (sem->count != 0xFFFFFFFF)
        {
            ++sem->count;
        }
        
        // 如果这个计数超过了最大允许的计数，则递减
        if ((sem->maxCount != 0) && (sem->count > sem->maxCount))
        {
            sem->count = sem->maxCount;
        }
    }
    
    // 退出临界区
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vSemGetInfo
** Descriptions         :   查询信号量的状态信息
** parameters           :   sem 查询的信号量
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vSemGetInfo(vSem *sem, vSemInfo *info)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 拷贝需要的信息
    info->count = sem->count;
    info->maxCount = sem->maxCount;
    info->taskCount = vEventWaitCount(&sem->event);
    
    // 退出临界区
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vSemDestroy
** Descriptions         :   销毁信号量
** parameters           :   sem 需要销毁的信号量
** Returned value       :   因销毁该信号量而唤醒的任务数量
***********************************************************************************************************/
uint32_t vSemDestroy(vSem *sem)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 清空事件控制块中的任务
    uint32_t count = vEventWakeupAll(&sem->event, (void *)0, vErrorDel);
    sem->count = 0;
    
    // 退出临界区
    vTaskCriticalExit(status);
    
    // 清空过程中可能有任务就绪，执行一次调度
    if (count > 0)
    {
        vTaskSched();
    }
    
    return count;
}

#endif
