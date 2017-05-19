#include "VidaOS.h"

/**********************************************************************************************************
** Function name        :   vEventInit
** Descriptions         :   初始化事件控制块
** parameters           :   event 事件控制块
** parameters           :   type 事件控制块的类型
** Returned value       :   无
***********************************************************************************************************/
void vEventInit(vEvent *event, vEventType type)
{
    event->type = type;
    vListInit(&event->waitList);
}

/**********************************************************************************************************
** Function name        :   vEventWait
** Descriptions         :   让指定在事件控制块上等待事件发生
** parameters           :   event 事件控制块
** parameters           :   task 等待事件发生的任务
** parameters           :   msg 事件消息存储的具体位置
** parameters           :   state 消息类型
** parameters           :   timeout 等待多长时间
** Returned value       :   优先级最高的且可运行的任务
***********************************************************************************************************/
//void vEventWait(vEvent *event, vTask *task, void *msg, uint32_t state, uint32_t timeout)
//{
//    // 进入临界区
//    uint32_t status = vTastCriticalEnter();
//    
//    task->state |= state;                   // 标记任务处于等待某种事件的状态
//    task->waitEvent = event;                // 设置任务等待的事件结构
//    task->eventMsg = msg;                   // 设置任务等待事件的消息存储位置
//                                            // 因有时候需要接受消息，所以需要接受区
//    task->waitEventResult = vErrorNoError;  // 清空事件的等待结果
//    
//    // 将任务从就绪队列中移除
//    vTaskSchedUnRdy(task);
//    
//    // 将任务插入到等待队列中
//    vListAddLast(&event->waitList, &task->linkNode);
//    
//    // 如果发现有设置超时，在同时插入到延时队列中
//    // 当时间到达时，由延时处理机制负责将任务从延时列表中移除，同时从事件列表中移除
//    if (timeout)
//    {
//        vTDlistTaskWait(task, timeout);
//    }
//    
//    // 退出临界区
//    vTaskCriticalExit(status);
//}

void vEventWait(vEvent *event, vTask *task, void *msg, uint32_t state, uint32_t timeout)
{
    vNode *refNode;
    vNode *nextNode;
    
    uint8_t isInsert = 0;
    
    // 任务不能是挂起,删除或者等待事件状态
    if (!((task->state & VIDAOS_TASK_STATE_SUSPEND) || (task->state & VIDAOS_TASK_STATE_DESTROYED) || (task->state & VIDAOS_TASK_WAIT_MASK)))
    {
        // 进入临界区
        uint32_t status = vTastCriticalEnter();
        
        // 如果任务已经在延时链表里,则唤醒任务
        if (task->state & VIDAOS_TASK_STATE_DELAYED)
        {
            vTDlistTaskRemove(task);
        }
        
        // 如果任务处于就绪状态,则将任务从就绪队列中移除
        if (task->state & VIDAOS_TASK_STATE_RDY)
        {
            vTaskSchedUnRdy(task);
        }
            
        for (refNode = event->waitList.headNode.nextNode; refNode != &(event->waitList.headNode); refNode = nextNode)
        {
            vTask *refTask = vNodeParent(refNode, vTask, linkNode);
            nextNode = refNode->nextNode;
            
            // 检查task的priority是否比refTask的priority小
            if (task->prio < refTask->prio)
            {
                // 如果小于, 则把任务插入refTask之前
                vListInsertBefore(&event->waitList, refNode, &(task->linkNode));
                
                isInsert = 1;
                
                break;
            }
        }
        
        if (!isInsert)
        {
            // 如果还没有插入,则直接插入队尾    
            vListAddLast(&event->waitList, &(task->linkNode));
        }
        
        task->state |= state;                   // 标记任务处于等待某种事件的状态
        task->waitEvent = event;                // 设置任务等待的事件结构
        task->eventMsg = msg;                   // 设置任务等待事件的消息存储位置
                                                // 因有时候需要接受消息，所以需要接受区
        task->waitEventResult = vErrorNoError;  // 清空事件的等待结果
        
        // 如果发现有设置超时，在同时插入到延时队列中
        // 当时间到达时，由延时处理机制负责将任务从延时列表中移除，同时从事件列表中移除
        if (timeout)
        {
            vTDlistTaskWait(task, timeout);
        }
        
        // 退出临界区
        vTaskCriticalExit(status);
    
    }
    
}

/**********************************************************************************************************
** Function name        :   vEventWakeupFirst
** Descriptions         :   从事件控制块中唤醒首个等待的任务
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   首个等待的任务，如果没有任务等待，则返回0
***********************************************************************************************************/
vTask *vEventWakeupFirst(vEvent *event, void *msg, uint32_t result)
{
    vNode *node;
    vTask *task = (vTask *)0;
    
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 取出等待队列中的第一个结点
    if ((node = vListRemoveFirst(&event->waitList)) != (vNode *)0)
    {
        // 转换为相应的任务结构
        task = vNodeParent(node, vTask, linkNode);
        
        // 设置收到的消息、结构，清除相应的等待标志位
        task->waitEvent = (vEvent *)0;
        task->eventMsg = msg;
        task->waitEventResult = result;
        task->state &= ~VIDAOS_TASK_WAIT_MASK;
        
        // 任务申请了超时等待，这里检查下，将其从延时队列中移除
        if (task->state & VIDAOS_TASK_STATE_DELAYED)
        {
            vTDlistTaskRemove(task);
        }
        
        // 将任务加入就绪队列
        vTaskSchedRdy(task);
    }
    
    // 退出临界区
    vTaskCriticalExit(status);
    
    return task; 
}

/**********************************************************************************************************
** Function name        :   vEventWakeup
** Descriptions         :   从事件控制块中唤醒指定任务
** parameters           :   event 事件控制块
** parameters           :   task 等待唤醒的任务
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   首个等待的任务，如果没有任务等待，则返回0
***********************************************************************************************************/
void vEventWakeup(vEvent *event, vTask *task, void *msg, uint32_t result)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 设置收到的消息、结构，清除相应的等待标志位
    vListRemove(&event->waitList, &task->linkNode);
    task->waitEvent = (vEvent *)0;
    task->eventMsg = msg;
    task->waitEventResult = result;
    task->state &= ~VIDAOS_TASK_WAIT_MASK;
    
    // 任务申请了超时等待，这里检查下，将其从延时队列中移除
    if (task->state & VIDAOS_TASK_STATE_DELAYED)
    {
        vTDlistTaskRemove(task);
    }
    
    // 将任务加入就绪队列
    vTaskSchedRdy(task);
    
    // 退出临界区
    vTaskCriticalExit(status); 
}

/**********************************************************************************************************
** Function name        :   vEventRemoveTask
** Descriptions         :   将任务从其等待队列中强制移除
** parameters           :   task 待移除的任务
** parameters           :   result 告知事件的等待结果
** Returned value       :   无
***********************************************************************************************************/
void vEventRemoveTask(vTask *task, void *msg, uint32_t result)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 将任务从所在的等待队列中移除
    // 注意，这里没有检查waitEvent是否为空。既然是从事件中移除，那么认为就不可能为空
    vListRemove(&task->waitEvent->waitList, &task->linkNode);
    
    // 设置收到的消息、结构，清除相应的等待标志位
    task->waitEvent = (vEvent *)0;
    task->eventMsg = msg;
    task->waitEventResult = result;
    task->state &= ~VIDAOS_TASK_WAIT_MASK;
    
    // 任务申请了超时等待，这里检查下，将其从延时队列中移除
    if (task->state & VIDAOS_TASK_STATE_DELAYED)
    {
        vTDlistTaskRemove(task);
    }
    
    // 退出临界区
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vEventWakeupAll
** Descriptions         :   清除所有等待中的任务，将事件发送给所有任务
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
uint32_t vEventWakeupAll(vEvent *event, void *msg, uint32_t result)
{
    vNode *node;
    uint32_t count;
    
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 获取等待中的任务数量
    count = vListCount(&event->waitList);
    
    // 遍历所有等待中的任务
    while ((node = vListRemoveFirst(&event->waitList)) != (vNode *)0)
    {
        // 转换为相应的任务结构
        vTask *task = vNodeParent(node, vTask, linkNode);
        
        // 设置收到的消息、结构，清除相应的等待标志位
        task->waitEvent = (vEvent *)0;
        task->eventMsg = msg;
        task->waitEventResult = result;
        task->state &= ~VIDAOS_TASK_WAIT_MASK;
        
        // 任务申请了超时等待，这里检查下，将其从延时队列中移除
        if (task->state & VIDAOS_TASK_STATE_DELAYED)
        {
            vTDlistTaskRemove(task);
        }
        
        // 将任务加入就绪队列
        vTaskSchedRdy(task);
    }
    
    // 退出临界区
    vTaskCriticalExit(status);
    
    return count;
}

/**********************************************************************************************************
** Function name        :   vEventWaitCount
** Descriptions         :   事件控制块中等待的任务数量
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
uint32_t vEventWaitCount(vEvent *event)
{
    uint32_t count = 0;
    
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    count = vListCount(&event->waitList);
    
    // 退出临界区
    vTaskCriticalExit(status);
    
    return count;
}


