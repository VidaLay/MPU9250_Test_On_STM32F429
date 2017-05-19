#include "VidaOS.h"
#include <string.h>

/**********************************************************************************************************
** Function name        :   vTaskInit
** Descriptions         :   ��ʼ������ṹ
** parameters           :   task        Ҫ��ʼ��������ṹ
** parameters           :   entry       �������ں���
** parameters           :   param       ���ݸ���������в���
** Returned value       :   ��
***********************************************************************************************************/
void vTaskInit(vTask * task, void (*entry)(void *), void *param, uint8_t prio, vTaskStack *stackBase, uint32_t size)
{
    uint32_t *stackTop;
    task->stackBase = stackBase;
    task->stackSize = size;
    memset(stackBase, 0, size);

    stackTop = stackBase + size / sizeof(vTaskStack);
    
    // Ϊ�˼򻯴��룬VidaOS������������ʱ�л�����һ�����񣬻��������й������ڲ�ͬ�������л�
    // ��ִ�еĲ��������ȱ��浱ǰ��������л���������CPU�Ĵ���ֵ���Ķ�ջ��(����Ѿ��������������Ļ�)��Ȼ����
    // ȡ������һ������Ķ�ջ��ȡ��֮ǰ�����л���������Ȼ��ָ���CPU�Ĵ���
    // �����л���֮ǰ��û�����й�����������Ϊ������һ������ٵġ������ֳ���Ȼ��ʹ�ø��ֳ��ָ���

    // ע���������㣺
    // 1������Ҫ�õ��ļĴ�����ֱ�����˼Ĵ����ţ�������IDE����ʱ�鿴Ч����
    // 2��˳���ܱ䣬Ҫ���PendSV_Handler�Լ�CPU���쳣�Ĵ������������
    
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

    *(--stackTop) = (unsigned long)(1<<24);             // XPSR, ������Thumbģʽ���ָ���Thumb״̬����ARM״̬����
    *(--stackTop) = (unsigned long)entry;               // �������ڵ�ַ
    *(--stackTop) = (unsigned long)0x14;                // R14(LR), ���񲻻�ͨ��return xxx�����Լ�������δ��
    *(--stackTop) = (unsigned long)0x12;                // R12, δ��
    *(--stackTop) = (unsigned long)0x3;                 // R3, δ��
    *(--stackTop) = (unsigned long)0x2;                 // R2, δ��
    *(--stackTop) = (unsigned long)0x1;                 // R1, δ��
    *(--stackTop) = (unsigned long)param;               // R0 = param, �����������ں���
    *(--stackTop) = (unsigned long)0x11;                // R11, δ��
    *(--stackTop) = (unsigned long)0x10;                // R10, δ��
    *(--stackTop) = (unsigned long)0x9;                 // R9, δ��
    *(--stackTop) = (unsigned long)0x8;                 // R8, δ��
    *(--stackTop) = (unsigned long)0x7;                 // R7, δ��
    *(--stackTop) = (unsigned long)0x6;                 // R6, δ��
    *(--stackTop) = (unsigned long)0x5;                 // R5, δ��
    *(--stackTop) = (unsigned long)0x4;                 // R4, δ��
    
    task->stack = stackTop;                             // �������յ�ֵ
    task->slice = VIDAOS_SLICE_MAX;                     // ��ʼ�������ʱ��Ƭ����
    task->prio = prio;                                  // ������������ȼ�
    task->state = VIDAOS_TASK_STATE_RDY;                // ��������Ϊ����״̬
    task->suspendCount = 0;                             // ��ʼ�������Ϊ0
    task->delayTicks = 0;
    
    vNodeInit(&(task->linkNode));                       // ��ʼ�����ӽ��
    vNodeInit(&(task->delayNode));                      // ��ʼ����ʱ���
  
    task->clean = (void(*)(void *))0;                   // ����������
    task->cleanParam = (void *)0;                       // ���ô��ݸ��������Ĳ���
    task->requestDeleteFlag = 0;                        // ����ɾ�����

    task->waitEvent = (vEvent *)0;                      // û�еȴ��¼�
    task->eventMsg = (void *)0;                         // û�еȴ��¼�
    task->waitEventResult = vErrorNoError;              // û�еȴ��¼�����
    
    task->waitFlagType = 0;
    task->requestEventFlags = 0;
    
    task->stackFreeMin = (uint32_t)stackTop - (uint32_t)stackBase;
    vTaskSchedRdy(task);                                // ����������������
    
#if (VIDAOS_ENABLE_HOOKS == 1)
    vHooksTaskInit(task);
#endif
}

/**********************************************************************************************************
** Function name        :   vTaskSuspend
** Descriptions         :   ����ָ��������
** parameters           :   task        �����������
** Returned value       :   ��
***********************************************************************************************************/
void vTaskSuspend(vTask *task)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ֻ������Ѿ��������״̬���߾���״̬���������
    if ((task->state & VIDAOS_TASK_STATE_RDY) || (task->state & VIDAOS_TASK_STATE_SUSPEND))
    {
        // ���ӹ������������������ִ�е�һ�ι������ʱ���ſ����Ƿ�
        // Ҫִ�������л�����
        if (++task->suspendCount == 1)
        {
            // ����ʽ�ܼ򵥣����ǽ���Ӿ����������Ƴ��������������Ͳ��ᷢ����
            // Ҳ��û���л�������������
            vTaskSchedUnRdy(task);
            
            // ���ù����־
            task->state |= VIDAOS_TASK_STATE_SUSPEND;
            
            // ��Ȼ���������������Լ�����ô���л�����������
            if (task == currentTask)
            {
                vTaskSched();
            }
        }
    }
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vTaskWakeup
** Descriptions         :   ���ѱ����������
** parameters           :   task        �����ѵ�����
** Returned value       :   ��
***********************************************************************************************************/
void vTaskWakeup(vTask *task)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ��������Ƿ��ڹ���״̬
    if (task->state & VIDAOS_TASK_STATE_SUSPEND)
    {
        // �ݼ�������������Ϊ0�ˣ�����������־��ͬʱ���ý������״̬
        if (--task->suspendCount == 0)
        {
            // ��������־
            task->state &= ~VIDAOS_TASK_STATE_SUSPEND;
            
            // ͬʱ������Żؾ���������
            vTaskSchedRdy(task);
            
            // ���������������ȼ����ߣ���ִ�е��ȣ��л���ȥ
            if (task->prio < currentTask->prio)
            {
                vTaskSched();
            }
        }
    }
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vTaskSetCleanCallFunc
** Descriptions         :   ��������ɾ��ʱ���õ�������
** parameters           :   task  �����õ�����
** parameters           :   clean  ��������ڵ�ַ
** parameters           :   param  ���ݸ��������Ĳ���
** Returned value       :   ��
***********************************************************************************************************/
void vTaskSetCleanCallFunc(vTask *task, void (*clean)(void * param), void *param)
{
    task->clean = clean;
    task->cleanParam = param;
}

/**********************************************************************************************************
** Function name        :   vTaskForceDelete
** Descriptions         :   ǿ��ɾ��ָ��������
** parameters           :   task  ��Ҫɾ��������
** Returned value       :   ��
***********************************************************************************************************/
void vTaskForceDelete(vTask *task)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // �����������ʱ״̬�������ʱ������ɾ��
    if (task->state & VIDAOS_TASK_STATE_DELAYED)
    {
        vTDlistTaskRemove(task);
    }
    // ������񲻴��ڹ���״̬����ô���Ǿ���̬���Ӿ�������ɾ��
    else if (!(task->state & VIDAOS_TASK_STATE_SUSPEND))
    {
        vTaskSchedRemove(task);
    }
    
    // ɾ��ʱ������������������������������
    if (task->clean)
    {
        task->clean(task->cleanParam);
    }
    
    // ���ɾ�������Լ�����ô��Ҫ�л�����һ����������ִ��һ���������
    if (currentTask == task)
    {
        vTaskSched();
    }
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vTaskRequestDelete
** Descriptions         :   ����ɾ��ĳ�������������Լ������Ƿ�ɾ���Լ�
** parameters           :   task  ��Ҫɾ��������
** Returned value       :   ��
***********************************************************************************************************/
void vTaskRequestDelete(vTask *task)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // �������ɾ�����
    task->requestDeleteFlag = 1;
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vTaskIsRequestedDelete
** Descriptions         :   �Ƿ��Ѿ�������ɾ���Լ�
** parameters           :   ��
** Returned value       :   ��0��ʾ����ɾ����0��ʾ������
***********************************************************************************************************/
uint8_t vTaskIsRequestedDelete(void)
{
    uint8_t deleteFlag;
    
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ��ȡ����ɾ�����
    deleteFlag = currentTask->requestDeleteFlag;
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
    
    return deleteFlag;
}

/**********************************************************************************************************
** Function name        :   vTaskDeleteSelf
** Descriptions         :   ɾ���Լ�
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vTaskDeleteSelf(void)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // �����ڵ��øú���ʱ�������Ǵ��ھ���״̬�������ܴ�����ʱ����������״̬
    // ���ԣ�ֻ��Ҫ�Ӿ����������Ƴ�����
    vTaskSchedRemove(currentTask);
    
    // ɾ��ʱ������������������������������
    if (currentTask->clean)
    {
        currentTask->clean(currentTask->cleanParam);
    }
    
    // ���������϶����л�����������ȥ����
    vTaskSched();
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vTaskGetInfo
** Descriptions         :   ��ȡ���������Ϣ
** parameters           :   task ��Ҫ��ѯ������
** parameters           :   info ������Ϣ�洢�ṹ
** Returned value       :   ��
***********************************************************************************************************/
void vTaskGetInfo(vTask *task, vTaskInfo *info)
{
    uint32_t *stackEnd;
    vNode *refNode;
    vNode *nextNode;
    uint32_t preTicks = 0;
    
    // �����ٽ���
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
    
    info->delayTicks = preTicks + task->delayTicks;     // ��ʱ��Ϣ
    info->prio = task->prio;                            // �������ȼ�
    info->state = task->state;                          // ����״̬
    info->slice = task->slice;                          // ʣ��ʱ��Ƭ
    info->suspendCount = task->suspendCount;            // ������Ĵ���
    info->stackSize = task->stackSize;
    
    // �����ջʹ����
    info->stackFreeRT = 0;
    info->stackFreeMin = task->stackFreeMin;
    stackEnd = task->stackBase;
    
    while ((*stackEnd++ == 0) && (stackEnd <= task->stackBase + task->stackSize / sizeof(vTaskStack)))
    {
        info->stackFreeRT++;
    }
    
    // ת�����ֽ���
    info->stackFreeRT *= sizeof(vTaskStack);
    
    // ͳ����Сʣ���ջ��
    if (info->stackFreeMin > info->stackFreeRT)
    {
        info->stackFreeMin = info->stackFreeRT;
        task->stackFreeMin = info->stackFreeMin;
    }
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}

