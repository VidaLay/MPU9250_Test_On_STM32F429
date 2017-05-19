#include "VidaOS.h"

// ���������
vList OSRdyTable[VIDAOS_PRO_COUNT];

// �������ȼ��ı��λ�ýṹ
vBitmap taskPrioBitmap;

// ������������
uint8_t schedLockCount;

/**********************************************************************************************************
** Function name        :   vTaskHighestReady
** Descriptions         :   ��ȡ��ǰ������ȼ��ҿ����е�����
** parameters           :   ��
** Returned value       :   ���ȼ���ߵ��ҿ����е�����
***********************************************************************************************************/
vTask *vTaskHighestReady(void)
{
    uint32_t highestPrio = vBitmapGetFirstSet(&taskPrioBitmap);
    vNode *node = vListFirst(&OSRdyTable[highestPrio]);
    return vNodeParent(node, vTask, linkNode);
}

/**********************************************************************************************************
** Function name        :   vTaskSchedInit
** Descriptions         :   ��ʼ��������
** parameters           :   ��
** Returned value       :   ��
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
** Descriptions         :   ��ֹ�������
** parameters           :   ��
** Returned value       :   ��
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
** Descriptions         :   �����������
** parameters           :   ��
** Returned value       :   ��
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
** Descriptions         :   ����������Ϊ����״̬
** parameters           :   task    �ȴ�����Ϊ����״̬������
** Returned value       :   ��
***********************************************************************************************************/
void vTaskSchedRdy(vTask *task)
{
    vListAddLast(&(OSRdyTable[task->prio]),&(task->linkNode));
    vBitmapSet(&taskPrioBitmap, task->prio);
    task->state |= VIDAOS_TASK_STATE_RDY;
}

/************************************************************************************************************
** Descriptions         :   vTaskSchedUnRdy
** Descriptions         :   �������Ƴ�����״̬
** parameters           :   task    �ȴ��Ƴ�����״̬������
** Returned value       :   ��
***********************************************************************************************************/
void vTaskSchedUnRdy(vTask *task)
{
    if (task->state & VIDAOS_TASK_STATE_RDY)
    {
        vListRemove(&(OSRdyTable[task->prio]),&(task->linkNode));
        
        // �����п��ܴ��ڶ������ֻ�е�û������ʱ�������λͼ���
        if (vListCount(&OSRdyTable[task->prio]) == 0)
        {
            vBitmapClear(&taskPrioBitmap, task->prio);
        } 
        
        task->state &= ~VIDAOS_TASK_STATE_RDY;
    }
}

/**********************************************************************************************************
** Function name        :   vTaskSchedRemove
** Descriptions         :   ������Ӿ����б����Ƴ�
** parameters           :   task    �ȴ��Ƴ�������
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
** Descriptions         :   ������Ƚӿڡ�VidaOSͨ������ѡ����һ�����������Ȼ���л������������С�
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vTaskSched(void)
{
    vTask *tempTask;
    
    // �����ٽ������Ա�������������������л��ڼ䣬������Ϊ�����жϵ���currentTask��nextTask���ܸ���
    uint32_t status = vTastCriticalEnter();
    
    // ��ε������Ѿ����������򲻽��е��ȣ�ֱ����bm
    if (schedLockCount > 0)
    {
        vTaskCriticalExit(status);
        return;
    } 
    
    // �ҵ����ȼ���ߵ����������������ȼ����ܱȵ�ǰ�͵�
    // ���ǵ�ǰ��������Ϊ��ʱ����Ҫ�л������Ա����л���ȥ��Ҳ����˵������ͨ���ж����ȼ��������Ƿ��л�
    // ֻҪ�жϲ��ǵ�ǰ���񣬾������л���ȥ
    tempTask = vTaskHighestReady();
    if (tempTask != currentTask)
    {
        nextTask = tempTask;
        
#if (VIDAOS_ENABLE_HOOKS == 1)
        vHooksTaskSwitch(currentTask, nextTask);
#endif
        
        vTaskSwitch();
    }
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}
