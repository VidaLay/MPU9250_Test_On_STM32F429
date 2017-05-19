#include "VidaOS.h"

// ������ʱ����
vList vTaskDelayedList;

/**********************************************************************************************************
** Function name        :   vTaskDelayedInit
** Descriptions         :   ��ʼ��������ʱ����
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vTaskDelayedInit(void)
{
    vListInit(&vTaskDelayedList);
}

/**********************************************************************************************************
** Function name        :   vTDlistTaskWait
** Descriptions         :   �����������ʱ������
** parameters           :   task    ��Ҫ��ʱ������
** parameters           :   ticks   ��ʱ��ticks
** Returned value       :   ��
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
    
    // �������ǹ������ɾ��״̬
    if (!((task->state & VIDAOS_TASK_STATE_SUSPEND) || (task->state & VIDAOS_TASK_STATE_DESTROYED)))
    {
        // ��������Ѿ�����ʱ������,��������,�����²��뵽��ʱ����
        if (task->state & VIDAOS_TASK_STATE_DELAYED)
        {
            vTDlistTaskRemove(task);
        }
        
        // ��������ھ���״̬,������Ӿ����������Ƴ�
        if (task->state & VIDAOS_TASK_STATE_RDY)
        {
            vTaskSchedUnRdy(task);
        }
            
        for (refNode = vTaskDelayedList.headNode.nextNode; refNode != &(vTaskDelayedList.headNode); refNode = nextNode)
        {
            vTask *refTask = vNodeParent(refNode, vTask, delayNode);
            nextNode = refNode->nextNode;
       
            // ���ticks�Ƿ��refTask��delayTicksС
            if (ticks < refTask->delayTicks)
            {
                // ���ticksС��, ����������refTask֮ǰ
                vListInsertBefore(&vTaskDelayedList, refNode, &(task->delayNode));
                
                // ���²ο������delayTicksֵ
                refTask->delayTicks -= ticks;
                
                // �����Ѳ��������delayTicksֵ
                task->delayTicks = ticks;
                
                isInsert = 1;
                
                break;
            }
            else
            {
                // ����, ����ticksֵ
                ticks -= refTask->delayTicks;
            }
        }
        
        if (!isInsert)
        {
            // �����û�в���
            task->delayTicks = ticks;
        
            vListAddLast(&vTaskDelayedList, &(task->delayNode));
        }
        
        task->state |= VIDAOS_TASK_STATE_DELAYED;
    }
}

/**********************************************************************************************************
** Function name        :   vTDlistTaskWakeup
** Descriptions         :   ����ʱ���������ʱ�����л���
** parameters           :   task  ��Ҫ���ѵ�����
** Returned value       :   ��
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
    
    // ����������������
    vTaskSchedRdy(task);
}

/**********************************************************************************************************
** Function name        :   vTDlistTaskRemove
** Descriptions         :   ����ʱ���������ʱ�������Ƴ�
** parameters           :   task  ��Ҫ�Ƴ�������
** Returned value       :   ��
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
** Descriptions         :   ʹ��ǰ���������ʱ״̬��
** parameters           :   delay ��ʱ���ٸ�ticks
** Returned value       :   ��
***********************************************************************************************************/
void vTaskDelay(uint32_t delay)
{
    if (delay > 0)
    {
        // �����ٽ������Ա�������������������л��ڼ䣬������Ϊ�����жϵ���currentTask��nextTask���ܸ���
        uint32_t status = vTastCriticalEnter();
        
//        // ������Ӿ��������Ƴ�
//        vTaskSchedUnRdy(currentTask);
        
        // ������ʱֵ��������ʱ����,��������Ӿ��������Ƴ�
        vTDlistTaskWait(currentTask, delay);
        
        // �˳��ٽ���
        vTaskCriticalExit(status);
        
        // Ȼ����������л����л�����һ�����񣬻��߿�������
        // delayTikcs����ʱ���ж����Զ���1.������0ʱ�����л������������С�    
        vTaskSched();
    }
}

/**********************************************************************************************************
** Function name        :   vTaskDelay_ms
** Descriptions         :   ʹ��ǰ���������ʱ״̬��
** parameters           :   ms ��ʱ�ĺ�����
** Returned value       :   ��
***********************************************************************************************************/
void vTaskDelay_ms(uint32_t ms)
{
    uint32_t delay = ms / (1000 / TICKS_PER_SEC);
    
    vTaskDelay(delay);
}

/**********************************************************************************************************
** Function name        :   vTaskDelayHumanTime
** Descriptions         :   ʹ��ǰ���������ʱ״̬��
** parameters           :   hour ��ʱ��Сʱ��
** parameters           :   minute ��ʱ�ķ�����
** parameters           :   second ��ʱ������
** parameters           :   ms ��ʱ�ĺ�����
** Returned value       :   ��
***********************************************************************************************************/
void vTaskDelayHumanTime(uint32_t hour, uint32_t minute, uint32_t second, uint16_t ms)
{
    uint32_t delay = ((hour*60*60*1000) + (minute*60*1000) + (second*1000) + ms) / (1000 / TICKS_PER_SEC);
    
    vTaskDelay(delay);
}


