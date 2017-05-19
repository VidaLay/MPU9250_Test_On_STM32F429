#include "VidaOS.h"

#if (VIDAOS_ENABLE_TIMER == 1)

// "硬"定时器列表
static vList vTimerHardList;

// "软"定时器列表
static vList vTimerSoftList;

// 用于访问软定时器列表的信号量
static vSem vTimerProtectSem;

// 用于软定时器任务与中断同步的计数信号量
static vSem vTimerTickSem;

/**********************************************************************************************************
** Function name        :   vTimerInit
** Descriptions         :   初始化定时器
** parameters           :   timer 等待初始化的定时器
** parameters           :   delayTicks 定时器初始启动的延时ticks数。
** parameters           :   durationTicks 给周期性定时器用的周期tick数，一次性定时器无效
** parameters           :   timerFunc 定时器回调函数
** parameters           :   arg 传递给定时器回调函数的参数
** parameters           :   timerFunc 定时器回调函数
** parameters           :   config 定时器的初始配置
** Returned value       :   无
***********************************************************************************************************/
void vTimerInit(vTimer *timer, uint32_t delayTicks, uint32_t durationTicks, void (*timerFunc)(void *arg), void *arg, uint32_t config)
{
    vNodeInit(&timer->linkNode);
    timer->startDelayTicks = delayTicks;
    timer->durationTicks = durationTicks;
    timer->timerFunc = timerFunc;
    timer->arg = arg;
    timer->config = config;
    
    // 如果初始启动延时为0，则使用周期值
    if (delayTicks == 0)
    {
        timer->delayTicks = durationTicks;
    }
    else
    {
        timer->delayTicks = timer->startDelayTicks;
    }
    
    timer->state = vTimerCreated;
}

/**********************************************************************************************************
** Function name        :   vTimerStart
** Descriptions         :   启动定时器
** parameters           :   timer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void vTimerStart(vTimer *timer)
{
    switch (timer->state)
    {
    case vTimerCreated:
    case vTimerStopped:
        timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;
        timer->state = vTimerStarted;
        
        // 根据定时器类型加入相应的定时器列表
        if (timer->config & TIMER_CONFIG_TYPE_HARD)
        {
            // 硬定时器，在时钟节拍中断中处理，所以使用临界区防护
            uint32_t status = vTastCriticalEnter();             // 时钟节拍中断处理函数会访问vTimerHardList,所以要加临界区保护
            
            // 加入硬定时器列表
            vListAddLast(&vTimerHardList, &timer->linkNode);
            
            // 退出临界区
            vTaskCriticalExit(status);
        }
        else
        {
            // 软定时器，先获取信号量。以处理此时定时器任务此时同时在访问软定时器列表导致的冲突问题
            vSemWait(&vTimerProtectSem, 0);                     // 设置只有任务会访问到vTimerSoftList,中断不允许访问,所以只需要用信号量来保护,因为代码足够简单,所以不需要使用互斥信号量
            
            // 加入软定时器列表
            vListAddLast(&vTimerSoftList, &timer->linkNode);
            vSemNotify(&vTimerProtectSem);
        }
        break;
    default:
        break; 
    }   
}

/**********************************************************************************************************
** Function name        :   vTimerStop
** Descriptions         :   终止定时器
** parameters           :   timer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void vTimerStop(vTimer *timer)
{
    switch (timer->state)
    {
    case vTimerStarted:
    case vTimerFuncRunning:
        // 如果已经启动，判断定时器类型，然后从相应的延时列表中移除
        if (timer->config & TIMER_CONFIG_TYPE_HARD)
        {
            // 硬定时器，在时钟节拍中断中处理，所以使用临界区防护
            uint32_t status = vTastCriticalEnter();             // 时钟节拍中断处理函数会访问vTimerHardList,所以要加临界区保护
            
            // 从硬定时器列表中移除
            vListRemove(&vTimerHardList, &timer->linkNode);
            
            // 退出临界区
            vTaskCriticalExit(status);
        }
        else
        {
            vSemWait(&vTimerProtectSem, 0);                     // 设置只有任务会访问到vTimerSoftList,中断不允许访问,所以只需要用信号量来保护,因为代码足够简单,所以不需要使用互斥信号量
            
            // 从软定时器列表中移除
            vListRemove(&vTimerSoftList, &timer->linkNode);
            vSemNotify(&vTimerProtectSem);
        }
                
        timer->state = vTimerStopped;
        break;
    default:
        break; 
    } 
}

/**********************************************************************************************************
** Function name        :   vTimerDestroy
** Descriptions         :   销毁定时器
** parameters           :   timer 销毁的定时器
** Returned value       :   无
***********************************************************************************************************/
void vTimerDestroy (vTimer * timer)
{
    vTimerStop(timer);
    timer->state = vTimerDestroyed;
}

/**********************************************************************************************************
** Function name        :   vTimerGetInfo
** Descriptions         :   查询状态信息
** parameters           :   timer 查询的定时器
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vTimerGetInfo (vTimer * timer, vTimerInfo * info)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();

    info->startDelayTicks = timer->startDelayTicks;
    info->durationTicks = timer->durationTicks;
    info->timerFunc = timer->timerFunc;
    info->arg = timer->arg;
    info->config = timer->config;
    info->state = timer->state;

    // 退出临界区
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vTimerListHandler
** Descriptions         :   遍历指定的定时器列表，调用各个定时器处理函数
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void vTimerListHandler(vList *timerList)
{
    vNode *node;
    vNode *nextNode;
    
    // 检查所有任务的delayTicks数，如果不0的话，减1
    for (node = timerList->headNode.nextNode; node != &(timerList->headNode); node = nextNode)
    {
        vTimer *timer = vNodeParent(node, vTimer, linkNode);
        nextNode = node->nextNode;
        
        // 如果延时已到，则调用定时器处理函数
        if ((timer->delayTicks == 0) || (--timer->delayTicks == 0))
        {
            // 切换为正在运行状态
            timer->state = vTimerFuncRunning;
            
            // 调用定时器处理函数
            timer->timerFunc(timer->arg);
            
            // 切换为已经启动状态
            timer->state = vTimerStarted;
            
            if (timer->durationTicks > 0)
            {
                // 如果是周期性的，则重复延时计数值
                timer->delayTicks = timer->durationTicks;        
            }
            else
            {
                // 否则，是一次性计数器，中止定时器
                vListRemove(timerList, &timer->linkNode);
                timer->state = vTimerStopped;
            }
        }
    }
}

/**********************************************************************************************************
** Function name        :   vTimerModuleTickNotify
** Descriptions         :   通知定时模块，系统节拍tick增加
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTimerModuleTickNotify(void)
{
    // 进入临界区
    uint32_t status = vTastCriticalEnter();
    
    // 处理硬定时器列表
    vTimerListHandler(&vTimerHardList);
    
    // 退出临界区
    vTaskCriticalExit(status);
    
    // 通知软定时器节拍变化
    vSemNotify(&vTimerTickSem);
}

/**********************************************************************************************************
***********************************************************************************************************/
static vTask vTimerTask;
static vTaskStack vTimerTaskStack[VIDAOS_TIMERTASK_STACK_SIZE];

/**********************************************************************************************************
** Function name        :   vTimerSoftTaskEntry
** Descriptions         :   处理软定时器列表的任务
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void vTimerSoftTaskEntry(void *param)
{
    for (;;)
    {
        // 等待系统节拍发送的中断事件信号
        vSemWait(&vTimerTickSem, 0);
        
        // 获取软定时器列表的访问权限
        vSemWait(&vTimerProtectSem, 0);
        
        // 处理软定时器列表
        vTimerListHandler(&vTimerSoftList);
        
        // 释放定时器列表访问权限
        vSemNotify(&vTimerProtectSem);
    }
}

/**********************************************************************************************************
** Function name        :   vTimerModuleInit
** Descriptions         :   定时器模块初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTimerModuleInit(void)
{
    vListInit(&vTimerHardList);
    vListInit(&vTimerSoftList);
    vSemInit(&vTimerProtectSem, 1, 1);
    vSemInit(&vTimerTickSem, 0, 0);
}

/**********************************************************************************************************
** Function name        :   vTimerTaskInit
** Descriptions         :   初始化软定时器任务
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTimerTaskInit(void)
{
#if VIDAOS_TIMERTASK_PRIO >= (VIDAOS_PRO_COUNT - 1)
    #error "The priority of timer task must be greater than (VIDAOS_PRO_COUNT - 1)"
#endif
    
    vTaskInit(&vTimerTask, vTimerSoftTaskEntry, (void *)0, VIDAOS_TIMERTASK_PRIO, vTimerTaskStack, (VIDAOS_TIMERTASK_STACK_SIZE * sizeof(vTaskStack)));
}

#endif
