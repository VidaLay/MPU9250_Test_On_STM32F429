#include "VidaOS.h"

/**********************************************************************************************************
** Function name        :   vEventInit
** Descriptions         :   ��ʼ���¼����ƿ�
** parameters           :   event �¼����ƿ�
** parameters           :   type �¼����ƿ������
** Returned value       :   ��
***********************************************************************************************************/
void vEventInit(vEvent *event, vEventType type)
{
    event->type = type;
    vListInit(&event->waitList);
}

/**********************************************************************************************************
** Function name        :   vEventWait
** Descriptions         :   ��ָ�����¼����ƿ��ϵȴ��¼�����
** parameters           :   event �¼����ƿ�
** parameters           :   task �ȴ��¼�����������
** parameters           :   msg �¼���Ϣ�洢�ľ���λ��
** parameters           :   state ��Ϣ����
** parameters           :   timeout �ȴ��೤ʱ��
** Returned value       :   ���ȼ���ߵ��ҿ����е�����
***********************************************************************************************************/
//void vEventWait(vEvent *event, vTask *task, void *msg, uint32_t state, uint32_t timeout)
//{
//    // �����ٽ���
//    uint32_t status = vTastCriticalEnter();
//    
//    task->state |= state;                   // ��������ڵȴ�ĳ���¼���״̬
//    task->waitEvent = event;                // ��������ȴ����¼��ṹ
//    task->eventMsg = msg;                   // ��������ȴ��¼�����Ϣ�洢λ��
//                                            // ����ʱ����Ҫ������Ϣ��������Ҫ������
//    task->waitEventResult = vErrorNoError;  // ����¼��ĵȴ����
//    
//    // ������Ӿ����������Ƴ�
//    vTaskSchedUnRdy(task);
//    
//    // ��������뵽�ȴ�������
//    vListAddLast(&event->waitList, &task->linkNode);
//    
//    // ������������ó�ʱ����ͬʱ���뵽��ʱ������
//    // ��ʱ�䵽��ʱ������ʱ������Ƹ����������ʱ�б����Ƴ���ͬʱ���¼��б����Ƴ�
//    if (timeout)
//    {
//        vTDlistTaskWait(task, timeout);
//    }
//    
//    // �˳��ٽ���
//    vTaskCriticalExit(status);
//}

void vEventWait(vEvent *event, vTask *task, void *msg, uint32_t state, uint32_t timeout)
{
    vNode *refNode;
    vNode *nextNode;
    
    uint8_t isInsert = 0;
    
    // �������ǹ���,ɾ�����ߵȴ��¼�״̬
    if (!((task->state & VIDAOS_TASK_STATE_SUSPEND) || (task->state & VIDAOS_TASK_STATE_DESTROYED) || (task->state & VIDAOS_TASK_WAIT_MASK)))
    {
        // �����ٽ���
        uint32_t status = vTastCriticalEnter();
        
        // ��������Ѿ�����ʱ������,��������
        if (task->state & VIDAOS_TASK_STATE_DELAYED)
        {
            vTDlistTaskRemove(task);
        }
        
        // ��������ھ���״̬,������Ӿ����������Ƴ�
        if (task->state & VIDAOS_TASK_STATE_RDY)
        {
            vTaskSchedUnRdy(task);
        }
            
        for (refNode = event->waitList.headNode.nextNode; refNode != &(event->waitList.headNode); refNode = nextNode)
        {
            vTask *refTask = vNodeParent(refNode, vTask, linkNode);
            nextNode = refNode->nextNode;
            
            // ���task��priority�Ƿ��refTask��priorityС
            if (task->prio < refTask->prio)
            {
                // ���С��, ����������refTask֮ǰ
                vListInsertBefore(&event->waitList, refNode, &(task->linkNode));
                
                isInsert = 1;
                
                break;
            }
        }
        
        if (!isInsert)
        {
            // �����û�в���,��ֱ�Ӳ����β    
            vListAddLast(&event->waitList, &(task->linkNode));
        }
        
        task->state |= state;                   // ��������ڵȴ�ĳ���¼���״̬
        task->waitEvent = event;                // ��������ȴ����¼��ṹ
        task->eventMsg = msg;                   // ��������ȴ��¼�����Ϣ�洢λ��
                                                // ����ʱ����Ҫ������Ϣ��������Ҫ������
        task->waitEventResult = vErrorNoError;  // ����¼��ĵȴ����
        
        // ������������ó�ʱ����ͬʱ���뵽��ʱ������
        // ��ʱ�䵽��ʱ������ʱ������Ƹ����������ʱ�б����Ƴ���ͬʱ���¼��б����Ƴ�
        if (timeout)
        {
            vTDlistTaskWait(task, timeout);
        }
        
        // �˳��ٽ���
        vTaskCriticalExit(status);
    
    }
    
}

/**********************************************************************************************************
** Function name        :   vEventWakeupFirst
** Descriptions         :   ���¼����ƿ��л����׸��ȴ�������
** parameters           :   event �¼����ƿ�
** parameters           :   msg �¼���Ϣ
** parameters           :   result ��֪�¼��ĵȴ����
** Returned value       :   �׸��ȴ����������û������ȴ����򷵻�0
***********************************************************************************************************/
vTask *vEventWakeupFirst(vEvent *event, void *msg, uint32_t result)
{
    vNode *node;
    vTask *task = (vTask *)0;
    
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ȡ���ȴ������еĵ�һ�����
    if ((node = vListRemoveFirst(&event->waitList)) != (vNode *)0)
    {
        // ת��Ϊ��Ӧ������ṹ
        task = vNodeParent(node, vTask, linkNode);
        
        // �����յ�����Ϣ���ṹ�������Ӧ�ĵȴ���־λ
        task->waitEvent = (vEvent *)0;
        task->eventMsg = msg;
        task->waitEventResult = result;
        task->state &= ~VIDAOS_TASK_WAIT_MASK;
        
        // ���������˳�ʱ�ȴ����������£��������ʱ�������Ƴ�
        if (task->state & VIDAOS_TASK_STATE_DELAYED)
        {
            vTDlistTaskRemove(task);
        }
        
        // ����������������
        vTaskSchedRdy(task);
    }
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
    
    return task; 
}

/**********************************************************************************************************
** Function name        :   vEventWakeup
** Descriptions         :   ���¼����ƿ��л���ָ������
** parameters           :   event �¼����ƿ�
** parameters           :   task �ȴ����ѵ�����
** parameters           :   msg �¼���Ϣ
** parameters           :   result ��֪�¼��ĵȴ����
** Returned value       :   �׸��ȴ����������û������ȴ����򷵻�0
***********************************************************************************************************/
void vEventWakeup(vEvent *event, vTask *task, void *msg, uint32_t result)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // �����յ�����Ϣ���ṹ�������Ӧ�ĵȴ���־λ
    vListRemove(&event->waitList, &task->linkNode);
    task->waitEvent = (vEvent *)0;
    task->eventMsg = msg;
    task->waitEventResult = result;
    task->state &= ~VIDAOS_TASK_WAIT_MASK;
    
    // ���������˳�ʱ�ȴ����������£��������ʱ�������Ƴ�
    if (task->state & VIDAOS_TASK_STATE_DELAYED)
    {
        vTDlistTaskRemove(task);
    }
    
    // ����������������
    vTaskSchedRdy(task);
    
    // �˳��ٽ���
    vTaskCriticalExit(status); 
}

/**********************************************************************************************************
** Function name        :   vEventRemoveTask
** Descriptions         :   ���������ȴ�������ǿ���Ƴ�
** parameters           :   task ���Ƴ�������
** parameters           :   result ��֪�¼��ĵȴ����
** Returned value       :   ��
***********************************************************************************************************/
void vEventRemoveTask(vTask *task, void *msg, uint32_t result)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ����������ڵĵȴ��������Ƴ�
    // ע�⣬����û�м��waitEvent�Ƿ�Ϊ�ա���Ȼ�Ǵ��¼����Ƴ�����ô��Ϊ�Ͳ�����Ϊ��
    vListRemove(&task->waitEvent->waitList, &task->linkNode);
    
    // �����յ�����Ϣ���ṹ�������Ӧ�ĵȴ���־λ
    task->waitEvent = (vEvent *)0;
    task->eventMsg = msg;
    task->waitEventResult = result;
    task->state &= ~VIDAOS_TASK_WAIT_MASK;
    
    // ���������˳�ʱ�ȴ����������£��������ʱ�������Ƴ�
    if (task->state & VIDAOS_TASK_STATE_DELAYED)
    {
        vTDlistTaskRemove(task);
    }
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vEventWakeupAll
** Descriptions         :   ������еȴ��е����񣬽��¼����͸���������
** parameters           :   event �¼����ƿ�
** parameters           :   msg �¼���Ϣ
** parameters           :   result ��֪�¼��ĵȴ����
** Returned value       :   ���ѵ���������
***********************************************************************************************************/
uint32_t vEventWakeupAll(vEvent *event, void *msg, uint32_t result)
{
    vNode *node;
    uint32_t count;
    
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ��ȡ�ȴ��е���������
    count = vListCount(&event->waitList);
    
    // �������еȴ��е�����
    while ((node = vListRemoveFirst(&event->waitList)) != (vNode *)0)
    {
        // ת��Ϊ��Ӧ������ṹ
        vTask *task = vNodeParent(node, vTask, linkNode);
        
        // �����յ�����Ϣ���ṹ�������Ӧ�ĵȴ���־λ
        task->waitEvent = (vEvent *)0;
        task->eventMsg = msg;
        task->waitEventResult = result;
        task->state &= ~VIDAOS_TASK_WAIT_MASK;
        
        // ���������˳�ʱ�ȴ����������£��������ʱ�������Ƴ�
        if (task->state & VIDAOS_TASK_STATE_DELAYED)
        {
            vTDlistTaskRemove(task);
        }
        
        // ����������������
        vTaskSchedRdy(task);
    }
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
    
    return count;
}

/**********************************************************************************************************
** Function name        :   vEventWaitCount
** Descriptions         :   �¼����ƿ��еȴ�����������
** parameters           :   event �¼����ƿ�
** parameters           :   msg �¼���Ϣ
** parameters           :   result ��֪�¼��ĵȴ����
** Returned value       :   ���ѵ���������
***********************************************************************************************************/
uint32_t vEventWaitCount(vEvent *event)
{
    uint32_t count = 0;
    
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    count = vListCount(&event->waitList);
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
    
    return count;
}


