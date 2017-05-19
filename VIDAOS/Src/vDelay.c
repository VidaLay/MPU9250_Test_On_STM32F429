#include "VidaOS.h"

// 任务延时链表
vList vTaskDelayedList;

/**********************************************************************************************************
** Function name        :   vTaskDelayedInit
** Descriptions         :   初始化任务延时机制
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskDelayedInit(void)
{
    vListInit(&vTaskDelayedList);
}

/**********************************************************************************************************
** Function name        :   vTDlistTaskWait
** Descriptions         :   将任务加入延时队列中
** parameters           :   task    需要延时的任务
** parameters           :   ticks   延时的ticks
** Returned value       :   无
***********************************************************************************************************/
//void vTDlistTaskWait(vTask *task, uint32_t ticks)
//{
//    task->delayTicks = ticks;

//    vListAddLast(&vTaskDelayedList, &(task->delayNode));
//    
//    task->state |= VIDAOS_TASK_STATE_DELAYED;
//}

void vTDlistTaskWait(vTask *task, uint32_t ticks)
{
    vNode *refNode;
    vNode *nextNode;
    
    uint8_t isInsert = 0;
    
    // 任务不能是挂起或者删除状态
    if (!((task->state & VIDAOS_TASK_STATE_SUSPEND) || (task->state & VIDAOS_TASK_STATE_DESTROYED)))
    {
        // 如果任务已经在延时链表里,则唤醒任务,再重新插入到延时链表
        if (task->state & VIDAOS_TASK_STATE_DELAYED)
        {
            vTDlistTaskRemove(task);
        }
        
        // 如果任务处于就绪状态,则将任务从就绪队列中移除
        if (task->state & VIDAOS_TASK_STATE_RDY)
        {
            vTaskSchedUnRdy(task);
        }
            
        for (refNode = vTaskDelayedList.headNode.nextNode; refNode != &(vTaskDelayedList.headNode); refNode = nextNode)
        {
            vTask *refTask = vNodeParent(refNode, vTask, delayNode);
            nextNode = refNode->nextNode;
       
            // 检查ticks是否比refTask的delayTicks小
            if (ticks < refTask->delayTicks)
            {
                // 如果ticks小于, 则把任务插入refTask之前
                vListInsertBefore(&vTaskDelayedList, refNode, &(task->delayNode));
                
                // 更新参考任务的delayTicks值
                refTask->delayTicks -= ticks;
                
                // 保存已插入任务的delayTicks值
                task->delayTicks = ticks;
                
                isInsert = 1;
                
                break;
            }
            else
            {
                // 否则, 更新ticks值
                ticks -= refTask->delayTicks;
            }
        }
        
        if (!isInsert)
        {
            // 如果还没有插入
            task->delayTicks = ticks;
        
            vListAddLast(&vTaskDelayedList, &(task->delayNode));
        }
        
        task->state |= VIDAOS_TASK_STATE_DELAYED;
    }
}

/**********************************************************************************************************
** Function name        :   vTDlistTaskWakeup
** Descriptions         :   将延时的任务从延时队列中唤醒
** parameters           :   task  需要唤醒的任务
** Returned value       :   无
***********************************************************************************************************/
//void vTDlistTaskWakeup(vTask *task)
//{
//    vListRemove(&vTaskDelayedList, &(task->delayNode));
//    task->state &= ~VIDAOS_TASK_STATE_DELAYED;
//}

void vTDlistTaskWakeup(vTask *task)
{
    if (task->delayTicks != 0)
    {
        vTask *nextTask = vNodeParent(task->delayNode.nextNode, vTask, delayNode);
        nextTask->delayTicks += task->delayTicks;
    }
    
    vListRemove(&vTaskDelayedList, &(task->delayNode));
    task->state &= ~VIDAOS_TASK_STATE_DELAYED;
    task->delayTicks = 0;
    
    // 将任务加入就绪队列
    vTaskSchedRdy(task);
}

/**********************************************************************************************************
** Function name        :   vTDlistTaskRemove
** Descriptions         :   将延时的任务从延时队列中移除
** parameters           :   task  需要移除的任务
** Returned value       :   无
***********************************************************************************************************/
void vTDlistTaskRemove(vTask *task)
{
    if (task->delayTicks != 0)
    {
        vTask *nextTask = vNodeParent(task->delayNode.nextNode, vTask, delayNode);
        nextTask->delayTicks += task->delayTicks;
    }
    
    vListRemove(&vTaskDelayedList, &(task->delayNode));
    task->state &= ~VIDAOS_TASK_STATE_DELAYED;
    task->delayTicks = 0;
}

/**********************************************************************************************************
** Function name        :   vTaskDelay
** Descriptions         :   使当前任务进入延时状态。
** parameters           :   delay 延时多少个ticks
** Returned value       :   无
***********************************************************************************************************/
void vTaskDelay(uint32_t delay)
{
    if (delay > 0)
    {
        // 进入临界区，以保护在整个任务调度与切换期间，不会因为发生中断导致currentTask和nextTask可能更改
        uint32_t status = vTastCriticalEnter();
        
//        // 将任务从就绪表中移除
//        vTaskSchedUnRdy(currentTask);
        
        // 设置延时值，插入延时队列,并将任务从就绪表中移除
        vTDlistTaskWait(currentTask, delay);
        
        // 退出临界区
        vTaskCriticalExit(status);
        
        // 然后进行任务切换，切换至另一个任务，或者空闲任务
        // delayTikcs会在时钟中断中自动减1.当减至0时，会切换回来继续运行。    
        vTaskSched();
    }
}

/**********************************************************************************************************
** Function name        :   vTaskDelay_ms
** Descriptions         :   使当前任务进入延时状态。
** parameters           :   ms 延时的毫秒数
** Returned value       :   无
***********************************************************************************************************/
void vTaskDelay_ms(uint32_t ms)
{
    uint32_t delay = ms / (1000 / TICKS_PER_SEC);
    
    vTaskDelay(delay);
}

/**********************************************************************************************************
** Function name        :   vTaskDelayHumanTime
** Descriptions         :   使当前任务进入延时状态。
** parameters           :   hour 延时的小时数
** parameters           :   minute 延时的分钟数
** parameters           :   second 延时的秒数
** parameters           :   ms 延时的毫秒数
** Returned value       :   无
***********************************************************************************************************/
void vTaskDelayHumanTime(uint32_t hour, uint32_t minute, uint32_t second, uint16_t ms)
{
    uint32_t delay = ((hour*60*60*1000) + (minute*60*1000) + (second*1000) + ms) / (1000 / TICKS_PER_SEC);
    
    vTaskDelay(delay);
}


