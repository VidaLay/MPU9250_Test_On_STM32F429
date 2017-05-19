#include "VidaOS.h"

#if (VIDAOS_ENABLE_MEMBLOCK == 1)

/**********************************************************************************************************
** Function name        :   vMemBlockInit
** Descriptions         :   初始化存储控制块
** parameters           :   memBlock 等待初始化的存储块结构
** parameters           :   memStart 存储区的起始地址
** parameters           :   blockSize 每个块的大小
** parameters           :   blockCnt 总的块数量
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
void vMemBlockInit(vMemBlock *memBlock, uint8_t *memStart, uint32_t blockSize, uint32_t blockCnt)
{
    uint8_t *memBlockStart = (uint8_t *)memStart;                                       //u8 * 是为了特指一个指向一个字节的指针
    uint8_t *memBlockEnd = memBlockStart + blockSize * blockCnt;
    
    // 每个存储块需要来放置链接指针，所以空间至少要比tNode大
	// 即便如此，实际用户可用的空间并没有少
    if (blockSize < sizeof(vNode))
    {
        return;
    }

    vEventInit(&memBlock->event, vEventTypeMemBlock);
    memBlock->memstart = memStart;
    memBlock->blockSize = blockSize;
    memBlock->maxCount = blockCnt;
    
    vListInit(&memBlock->blockList);
    
    while (memBlockStart < memBlockEnd)
    {
        vNodeInit((vNode *) memBlockStart);
        vListAddLast(&memBlock->blockList, (vNode *) memBlockStart);
        
        memBlockStart += blockSize;
    }

}

/**********************************************************************************************************
** Function name        :   vMemBlockWait
** Descriptions         :   等待存储块
** parameters           :   memBlock 等待的存储块结构
** parameters			:   mem 存储块
** parameters           :   waitTicks 当没有存储块时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vMemBlockWait(vMemBlock *memBlock, uint8_t **mem, uint32_t waitTicks)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 首先检查是否有空闲的存储块
    if (vListCount(&memBlock->blockList) > 0)
    {
        // 如果有的话，取出一个
        *mem = (uint8_t *)vListRemoveFirst(&memBlock->blockList);
        
        // 退出临界区
        vTaskCriticalExit(status);
        
        return vErrorNoError;
    }
    else
    {
        // 否则, 将任务插入事件队列中
        vEventWait(&memBlock->event, currentTask, (void *)0, vEventTypeMemBlock, waitTicks);
        
        // 退出临界区
        vTaskCriticalExit(status);
        
        // 最后再执行一次事件调度，以便于切换到其它任务
        vTaskSched();
        
        // 当切换回来时，从vTask中取出获得的消息
        *mem = (uint8_t *)currentTask->eventMsg;
        
        // 取出等待结果
        return currentTask->waitEventResult;
    }
}

/**********************************************************************************************************
** Function name        :   vMemBlockNoWaitGet
** Descriptions         :   获取存储块，如果没有存储块，则立即退回
** parameters           :   memBlock 等待的存储块结构
** parameters			:   mem 存储块
** Returned value       :   获取结果, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vMemBlockNoWaitGet(vMemBlock *memBlock, uint8_t **mem)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 首先检查是否有空闲的存储块
    if (vListCount(&memBlock->blockList) > 0)
    {
        // 如果有的话，取出一个
        *mem = (uint8_t *)vListRemoveFirst(&memBlock->blockList);
        
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
** Function name        :   vMemBlockNotify
** Descriptions         :   通知存储块可用，唤醒等待队列中的一个任务，或者将存储块加入队列中
** parameters           :   memBlock 操作的存储块结构
** parameters			:   mem 存储块
** Returned value       :   无
***********************************************************************************************************/
void vMemBlockNotify(vMemBlock *memBlock, uint8_t *mem)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 检查是否有任务等待
    if (vEventWaitCount(&memBlock->event) > 0)
    {
        // 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
        vTask *task = vEventWakeupFirst(&memBlock->event, (void *)mem, vErrorNoError);
        
        // 如果这个任务的优先级更高，就执行调度，切换过去
        if (task->prio < currentTask->prio)
        {
            vTaskSched();
        }
    }
    else
    {
        // 如果没有任务等待的话，将存储块插入到队列中
        vListAddLast(&memBlock->blockList, (vNode *)mem);
    }
    
    // 退出临界区
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vMemBlockGetInfo
** Descriptions         :   查询存储控制块的状态信息
** parameters           :   memBlock 存储块结构
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vMemBlockGetInfo(vMemBlock *memBlock, vMemBlockInfo *info)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    info->count = vListCount(&memBlock->blockList);
    info->maxCount = memBlock->maxCount;
    info->blockSize = memBlock->blockSize;
    info->taskCount = vEventWaitCount(&memBlock->event);
    
    // 退出临界区
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vMemBlockDestroy
** Descriptions         :   销毁存储控制块
** parameters           :   memBlock 需要销毁的存储块结构
** Returned value       :   因销毁该存储控制块而唤醒的任务数量
***********************************************************************************************************/
uint32_t vMemBlockDestroy(vMemBlock *memblock)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 清空事件控制块中的任务
    uint32_t count = vEventWakeupAll(&memblock->event, (void *)0, vErrorDel);
    
    // 退出临界区
    vTaskCriticalExit(status);
    
    if (count > 0)
    {
        vTaskSched();
    }
    
    return count;
}

#endif
