#include "VidaOS.h"
#include "stm32f4xx.h"

#define NVIC_INT_CTRL           0xE000ED04
#define NVIC_PENDSVSET          0x10000000
#define NVIC_SYSPRI2            0xE000ED22
#define NVIC_PENDSV_PRI         0x000000FF

#define MEM32(addr)         *(volatile unsigned long *)(addr)
#define MEM8(addr)          *(volatile unsigned char *)(addr)

// 当前任务：记录当前是哪个任务正在运行
vTask *currentTask;

// 下一个将即运行的任务：在进行任务切换前，先设置好该值，然后任务切换过程中会从中读取下一任务信息
vTask *nextTask;

// OS状态
uint8_t vOS_State = vStopped;

#if (VIDAOS_ENABLE_CPUUSAGE_STAT == 1)
// 时钟节拍计数
uint32_t tickCount;

// 空闲任务计数与最大计数
uint32_t idleCount;
uint32_t idleMaxCount;

static void cpuUsageStatInit(void);
static void checkCPUUsage(void);
static void cpuUsageSyncWithSysTick(void);
#endif

/**********************************************************************************************************
** Function name        :   PendSV_Handler
** Descriptions         :   PendSV异常处理函数。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
__asm void PendSV_Handler(void)
{
    IMPORT currentTask
    IMPORT nextTask
    
    MRS R0, PSP
    CBZ R0, PendSVHander_nosave
    STMDB R0!, {R4-R11}
    LDR R1, = currentTask
    LDR R1, [R1]
    STR R0, [R1]
PendSVHander_nosave
    
    LDR R0, =currentTask
    LDR R1, =nextTask
    LDR R2, [R1]
    STR R2, [R0]
    
    LDR R0, [R2]
    LDMIA R0!, {R4-R11}
    
    MSR PSP, R0
    
#if (VIDAOS_ENABLE_FPU == 1)
    MOV LR, #0xFFFFFFED
#else
    MOV LR, #0xFFFFFFFD
#endif
    
    BX LR
}

/**********************************************************************************************************
** Function name        :   vTastCriticalEnter
** Descriptions         :   进入临界区
** parameters           :   无
** Returned value       :   进入之前的临界区状态值
***********************************************************************************************************/
uint32_t vTastCriticalEnter(void)
{
    uint16_t primask = __get_PRIMASK();
    __disable_irq();
    
    return primask;
}

/**********************************************************************************************************
** Function name        :   vTaskCriticalExit
** Descriptions         :   退出临界区,恢复之前的临界区状态
** parameters           :   status 进入临界区之前的CPU
** Returned value       :   进入临界区之前的临界区状态值
***********************************************************************************************************/
void vTaskCriticalExit(uint32_t status)
{
    __set_PRIMASK(status);
}

/**********************************************************************************************************
** Function name        :   vTaskSystemTickHandler
** Descriptions         :   系统时钟节拍处理。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSystemTickHandler(void)
{
    vNode *node;
    vNode *nextNode;
    vTask *task;
    
    // 进入临界区，以保护在整个任务调度与切换期间，不会因为发生中断导致currentTask和nextTask可能更改
    uint32_t status = vTastCriticalEnter();
    
//    // 检查所有任务的delayTicks数，如果不0的话，减1。
//    for (node = vTaskDelayedList.headNode.nextNode; node != &(vTaskDelayedList.headNode); node = nextNode)
//    {
//        vTask *task = vNodeParent(node, vTask, delayNode);
//        nextNode = node->nextNode;                                      // vListRemove会初始化node,需要临时保存一下下一个节点的地址
//        
//        if ((task->delayTicks == 0) || (--task->delayTicks == 0))
//        {
//            // 如果任务还处于等待事件的状态，则将其从事件等待队列中唤醒
//            if (task->waitEvent)
//            {
//                // 此时，消息为空，等待结果为超时
//                vEventRemoveTask(task, (void *)0, vErrorTimeout);
//            }
//            
//            // 将任务从延时队列中移除
//            vTDlistTaskWakeup(task);
//            
//            // 将任务恢复到就绪状态
//            vTaskSchedRdy(task);
//        }
//    }
    
    if (vTaskDelayedList.nodeCount != 0)
    {
        // 检验首节点任务的delayTicks值
        node = vTaskDelayedList.headNode.nextNode;
        task = vNodeParent(node, vTask, delayNode);
        
        if (task->delayTicks != 0)
        {
            task->delayTicks--;
        }
        
        // 唤醒任务,直到出现第一个delayTicks不为0
        while (task->delayTicks == 0)
        {
            nextNode = node->nextNode;
            
            // 如果任务还处于等待事件的状态，则将其从事件等待队列中唤醒
            if (task->waitEvent)
            {
                // 此时，消息为空，等待结果为超时
                vEventRemoveTask(task, (void *)0, vErrorTimeout);
            }
            
            // 将任务从延时队列中移除
            vTDlistTaskWakeup(task);
            
            // 切换至下一个任务
            node = nextNode;
            task = vNodeParent(node, vTask, delayNode);    
        }
    }
    
    // 检查下当前任务的时间片是否已经到了
    if (--currentTask->slice == 0)
    {
        // 如果当前任务中还有其它任务的话，那么切换到下一个任务
        // 方法是将当前任务从队列的头部移除，插入到尾部
        // 这样后面执行tTaskSched()时就会从头部取出新的任务取出新的任务作为当前任务运行
        if (vListCount(&OSRdyTable[currentTask->prio]) > 0)
        {
            vListRemoveFirst(&OSRdyTable[currentTask->prio]);
            vListAddLast(&OSRdyTable[currentTask->prio],&(currentTask->linkNode));
            
            // 重置计数器
            currentTask->slice = VIDAOS_SLICE_MAX;
        }
    }
    
#if (VIDAOS_ENABLE_CPUUSAGE_STAT == 1)
    // 节拍计数增加
    tickCount++;
    // 检查cpu使用率
    checkCPUUsage();
#endif
    
    // 退出临界区
    vTaskCriticalExit(status);

#if (VIDAOS_ENABLE_TIMER == 1)    
    // 通知定时器模块节拍事件
    vTimerModuleTickNotify();
#endif
    
#if (VIDAOS_ENABLE_HOOKS == 1)
    vHooksSysTick();
#endif
    
    // 这个过程中可能有任务延时完毕(delayTicks = 0)，进行一次调度。
    vTaskSched();
}

/**********************************************************************************************************
***********************************************************************************************************/
#if (VIDAOS_ENABLE_CPUUSAGE_STAT == 1)
/**********************************************************************************************************
** Function name        :   vTickCountInit
** Descriptions         :   初始化时钟节拍计数
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTickCountInit(void)
{
    tickCount = 0;
}

static float cpuUsage;
static uint32_t enableCPUUsageStat;
static uint32_t enableidleMaxCountStat;

/**********************************************************************************************************
** Function name        :   cpuUsageStatInit
** Descriptions         :   初始化CPU统计
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void cpuUsageStatInit(void)
{
    idleCount = 0;
    idleMaxCount = 0;
    enableCPUUsageStat = 0;
    enableidleMaxCountStat = 0;
    cpuUsage = 0.0f;
}

/**********************************************************************************************************
** Function name        :   checkCPUUsage
** Descriptions         :   检查CPU使用率
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void checkCPUUsage(void)
{
    // 与空闲任务的cpu统计同步
    if (enableCPUUsageStat == 0)
    {
        enableCPUUsageStat = 1;
        enableidleMaxCountStat = 1;
        tickCount = 0;
    }
    else
    {
        if ((tickCount == TICKS_PER_SEC) && (enableidleMaxCountStat == 1))         // 检查enableidleMaxCountStat 避免tickCount溢出后产生异常
        {
            // 统计最初1s内的最大计数值
            idleMaxCount = idleCount;
            idleCount = 0;
            enableidleMaxCountStat = 0;
            
            // 计数完毕，开启调度器，允许切换到其它任务
            vTaskSchedEnable();
        }
        else if (tickCount % TICKS_PER_SEC == 0)
        {
            // 之后每隔1s统计一次，同时计算cpu利用率
            cpuUsage = 100 - (idleCount * 100.0 / idleMaxCount);
            idleCount = 0;
        }    
    }  
}

/**********************************************************************************************************
** Function name        :   cpuUsageSyncWithSysTick
** Descriptions         :   为检查CPU使用率与系统时钟节拍同步
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void cpuUsageSyncWithSysTick(void)
{
    // 等待与时钟节拍同步
    while (enableCPUUsageStat == 0)
    {
        ;;
    }
}

/**********************************************************************************************************
** Function name        :   vCPUUsageGet
** Descriptions         :   获得当前任务CPU占用率
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
float vCPUUsageGet(void)
{
    float usage = 0;
    
    uint32_t status = vTastCriticalEnter();
    
    usage = cpuUsage;
    
    vTaskCriticalExit(status);
    
    return usage;
}
#endif

/**********************************************************************************************************
***********************************************************************************************************/
// 用于空闲任务的任务结构和堆栈空间
vTask vTaskIdle;
vTaskStack idleTaskStack[VIDAOS_IDLETASK_STACK_SIZE];

/**********************************************************************************************************
** Function name        :   idleTaskEntry
** Descriptions         :   空闲任务入口函数
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void idleTaskEntry(void *param)
{
    // 禁止调度，防止后面在创建任务时切换到其它任务中去
    vTaskSchedDisable();
    
#if (VIDAOS_ENABLE_TIMER == 1)
    // 初始化定时器任务
    vTimerTaskInit();
#endif
    
    // 初始化App相关配置
    vInitApp();
    
    // 启动系统时钟节拍
    vSetSysTickPeriod(VIDAOS_SYSTICK_MS);
    
#if (VIDAOS_ENABLE_CPUUSAGE_STAT == 1)
    // 等待与时钟同步
    cpuUsageSyncWithSysTick();
#endif
    for (;;)
    {
        uint32_t status = vTastCriticalEnter();

#if (VIDAOS_ENABLE_CPUUSAGE_STAT == 1)
        idleCount++;
#else
        vTaskSchedEnable();
#endif
        vTaskCriticalExit(status);
        
#if (VIDAOS_ENABLE_HOOKS == 1)
        vHooksCPUIdle();
#endif
    }
}

/**********************************************************************************************************
** Function name        :   vOSStart
** Descriptions         :   启动系统
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vOSStart(void)
{    
#if (VIDAOS_ENABLE_FPU == 1)
    __set_CONTROL(0x40);
#else
    __set_CONTROL(0x00);    
#endif
    
    // 优先初始化OS的核心功能
    vTaskSchedInit();
    
    // 初始化延时队列
    vTaskDelayedInit();
    
#if (VIDAOS_ENABLE_TIMER == 1)
    // 初始化定时器模块
    vTimerModuleInit();
#endif
   
#if (VIDAOS_ENABLE_CPUUSAGE_STAT == 1)
    // 初始化时钟节拍
    vTickCountInit();
    
    // 初始化CPU统计
    cpuUsageStatInit();
#endif
    
    // 创建空闲任务
    vTaskInit(&vTaskIdle, idleTaskEntry, (void *)0, VIDAOS_PRO_COUNT - 1, idleTaskStack, (VIDAOS_IDLETASK_STACK_SIZE * sizeof(vTaskStack)));
    
    // 自动查找最高优先级的任务运行
    nextTask = vTaskHighestReady();
    
    __set_PSP(0);
    
    MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
    vOS_State = vRunning;
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

/**********************************************************************************************************
** Function name        :   vTaskSwitch
** Descriptions         :   进行一次任务切换，VidaOS会预先配置好currentTask和nextTask, 然后调用该函数，切换至
**                          nextTask运行
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vTaskSwitch()
{
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}
