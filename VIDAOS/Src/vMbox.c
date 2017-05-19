#include "VidaOS.h"

#if (VIDAOS_ENABLE_MBOX == 1)

/**********************************************************************************************************
** Function name        :   tMboxInit
** Descriptions         :   初始化邮箱
** parameters           :   mbox 等待初始化的邮箱
** parameters           :   msgBuffer 消息存储缓冲区
** parameters           :   maxCount 最大计数
** Returned value       :   无
***********************************************************************************************************/
void vMboxInit(vMbox *mbox, void **msgBuffer, uint32_t maxCount)
{
    vEventInit(&mbox->event, vEventTypeMbox);
    
    mbox->msgBuffer = msgBuffer;
    mbox->maxCount = maxCount;
    mbox->read = 0;
    mbox->write = 0;
    mbox->count = 0;
}

/**********************************************************************************************************
** Function name        :   vMboxWait
** Descriptions         :   等待邮箱, 获取一则消息
** parameters           :   mbox 等待的邮箱
** parameters           :   msg 消息存储缓存区
** parameters           :   waitTicks 最大等待的ticks数，为0表示无限等待
** Returned value       :   等待结果,vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vMboxWait(vMbox *mbox, void **msg, uint32_t waitTicks)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 首先检查消息计数是否大于0
    if (mbox->count > 0)
    {
        // 如果大于0的话，取出一个
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];
        
        // 同时读取索引前移，如果超出边界则回绕
        if (mbox->read >= mbox->maxCount)
        {
            mbox->read = 0;
        }
        
        // 退出临界区
        vTaskCriticalExit(status);
        
        return vErrorNoError;
    }
    else
    {
        // 否则， 将任务插入事件队列中
        vEventWait(&mbox->event, currentTask, (void *)0, vEventTypeMbox, waitTicks);
        
        // 退出临界区
        vTaskCriticalExit(status);
        
        // 最后再执行一次事件调度，以便于切换到其它任务
        vTaskSched();
        
        // 当切换回来时，从vTask中取出获得的消息
        *msg = (uint8_t *)currentTask->eventMsg;
        
        // 取出等待结果
        return currentTask->waitEventResult;
    }
}

/**********************************************************************************************************
** Function name        :   vMboxNoWaitGet
** Descriptions         :   获取一则消息，如果没有消息，则立即退回
** parameters           :   mbox 获取消息的邮箱
** parameters           :   msg 消息存储缓存区
** Returned value       :   获取结果, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vMboxNoWaitGet(vMbox *mbox, void **msg)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 首先检查消息计数是否大于0
    if (mbox->count > 0)
    {
        // 如果大于0的话，取出一个
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];
        
        // 同时读取索引前移，如果超出边界则回绕
        if (mbox->read >= mbox->maxCount)
        {
            mbox->read = 0;
        }
        
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
** Function name        :   vMboxNotify
** Descriptions         :   通知消息可用，唤醒等待队列中的一个任务，或者将消息插入到邮箱中
** parameters           :   mbox 操作的信号量
** parameters           :   msg 发送的消息
** parameters           :   notifyOption 发送的选项 vMboxStoreNormal/vMboxStoreFront/vMboxSentToAll
** Returned value       :   vErrorNoError/ vErrorResourceFull
***********************************************************************************************************/
uint32_t vMboxNotify(vMbox *mbox, void *msg, uint32_t notifyOption)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 检查是否有任务等待
    if (vEventWaitCount(&mbox->event) > 0)
    {
        if (notifyOption & vMboxSentToAll)
        {
            // 如果是vMboxSentToAll的话，则直接唤醒所有任务
            vEventWakeupAll(&mbox->event, (void *)msg, vErrorNoError);
            
            vTaskSched();
        }
        else
        {
            // 如果不是vMboxSentToAll的话，则直接唤醒位于队列首部（最先等待）的任务
            vTask *task = vEventWakeupFirst(&mbox->event, (void *)msg, vErrorNoError);
            
            // 如果这个任务的优先级更高，就执行调度，切换过去
            if (task->prio < currentTask->prio)
            {
                vTaskSched();
            }
        }
    }
    else
    {
        // 如果没有任务等待的话，将消息插入到缓冲区中
        if (mbox->count >= mbox->maxCount)
        {
            // 退出临界区
            vTaskCriticalExit(status);
            
            return vErrorResourceFull;
        }
        
        // 可以选择将消息插入到头，这样后面任务获取的消息的时候，优先获取该消息
        if (notifyOption & vMboxStoreFront)
        {
            if (mbox->read <= 0)
            {
                mbox->read = mbox->maxCount - 1;
            }
            else
            {
                --mbox->read;
            }
            
            mbox->msgBuffer[mbox->read] = msg;
        }
        else
        {
            mbox->msgBuffer[mbox->write++] = msg;
            if (mbox->write >= mbox->maxCount)
            {
                mbox->write = 0;
            }
        }
        
        // 增加消息计数
        mbox->count++;
    }
    
    // 退出临界区
    vTaskCriticalExit(status);
    
    return vErrorNoError;
}

/**********************************************************************************************************
** Function name        :   vMboxFlush
** Descriptions         :   清空邮箱中所有消息
** parameters           :   mbox 等待清空的邮箱
** Returned value       :   无
***********************************************************************************************************/
void vMboxFlush(vMbox *mbox)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 如果队列中有任务等待，说明邮箱已经是空的了，不需要再清空
    //if (vEventWaitCount(&mbox->event) == 0)
    if (mbox->count > 0)
    {
        mbox->read = 0;
        mbox->write = 0;
        mbox->count = 0;
    }
    
    // 退出临界区
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vMboxDestroy
** Descriptions         :   销毁邮箱
** parameters           :   mbox 需要销毁的邮箱
** Returned value       :   因销毁该信号量而唤醒的任务数量
***********************************************************************************************************/
uint32_t vMboxDestroy(vMbox *mbox)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 清空事件控制块中的任务
    uint32_t count = vEventWakeupAll(&mbox->event, (void *)0, vErrorDel);
    
    // 退出临界区
    vTaskCriticalExit(status);
    
    // 清空过程中可能有任务就绪，执行一次调度
    if (count > 0)
    {
        vTaskSched();
    }
    
    return count;
}

/**********************************************************************************************************
** Function name        :   vMboxGetInfo
** Descriptions         :   查询状态信息
** parameters           :   mbox 查询的邮箱
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vMboxGetInfo(vMbox *mbox, vMboxInfo *info)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    info->count = mbox->count;
    info->maxCount = mbox->maxCount;
    info->taskCount = vEventWaitCount(&mbox->event);
    
    // 退出临界区
    vTaskCriticalExit(status);
}

#endif
