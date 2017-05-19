#include "VidaOS.h"
#include <string.h>

/**********************************************************************************************************
** Function name        :   vTaskInit
** Descriptions         :   初始化任务结构
** parameters           :   task        要初始化的任务结构
** parameters           :   entry       任务的入口函数
** parameters           :   param       传递给任务的运行参数
** Returned value       :   无
***********************************************************************************************************/
void vTaskInit(vTask * task, void (*entry)(void *), void *param, uint8_t prio, vTaskStack *stackBase, uint32_t size)
{
    uint32_t *stackTop;
    task->stackBase = stackBase;
    task->stackSize = size;
    memset(stackBase, 0, size);

    stackTop = stackBase + size / sizeof(vTaskStack);
    
    // 为了简化代码，VidaOS无论是在启动时切换至第一个任务，还是在运行过程中在不同间任务切换
    // 所执行的操作都是先保存当前任务的运行环境参数（CPU寄存器值）的堆栈中(如果已经运行运行起来的话)，然后再
    // 取出从下一个任务的堆栈中取出之前的运行环境参数，然后恢复到CPU寄存器
    // 对于切换至之前从没有运行过的任务，我们为它配置一个“虚假的”保存现场，然后使用该现场恢复。

    // 注意以下两点：
    // 1、不需要用到的寄存器，直接填了寄存器号，方便在IDE调试时查看效果；
    // 2、顺序不能变，要结合PendSV_Handler以及CPU对异常的处理流程来理解
    
#if (VIDAOS_ENABLE_FPU == 1)
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
    *(--stackTop) = (unsigned long)(0xFFFFFFFF);
#endif

    *(--stackTop) = (unsigned long)(1<<24);             // XPSR, 设置了Thumb模式，恢复到Thumb状态而非ARM状态运行
    *(--stackTop) = (unsigned long)entry;               // 程序的入口地址
    *(--stackTop) = (unsigned long)0x14;                // R14(LR), 任务不会通过return xxx结束自己，所以未用
    *(--stackTop) = (unsigned long)0x12;                // R12, 未用
    *(--stackTop) = (unsigned long)0x3;                 // R3, 未用
    *(--stackTop) = (unsigned long)0x2;                 // R2, 未用
    *(--stackTop) = (unsigned long)0x1;                 // R1, 未用
    *(--stackTop) = (unsigned long)param;               // R0 = param, 传给任务的入口函数
    *(--stackTop) = (unsigned long)0x11;                // R11, 未用
    *(--stackTop) = (unsigned long)0x10;                // R10, 未用
    *(--stackTop) = (unsigned long)0x9;                 // R9, 未用
    *(--stackTop) = (unsigned long)0x8;                 // R8, 未用
    *(--stackTop) = (unsigned long)0x7;                 // R7, 未用
    *(--stackTop) = (unsigned long)0x6;                 // R6, 未用
    *(--stackTop) = (unsigned long)0x5;                 // R5, 未用
    *(--stackTop) = (unsigned long)0x4;                 // R4, 未用
    
    task->stack = stackTop;                             // 保存最终的值
    task->slice = VIDAOS_SLICE_MAX;                     // 初始化任务的时间片计数
    task->prio = prio;                                  // 设置任务的优先级
    task->state = VIDAOS_TASK_STATE_RDY;                // 设置任务为就绪状态
    task->suspendCount = 0;                             // 初始挂起次数为0
    task->delayTicks = 0;
    
    vNodeInit(&(task->linkNode));                       // 初始化链接结点
    vNodeInit(&(task->delayNode));                      // 初始化延时结点
  
    task->clean = (void(*)(void *))0;                   // 设置清理函数
    task->cleanParam = (void *)0;                       // 设置传递给清理函数的参数
    task->requestDeleteFlag = 0;                        // 请求删除标记

    task->waitEvent = (vEvent *)0;                      // 没有等待事件
    task->eventMsg = (void *)0;                         // 没有等待事件
    task->waitEventResult = vErrorNoError;              // 没有等待事件错误
    
    task->waitFlagType = 0;
    task->requestEventFlags = 0;
    
    task->stackFreeMin = (uint32_t)stackTop - (uint32_t)stackBase;
    vTaskSchedRdy(task);                                // 将任务插入就绪队列
    
#if (VIDAOS_ENABLE_HOOKS == 1)
    vHooksTaskInit(task);
#endif
}

/**********************************************************************************************************
** Function name        :   vTaskSuspend
** Descriptions         :   挂起指定的任务
** parameters           :   task        待挂起的任务
** Returned value       :   无
***********************************************************************************************************/
void vTaskSuspend(vTask *task)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 只允许对已经进入挂起状态或者就绪状态的任务挂起
    if ((task->state & VIDAOS_TASK_STATE_RDY) || (task->state & VIDAOS_TASK_STATE_SUSPEND))
    {
        // 增加挂起计数，仅当该任务被执行第一次挂起操作时，才考虑是否
        // 要执行任务切换操作
        if (++task->suspendCount == 1)
        {
            // 挂起方式很简单，就是将其从就绪队列中移除，这样调度器就不会发现他
            // 也就没法切换到该任务运行
            vTaskSchedUnRdy(task);
            
            // 设置挂起标志
            task->state |= VIDAOS_TASK_STATE_SUSPEND;
            
            // 当然，这个任务可能是自己，那么就切换到其它任务
            if (task == currentTask)
            {
                vTaskSched();
            }
        }
    }
    
    // 退出临界区
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vTaskWakeup
** Descriptions         :   唤醒被挂起的任务
** parameters           :   task        待唤醒的任务
** Returned value       :   无
***********************************************************************************************************/
void vTaskWakeup(vTask *task)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 检查任务是否处于挂起状态
    if (task->state & VIDAOS_TASK_STATE_SUSPEND)
    {
        // 递减挂起计数，如果为0了，则清除挂起标志，同时设置进入就绪状态
        if (--task->suspendCount == 0)
        {
            // 清除挂起标志
            task->state &= ~VIDAOS_TASK_STATE_SUSPEND;
            
            // 同时将任务放回就绪队列中
            vTaskSchedRdy(task);
            
            // 如果唤醒任务的优先级更高，就执行调度，切换过去
            if (task->prio < currentTask->prio)
            {
                vTaskSched();
            }
        }
    }
    
    // 退出临界区
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vTaskSetCleanCallFunc
** Descriptions         :   设置任务被删除时调用的清理函数
** parameters           :   task  待设置的任务
** parameters           :   clean  清理函数入口地址
** parameters           :   param  传递给清理函数的参数
** Returned value       :   无
***********************************************************************************************************/
void vTaskSetCleanCallFunc(vTask *task, void (*clean)(void * param), void *param)
{
    task->clean = clean;
    task->cleanParam = param;
}

/**********************************************************************************************************
** Function name        :   vTaskForceDelete
** Descriptions         :   强制删除指定的任务
** parameters           :   task  需要删除的任务
** Returned value       :   无
***********************************************************************************************************/
void vTaskForceDelete(vTask *task)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 如果任务处于延时状态，则从延时队列中删除
    if (task->state & VIDAOS_TASK_STATE_DELAYED)
    {
        vTDlistTaskRemove(task);
    }
    // 如果任务不处于挂起状态，那么就是就绪态，从就绪表中删除
    else if (!(task->state & VIDAOS_TASK_STATE_SUSPEND))
    {
        vTaskSchedRemove(task);
    }
    
    // 删除时，如果有设置清理函数，则调用清理函数
    if (task->clean)
    {
        task->clean(task->cleanParam);
    }
    
    // 如果删除的是自己，那么需要切换至另一个任务，所以执行一次任务调度
    if (currentTask == task)
    {
        vTaskSched();
    }
    
    // 退出临界区
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vTaskRequestDelete
** Descriptions         :   请求删除某个任务，由任务自己决定是否删除自己
** parameters           :   task  需要删除的任务
** Returned value       :   无
***********************************************************************************************************/
void vTaskRequestDelete(vTask *task)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 设置清除删除标记
    task->requestDeleteFlag = 1;
    
    // 退出临界区
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vTaskIsRequestedDelete
** Descriptions         :   是否已经被请求删除自己
** parameters           :   无
** Returned value       :   非0表示请求删除，0表示无请求
***********************************************************************************************************/
uint8_t vTaskIsRequestedDelete(void)
{
    uint8_t deleteFlag;
    
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 获取请求删除标记
    deleteFlag = currentTask->requestDeleteFlag;
    
    // 退出临界区
    vTaskCriticalExit(status);
    
    return deleteFlag;
}

/**********************************************************************************************************
** Function name        :   vTaskDeleteSelf
** Descriptions         :   删除自己
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskDeleteSelf(void)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 任务在调用该函数时，必须是处于就绪状态，不可能处于延时或挂起等其它状态
    // 所以，只需要从就绪队列中移除即可
    vTaskSchedRemove(currentTask);
    
    // 删除时，如果有设置清理函数，则调用清理函数
    if (currentTask->clean)
    {
        currentTask->clean(currentTask->cleanParam);
    }
    
    // 接下来，肯定是切换到其它任务去运行
    vTaskSched();
    
    // 退出临界区
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vTaskGetInfo
** Descriptions         :   获取任务相关信息
** parameters           :   task 需要查询的任务
** parameters           :   info 任务信息存储结构
** Returned value       :   无
***********************************************************************************************************/
void vTaskGetInfo(vTask *task, vTaskInfo *info)
{
    uint32_t *stackEnd;
    vNode *refNode;
    vNode *nextNode;
    uint32_t preTicks = 0;
    
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    if (task->state & VIDAOS_TASK_STATE_DELAYED)
    {
        for (refNode = vTaskDelayedList.headNode.nextNode; refNode != &(task->delayNode); refNode = nextNode)
        {
            vTask *refTask = vNodeParent(refNode, vTask, delayNode);
            nextNode = refNode->nextNode;
            
            preTicks += refTask->delayTicks;
        }
    }
    
    info->delayTicks = preTicks + task->delayTicks;     // 延时信息
    info->prio = task->prio;                            // 任务优先级
    info->state = task->state;                          // 任务状态
    info->slice = task->slice;                          // 剩余时间片
    info->suspendCount = task->suspendCount;            // 被挂起的次数
    info->stackSize = task->stackSize;
    
    // 计算堆栈使用量
    info->stackFreeRT = 0;
    info->stackFreeMin = task->stackFreeMin;
    stackEnd = task->stackBase;
    
    while ((*stackEnd++ == 0) && (stackEnd <= task->stackBase + task->stackSize / sizeof(vTaskStack)))
    {
        info->stackFreeRT++;
    }
    
    // 转换成字节数
    info->stackFreeRT *= sizeof(vTaskStack);
    
    // 统计最小剩余堆栈量
    if (info->stackFreeMin > info->stackFreeRT)
    {
        info->stackFreeMin = info->stackFreeRT;
        task->stackFreeMin = info->stackFreeMin;
    }
    
    // 退出临界区
    vTaskCriticalExit(status);
}

