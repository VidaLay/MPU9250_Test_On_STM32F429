#include "VidaOS.h"

// 任务就绪表
vList OSRdyTable[VIDAOS_PRO_COUNT];

// 任务优先级的标记位置结构
vBitmap taskPrioBitmap;

// 调度锁计数器
uint8_t schedLockCount;

/**********************************************************************************************************
** Function name        :   vTaskHighestReady
** Descriptions         :   获取当前最高优先级且可运行的任务
** parameters           :   无
** Returned value       :   优先级最高的且可运行的任务
***********************************************************************************************************/
vTask *vTaskHighestReady(void)
{
    uint32_t highestPrio = vBitmapGetFirstSet(&taskPrioBitmap);
    vNode *node = vListFirst(&OSRdyTable[highestPrio]);
    return vNodeParent(node, vTask, linkNode);
}

/**********************************************************************************************************
** Function name        :   vTaskSchedInit
** Descriptions         :   初始化调度器
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSchedInit(void)
{
    int i;
    schedLockCount = 0;
    vBitmapInit(&taskPrioBitmap);
    for (i = 0; i < VIDAOS_PRO_COUNT; i++)
    {
        vListInit(&OSRdyTable[i]);
    }
}

/**********************************************************************************************************
** Function name        :   vTaskSchedDisable
** Descriptions         :   禁止任务调度
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSchedDisable(void)
{
    uint32_t status = vTastCriticalEnter();
    
    if (schedLockCount < 255)
    {
        schedLockCount++;
    }
    
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vTaskSchedEnable
** Descriptions         :   允许任务调度
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSchedEnable(void)
{
    uint32_t status = vTastCriticalEnter();
    
    if (schedLockCount > 0)
    {
        if (--schedLockCount == 0)
        {
            vTaskSched();
        }
    }
    
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vTaskSchedRdy
** Descriptions         :   将任务设置为就绪状态
** parameters           :   task    等待设置为就绪状态的任务
** Returned value       :   无
***********************************************************************************************************/
void vTaskSchedRdy(vTask *task)
{
    vListAddLast(&(OSRdyTable[task->prio]),&(task->linkNode));
    vBitmapSet(&taskPrioBitmap, task->prio);
    task->state |= VIDAOS_TASK_STATE_RDY;
}

/************************************************************************************************************
** Descriptions         :   vTaskSchedUnRdy
** Descriptions         :   将任务移除就绪状态
** parameters           :   task    等待移除就绪状态的任务
** Returned value       :   无
***********************************************************************************************************/
void vTaskSchedUnRdy(vTask *task)
{
    if (task->state & VIDAOS_TASK_STATE_RDY)
    {
        vListRemove(&(OSRdyTable[task->prio]),&(task->linkNode));
        
        // 队列中可能存在多个任务。只有当没有任务时，才清除位图标记
        if (vListCount(&OSRdyTable[task->prio]) == 0)
        {
            vBitmapClear(&taskPrioBitmap, task->prio);
        } 
        
        task->state &= ~VIDAOS_TASK_STATE_RDY;
    }
}

/**********************************************************************************************************
** Function name        :   vTaskSchedRemove
** Descriptions         :   将任务从就绪列表中移除
** parameters           :   task    等待移除的任务
** Returned value       :   
***********************************************************************************************************/
void vTaskSchedRemove(vTask *task)
{
    vListRemove(&(OSRdyTable[task->prio]),&(task->linkNode));
    if (vListCount(&OSRdyTable[task->prio]) == 0)
    {
        vBitmapClear(&taskPrioBitmap, task->prio);
    } 
}

/**********************************************************************************************************
** Function name        :   vTaskSched
** Descriptions         :   任务调度接口。VidaOS通过它来选择下一个具体的任务，然后切换至该任务运行。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSched(void)
{
    vTask *tempTask;
    
    // 进入临界区，以保护在整个任务调度与切换期间，不会因为发生中断导致currentTask和nextTask可能更改
    uint32_t status = vTastCriticalEnter();
    
    // 如何调度器已经被上锁，则不进行调度，直接退bm
    if (schedLockCount > 0)
    {
        vTaskCriticalExit(status);
        return;
    } 
    
    // 找到优先级最高的任务。这个任务的优先级可能比当前低低
    // 但是当前任务是因为延时才需要切换，所以必须切换过去，也就是说不能再通过判断优先级来决定是否切换
    // 只要判断不是当前任务，就立即切换过去
    tempTask = vTaskHighestReady();
    if (tempTask != currentTask)
    {
        nextTask = tempTask;
        
#if (VIDAOS_ENABLE_HOOKS == 1)
        vHooksTaskSwitch(currentTask, nextTask);
#endif
        
        vTaskSwitch();
    }
    
    // 退出临界区
    vTaskCriticalExit(status);
}
