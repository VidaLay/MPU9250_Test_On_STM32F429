#include "VidaOS.h"
#include "stm32f4xx.h"

#define NVIC_INT_CTRL           0xE000ED04
#define NVIC_PENDSVSET          0x10000000
#define NVIC_SYSPRI2            0xE000ED22
#define NVIC_PENDSV_PRI         0x000000FF

#define MEM32(addr)         *(volatile unsigned long *)(addr)
#define MEM8(addr)          *(volatile unsigned char *)(addr)

// ��ǰ���񣺼�¼��ǰ���ĸ�������������
vTask *currentTask;

// ��һ���������е������ڽ��������л�ǰ�������úø�ֵ��Ȼ�������л������л���ж�ȡ��һ������Ϣ
vTask *nextTask;

// OS״̬
uint8_t vOS_State = vStopped;

#if (VIDAOS_ENABLE_CPUUSAGE_STAT == 1)
// ʱ�ӽ��ļ���
uint32_t tickCount;

// �������������������
uint32_t idleCount;
uint32_t idleMaxCount;

static void cpuUsageStatInit(void);
static void checkCPUUsage(void);
static void cpuUsageSyncWithSysTick(void);
#endif

/**********************************************************************************************************
** Function name        :   PendSV_Handler
** Descriptions         :   PendSV�쳣��������
** parameters           :   ��
** Returned value       :   ��
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
** Descriptions         :   �����ٽ���
** parameters           :   ��
** Returned value       :   ����֮ǰ���ٽ���״ֵ̬
***********************************************************************************************************/
uint32_t vTastCriticalEnter(void)
{
    uint16_t primask = __get_PRIMASK();
    __disable_irq();
    
    return primask;
}

/**********************************************************************************************************
** Function name        :   vTaskCriticalExit
** Descriptions         :   �˳��ٽ���,�ָ�֮ǰ���ٽ���״̬
** parameters           :   status �����ٽ���֮ǰ��CPU
** Returned value       :   �����ٽ���֮ǰ���ٽ���״ֵ̬
***********************************************************************************************************/
void vTaskCriticalExit(uint32_t status)
{
    __set_PRIMASK(status);
}

/**********************************************************************************************************
** Function name        :   vTaskSystemTickHandler
** Descriptions         :   ϵͳʱ�ӽ��Ĵ���
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vTaskSystemTickHandler(void)
{
    vNode *node;
    vNode *nextNode;
    vTask *task;
    
    // �����ٽ������Ա�������������������л��ڼ䣬������Ϊ�����жϵ���currentTask��nextTask���ܸ���
    uint32_t status = vTastCriticalEnter();
    
//    // ������������delayTicks���������0�Ļ�����1��
//    for (node = vTaskDelayedList.headNode.nextNode; node != &(vTaskDelayedList.headNode); node = nextNode)
//    {
//        vTask *task = vNodeParent(node, vTask, delayNode);
//        nextNode = node->nextNode;                                      // vListRemove���ʼ��node,��Ҫ��ʱ����һ����һ���ڵ�ĵ�ַ
//        
//        if ((task->delayTicks == 0) || (--task->delayTicks == 0))
//        {
//            // ������񻹴��ڵȴ��¼���״̬��������¼��ȴ������л���
//            if (task->waitEvent)
//            {
//                // ��ʱ����ϢΪ�գ��ȴ����Ϊ��ʱ
//                vEventRemoveTask(task, (void *)0, vErrorTimeout);
//            }
//            
//            // ���������ʱ�������Ƴ�
//            vTDlistTaskWakeup(task);
//            
//            // ������ָ�������״̬
//            vTaskSchedRdy(task);
//        }
//    }
    
    if (vTaskDelayedList.nodeCount != 0)
    {
        // �����׽ڵ������delayTicksֵ
        node = vTaskDelayedList.headNode.nextNode;
        task = vNodeParent(node, vTask, delayNode);
        
        if (task->delayTicks != 0)
        {
            task->delayTicks--;
        }
        
        // ��������,ֱ�����ֵ�һ��delayTicks��Ϊ0
        while (task->delayTicks == 0)
        {
            nextNode = node->nextNode;
            
            // ������񻹴��ڵȴ��¼���״̬��������¼��ȴ������л���
            if (task->waitEvent)
            {
                // ��ʱ����ϢΪ�գ��ȴ����Ϊ��ʱ
                vEventRemoveTask(task, (void *)0, vErrorTimeout);
            }
            
            // ���������ʱ�������Ƴ�
            vTDlistTaskWakeup(task);
            
            // �л�����һ������
            node = nextNode;
            task = vNodeParent(node, vTask, delayNode);    
        }
    }
    
    // ����µ�ǰ�����ʱ��Ƭ�Ƿ��Ѿ�����
    if (--currentTask->slice == 0)
    {
        // �����ǰ�����л�����������Ļ�����ô�л�����һ������
        // �����ǽ���ǰ����Ӷ��е�ͷ���Ƴ������뵽β��
        // ��������ִ��tTaskSched()ʱ�ͻ��ͷ��ȡ���µ�����ȡ���µ�������Ϊ��ǰ��������
        if (vListCount(&OSRdyTable[currentTask->prio]) > 0)
        {
            vListRemoveFirst(&OSRdyTable[currentTask->prio]);
            vListAddLast(&OSRdyTable[currentTask->prio],&(currentTask->linkNode));
            
            // ���ü�����
            currentTask->slice = VIDAOS_SLICE_MAX;
        }
    }
    
#if (VIDAOS_ENABLE_CPUUSAGE_STAT == 1)
    // ���ļ�������
    tickCount++;
    // ���cpuʹ����
    checkCPUUsage();
#endif
    
    // �˳��ٽ���
    vTaskCriticalExit(status);

#if (VIDAOS_ENABLE_TIMER == 1)    
    // ֪ͨ��ʱ��ģ������¼�
    vTimerModuleTickNotify();
#endif
    
#if (VIDAOS_ENABLE_HOOKS == 1)
    vHooksSysTick();
#endif
    
    // ��������п�����������ʱ���(delayTicks = 0)������һ�ε��ȡ�
    vTaskSched();
}

/**********************************************************************************************************
***********************************************************************************************************/
#if (VIDAOS_ENABLE_CPUUSAGE_STAT == 1)
/**********************************************************************************************************
** Function name        :   vTickCountInit
** Descriptions         :   ��ʼ��ʱ�ӽ��ļ���
** parameters           :   ��
** Returned value       :   ��
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
** Descriptions         :   ��ʼ��CPUͳ��
** parameters           :   ��
** Returned value       :   ��
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
** Descriptions         :   ���CPUʹ����
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
static void checkCPUUsage(void)
{
    // ����������cpuͳ��ͬ��
    if (enableCPUUsageStat == 0)
    {
        enableCPUUsageStat = 1;
        enableidleMaxCountStat = 1;
        tickCount = 0;
    }
    else
    {
        if ((tickCount == TICKS_PER_SEC) && (enableidleMaxCountStat == 1))         // ���enableidleMaxCountStat ����tickCount���������쳣
        {
            // ͳ�����1s�ڵ�������ֵ
            idleMaxCount = idleCount;
            idleCount = 0;
            enableidleMaxCountStat = 0;
            
            // ������ϣ������������������л�����������
            vTaskSchedEnable();
        }
        else if (tickCount % TICKS_PER_SEC == 0)
        {
            // ֮��ÿ��1sͳ��һ�Σ�ͬʱ����cpu������
            cpuUsage = 100 - (idleCount * 100.0 / idleMaxCount);
            idleCount = 0;
        }    
    }  
}

/**********************************************************************************************************
** Function name        :   cpuUsageSyncWithSysTick
** Descriptions         :   Ϊ���CPUʹ������ϵͳʱ�ӽ���ͬ��
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
static void cpuUsageSyncWithSysTick(void)
{
    // �ȴ���ʱ�ӽ���ͬ��
    while (enableCPUUsageStat == 0)
    {
        ;;
    }
}

/**********************************************************************************************************
** Function name        :   vCPUUsageGet
** Descriptions         :   ��õ�ǰ����CPUռ����
** parameters           :   ��
** Returned value       :   ��
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
// ���ڿ������������ṹ�Ͷ�ջ�ռ�
vTask vTaskIdle;
vTaskStack idleTaskStack[VIDAOS_IDLETASK_STACK_SIZE];

/**********************************************************************************************************
** Function name        :   idleTaskEntry
** Descriptions         :   ����������ں���
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void idleTaskEntry(void *param)
{
    // ��ֹ���ȣ���ֹ�����ڴ�������ʱ�л�������������ȥ
    vTaskSchedDisable();
    
#if (VIDAOS_ENABLE_TIMER == 1)
    // ��ʼ����ʱ������
    vTimerTaskInit();
#endif
    
    // ��ʼ��App�������
    vInitApp();
    
    // ����ϵͳʱ�ӽ���
    vSetSysTickPeriod(VIDAOS_SYSTICK_MS);
    
#if (VIDAOS_ENABLE_CPUUSAGE_STAT == 1)
    // �ȴ���ʱ��ͬ��
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
** Descriptions         :   ����ϵͳ
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vOSStart(void)
{    
#if (VIDAOS_ENABLE_FPU == 1)
    __set_CONTROL(0x40);
#else
    __set_CONTROL(0x00);    
#endif
    
    // ���ȳ�ʼ��OS�ĺ��Ĺ���
    vTaskSchedInit();
    
    // ��ʼ����ʱ����
    vTaskDelayedInit();
    
#if (VIDAOS_ENABLE_TIMER == 1)
    // ��ʼ����ʱ��ģ��
    vTimerModuleInit();
#endif
   
#if (VIDAOS_ENABLE_CPUUSAGE_STAT == 1)
    // ��ʼ��ʱ�ӽ���
    vTickCountInit();
    
    // ��ʼ��CPUͳ��
    cpuUsageStatInit();
#endif
    
    // ������������
    vTaskInit(&vTaskIdle, idleTaskEntry, (void *)0, VIDAOS_PRO_COUNT - 1, idleTaskStack, (VIDAOS_IDLETASK_STACK_SIZE * sizeof(vTaskStack)));
    
    // �Զ�����������ȼ�����������
    nextTask = vTaskHighestReady();
    
    __set_PSP(0);
    
    MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
    vOS_State = vRunning;
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

/**********************************************************************************************************
** Function name        :   vTaskSwitch
** Descriptions         :   ����һ�������л���VidaOS��Ԥ�����ú�currentTask��nextTask, Ȼ����øú������л���
**                          nextTask����
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vTaskSwitch()
{
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}
