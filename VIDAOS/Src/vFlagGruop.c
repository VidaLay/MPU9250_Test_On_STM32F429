#include "VidaOS.h"

#if (VIDAOS_ENABLE_FLAGGROUP == 1)

/**********************************************************************************************************
** Function name        :   vFlagGroupInit
** Descriptions         :   初始化事件标志组
** parameters           :   flagGroup 等待初始化的事件标志组
** parameters           :   flags 初始的事件标志
** Returned value       :   无
***********************************************************************************************************/
void vFlagGroupInit(vFlagGroup * flagGroup, uint32_t flags)
{
    vEventInit(&flagGroup->event, vEventTypeFlagGroup);
    flagGroup->flags = flags;
}

/**********************************************************************************************************
** Function name        :   vFlagGroupCheckAndConsume
** Descriptions         :   辅助函数。检查并消耗掉事件标志
** parameters           :   flagGroup 等待初始化的事件标志组
** parameters           :   waitType 事件标志检查类型
** parameters           :   flags 待检查事件标志存储地址和检查结果存储位置
** Returned value       :   vErrorNoError 事件匹配；vErrorResourceUnavaliable 事件未匹配
***********************************************************************************************************/
static uint32_t vFlagGroupCheckAndConsume(vFlagGroup *flagGroup, uint32_t waitType, uint32_t *flags)
{
    uint32_t reqFlag = *flags;
    uint32_t isSet = waitType & VFLAGGROUP_SET;
    uint32_t isAll = waitType & VFLAGGROUP_ALL;
    uint32_t isConsume = waitType & VFLAGGROUP_CONSUME;
    
    // 有哪些类型的标志位出现
	// flagGroup->flags & flags：计算出哪些位为1
	// ~flagGroup->flags & flags:计算出哪位为0
    uint32_t calFlag = isSet ? (flagGroup->flags & reqFlag) : (~flagGroup->flags & reqFlag);
    
    // 所有标志位出现, 或者做任意标志位出现，满足条件
    if (((isAll != 0) && (calFlag == reqFlag)) || ((isAll == 0) && (calFlag != 0)))
    {
        // 是否消耗掉标志位
        if (isConsume)   
        {
            if (isSet)
            {
                // 清除为1的标志位，变成0
                flagGroup->flags &=  ~reqFlag;
            }
            else
            {
                // 清除为0的标志位，变成1
                flagGroup->flags |=  reqFlag;
            }
        }
        
        *flags = calFlag;
        
        return vErrorNoError;
    }
    
    *flags = calFlag;                       //此时的flags是表示flagGroup->flags 与 requestFlag 相符的位
    
    return vErrorResourceUnavaliable;
}

/**********************************************************************************************************
** Function name        :   vFlagGroupWait
** Descriptions         :   等待事件标志组中特定的标志
** parameters           :   flagGroup 等待的事件标志组
** parameters           :   waitType 等待的事件类型
** parameters           :   requstFlag 请求的事件标志
** parameters           :   resultFlag 等待标志结果
** parameters           :   waitTicks 当等待的标志没有满足条件时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果, vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vFlagGroupWait(vFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag, uint32_t waitTicks)
{
    uint32_t result;
    uint32_t flags = requestFlag;
    
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    result = vFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
    
    if (result != vErrorNoError)
    {
        // 如果事件标志不满足条件，则插入到等待队列中
        currentTask->waitFlagType = waitType;
        currentTask->requestEventFlags = requestFlag;
        vEventWait(&flagGroup->event, currentTask, (void *)0, vEventTypeFlagGroup, waitTicks);
        
        // 退出临界区
        vTaskCriticalExit(status);
        
        // 再执行一次事件调度，以便于切换到其它任务
        vTaskSched();
        
        // 当切换回来时，从vTask中取出获得的消息
        *resultFlag = currentTask->requestEventFlags;
        
        // 取出等待结果
        result = currentTask->waitEventResult;       
    }
    else
    {
        *resultFlag = flags;
        
        // 退出临界区
        vTaskCriticalExit(status);
    }
    
    return result;
}

/**********************************************************************************************************
** Function name        :   vFlagGroupNoWaitGet
** Descriptions         :   获取事件标志组中特定的标志
** parameters           :   flagGroup 获取的事件标志组
** parameters           :   waitType 获取的事件类型
** parameters           :   requstFlag 请求的事件标志
** parameters           :   resultFlag 等待标志结果
** Returned value       :   获取结果, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vFlagGroupNoWaitGet(vFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag)  //该函数只是用来查询当前标志位与需求标志位相符的位
{
    uint32_t result;
    uint32_t flags = requestFlag;
    
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    result = vFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
    
    *resultFlag = flags;
    
    // 退出临界区
    vTaskCriticalExit(status);
    
    return result;
}

/**********************************************************************************************************
** Function name        :   vFlagGroupNotify
** Descriptions         :   通知事件标志组中的任务有新的标志发生
** parameters           :   flagGroup 事件标志组
** parameters           :   isSet 是否是置1事件
** parameters           :   flags 产生的事件标志
***********************************************************************************************************/
void vFlagGroupNotify(vFlagGroup *flagGroup, uint8_t isSet, uint32_t flags)
{
    vList *waitList;
    vNode *node;
    vNode *nextNode;
    uint8_t sched = 0;
    
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    if (isSet)
    {
        flagGroup->flags |= flags;          // 置1事件
    }
    else
    {
        flagGroup->flags &= ~flags;         // 清0事件
    }
    
    waitList = &flagGroup->event.waitList;
    
    // 遍历所有的等待任务, 获取满足条件的任务，加入到待移除列表中
    for (node = waitList->headNode.nextNode; node != &(waitList->headNode); node = nextNode)
    {
        uint32_t result;
        vTask *task = vNodeParent(node, vTask, linkNode);
        uint32_t flags = task->requestEventFlags;
        nextNode = node->nextNode;
        
        result = vFlagGroupCheckAndConsume(flagGroup, task->waitFlagType, &flags); 
        
        if (result == vErrorNoError)
        {
            // 唤醒任务
            task->requestEventFlags = flags;
            vEventWakeup(&flagGroup->event, task, (void *)0, vErrorNoError);
            sched = 1;
        }
    }
    
    // 如果有任务就绪，则执行一次调度
    if (sched)
    {
        vTaskSched();
    }
    
    // 退出临界区
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vFlagGroupGetInfo
** Descriptions         :   查询事件标志组的状态信息
** parameters           :   flagGroup 事件标志组
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vFlagGroupGetInfo(vFlagGroup *flagGroup, vFlagGroupInfo * info)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    info->flags = flagGroup->flags;
    info->taskCount = vEventWaitCount(&flagGroup->event);
    
    // 退出临界区
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vFlagGroupDestroy
** Descriptions         :   销毁事件标志组
** parameters           :   flagGroup 事件标志组
** Returned value       :   因销毁该存储控制块而唤醒的任务数量
***********************************************************************************************************/
uint32_t vFlagGroupDestroy(vFlagGroup *flagGroup)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 清空事件控制块中的任务
    uint32_t count = vEventWakeupAll(&flagGroup->event, (void *)0, vErrorDel);
    
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
