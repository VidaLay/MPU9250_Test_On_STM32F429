#include "VidaOS.h"

#if (VIDAOS_ENABLE_MUTEX == 1)

/**********************************************************************************************************
** Function name        :   vMutexInit
** Descriptions         :   初始化互斥信号量
** parameters           :   mutex 等待初始化的互斥信号量
** Returned value       :   无
***********************************************************************************************************/
void vMutexInit(vMutex *mutex)
{
    vEventInit(&mutex->event, vEventTypeMutex);
    
    mutex->lockedCount = 0;
    mutex->owner = (vTask *)0;
    mutex->ownerOriginalPrio = VIDAOS_PRO_COUNT;
}

/**********************************************************************************************************
** Function name        :   vMutexWait
** Descriptions         :   等待信号量
** parameters           :   mutex 等待的信号量
** parameters           :   waitTicks 最大等待的ticks数，为0表示无限等待
** Returned value       :   等待结果, vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vMutexWait(vMutex *mutex, uint32_t waitTicks)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    if (mutex->lockedCount <= 0)
    {
        // 如果没有锁定，则使用当前任务锁定
        mutex->owner = currentTask;
        mutex->ownerOriginalPrio = currentTask->prio;
        mutex->lockedCount ++;
        
        // 退出临界区
        vTaskCriticalExit(status);
        
        return vErrorNoError;
    }
    else
    {
        // 信号量已经被锁定
        if (currentTask == mutex->owner)
        {
            // 如果是信号量的拥有者再次wait，简单增加计数
            mutex->lockedCount++;
            
            // 退出临界区
            vTaskCriticalExit(status);
            
            return vErrorNoError;
        }
        else
        {
            // 如果是信号量拥有者之外的任务wait，则要检查下是否需要使用优先级继承方式处理
            if (currentTask->prio < mutex->owner->prio)
            {
                // 如果当前任务的优先级比拥有者优先级更高，则使用优先级继承提升原拥有者的优先
                if (mutex->owner->state & VIDAOS_TASK_STATE_RDY)
                {
                    // 任务处于就绪状态时，更改任务在就绪表中的位置
                    vTaskSchedUnRdy(mutex->owner);
                    mutex->owner->prio = currentTask->prio;
                    vTaskSchedRdy(mutex->owner);
                }
                else
                {
                    // 其它状态，只需要修改优先级
                    mutex->owner->prio = currentTask->prio;
                }
            }
            
            // 当前任务进入等待队列中
            vEventWait(&mutex->event, currentTask, (void *)0, vEventTypeMutex, waitTicks);
            
            // 退出临界区
            vTaskCriticalExit(status);
            
            // 执行调度， 切换至其它任务
            vTaskSched();
            
            // 当切换回来时，取出等待结果
            return currentTask->waitEventResult;
        }
    }
}

/**********************************************************************************************************
** Function name        :   vMutexNoWaitGet
** Descriptions         :   获取信号量，如果已经被锁定，立即返回
** parameters           :   mutex 等待的信号量
** Returned value       :   获取结果, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vMutexNoWaitGet(vMutex *mutex)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    if (mutex->lockedCount <= 0)
    {
        // 如果没有锁定，则使用当前任务锁定
        mutex->owner = currentTask;
        mutex->ownerOriginalPrio = currentTask->prio;
        mutex->lockedCount ++;
        
        // 退出临界区
        vTaskCriticalExit(status);
        
        return vErrorNoError;
    }
    else
    {
        // 信号量已经被锁定
        if (currentTask == mutex->owner)
        {
            // 如果是信号量的拥有者再次wait，简单增加计数
            mutex->lockedCount++;
            
            // 退出临界区
            vTaskCriticalExit(status);
            
            return vErrorNoError;
        }
        else
        {
            // 退出临界区
            vTaskCriticalExit(status);
            
            return vErrorResourceUnavaliable;
        }
    }
}

/**********************************************************************************************************
** Function name        :   vMutexNotify
** Descriptions         :   通知互斥信号量可用
** parameters           :   mbox 操作的信号量
** parameters           :   msg 发送的消息
** parameters           :   notifyOption 发送的选项
** Returned value       :   vErrorResourceFull
***********************************************************************************************************/
uint32_t vMutexNotify(vMutex *mutex)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    if (mutex->lockedCount <= 0)
    {
        // 锁定计数为0，信号量未被锁定，直接退出
        // 退出临界区
        vTaskCriticalExit(status);
        
        return vErrorNoError;
    }

    if (mutex->owner != currentTask)         // 当占有者不是当前任务时报错,释放者必须是占有者
    {
        // 不是拥有者释放，认为是非法
        // 退出临界区
        vTaskCriticalExit(status);
        
        return vErrorOwner;
    }

    if (--mutex->lockedCount > 0)
    {
        // 减1后计数仍不为0, 直接退出，不需要唤醒等待的任务
        // 退出临界区
        vTaskCriticalExit(status);
        
        return vErrorNoError;
    }

    // 是否有发生优先级继承
    if (mutex->ownerOriginalPrio != mutex->owner->prio)
    {
        // 有发生优先级继承，恢复拥有者的优先级
        if (mutex->owner->state & VIDAOS_TASK_STATE_RDY)
        {
            // 任务处于就绪状态时，更改任务在就绪表中的位置
            vTaskSchedUnRdy(mutex->owner);
            mutex->owner->prio = mutex->ownerOriginalPrio;
            vTaskSchedRdy(mutex->owner);
        }
        else
        {
            // 其它状态，只需要修改优先级
            mutex->owner->prio = mutex->ownerOriginalPrio;
        }
    }

    // 检查是否有任务等待
    if (vEventWaitCount(&mutex->event) > 0)
    {
        // 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
        vTask *task = vEventWakeupFirst(&mutex->event, (void *)0, vErrorNoError);
        
        mutex->owner = task;
        mutex->ownerOriginalPrio = task->prio;
        mutex->lockedCount++;
        
        // 如果这个任务的优先级更高，就执行调度，切换过去
        if (task->prio < currentTask->prio)
        {
            vTaskSched();
        }
        
    }
    
    // 退出临界区
    vTaskCriticalExit(status);
    
    return vErrorNoError;
}

/**********************************************************************************************************
** Function name        :   vMutexDestroy
** Descriptions         :   销毁信号量
** parameters           :   mutex 待销毁互的斥信号量
** Returned value       :   因销毁该信号量而唤醒的任务数量
***********************************************************************************************************/
uint32_t vMutexDestroy(vMutex *mutex)
{
    uint32_t count;
    
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 信号量是否已经被锁定，未锁定时没有任务等待，不必处理
    if (mutex->lockedCount >0)
    {
        // 是否有发生优先级继承? 如果发生, 需要恢复拥有者的原优先级
        if (mutex->ownerOriginalPrio != mutex->owner->prio)
        {
            // 有发生优先级继承，恢复拥有者的优先级
            if (mutex->owner->state & VIDAOS_TASK_STATE_RDY)
            {
                // 任务处于就绪状态时，更改任务在就绪表中的位置
                vTaskSchedUnRdy(mutex->owner);
                mutex->owner->prio = mutex->ownerOriginalPrio;
                vTaskSchedRdy(mutex->owner);
            }
            else
            {
                // 其它状态，只需要修改优先级
                mutex->owner->prio = mutex->ownerOriginalPrio;
            }
        }
        
        // 然后，清空事件控制块中的任务
        count = vEventWakeupAll(&mutex->event, (void *)0, vErrorDel);
        
        // 清空过程中如果有任务就绪，执行一次调度
        if (count > 0)
        {
            vTaskSched();
        }
    }
    
    // 退出临界区
    vTaskCriticalExit(status);
    
    return count;
}

/**********************************************************************************************************
** Function name        :   vMutexGetInfo
** Descriptions         :   查询状态信息
** parameters           :   mutex 查询的互斥信号量
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vMutexGetInfo(vMutex *mutex, vMutexInfo *info)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    info->taskCount = vEventWaitCount(&mutex->event);
    info->ownerPrio = mutex->ownerOriginalPrio;
    
    if (mutex->owner != (vTask *)0)
    {
        info->inheritedPrio = mutex->owner->prio;
    }
    else
    {
        info->inheritedPrio = VIDAOS_PRO_COUNT;
    }
    
    info->owner = mutex->owner;
    info->lockedCount = mutex->lockedCount;
    
    // 退出临界区
    vTaskCriticalExit(status);
}

#endif
