#include "VidaOS.h"

#if (VIDAOS_ENABLE_FLAGGROUP == 1)

/**********************************************************************************************************
** Function name        :   vFlagGroupInit
** Descriptions         :   ��ʼ���¼���־��
** parameters           :   flagGroup �ȴ���ʼ�����¼���־��
** parameters           :   flags ��ʼ���¼���־
** Returned value       :   ��
***********************************************************************************************************/
void vFlagGroupInit(vFlagGroup * flagGroup, uint32_t flags)
{
    vEventInit(&flagGroup->event, vEventTypeFlagGroup);
    flagGroup->flags = flags;
}

/**********************************************************************************************************
** Function name        :   vFlagGroupCheckAndConsume
** Descriptions         :   ������������鲢���ĵ��¼���־
** parameters           :   flagGroup �ȴ���ʼ�����¼���־��
** parameters           :   waitType �¼���־�������
** parameters           :   flags ������¼���־�洢��ַ�ͼ�����洢λ��
** Returned value       :   vErrorNoError �¼�ƥ�䣻vErrorResourceUnavaliable �¼�δƥ��
***********************************************************************************************************/
static uint32_t vFlagGroupCheckAndConsume(vFlagGroup *flagGroup, uint32_t waitType, uint32_t *flags)
{
    uint32_t reqFlag = *flags;
    uint32_t isSet = waitType & VFLAGGROUP_SET;
    uint32_t isAll = waitType & VFLAGGROUP_ALL;
    uint32_t isConsume = waitType & VFLAGGROUP_CONSUME;
    
    // ����Щ���͵ı�־λ����
	// flagGroup->flags & flags���������ЩλΪ1
	// ~flagGroup->flags & flags:�������λΪ0
    uint32_t calFlag = isSet ? (flagGroup->flags & reqFlag) : (~flagGroup->flags & reqFlag);
    
    // ���б�־λ����, �����������־λ���֣���������
    if (((isAll != 0) && (calFlag == reqFlag)) || ((isAll == 0) && (calFlag != 0)))
    {
        // �Ƿ����ĵ���־λ
        if (isConsume)   
        {
            if (isSet)
            {
                // ���Ϊ1�ı�־λ�����0
                flagGroup->flags &=  ~reqFlag;
            }
            else
            {
                // ���Ϊ0�ı�־λ�����1
                flagGroup->flags |=  reqFlag;
            }
        }
        
        *flags = calFlag;
        
        return vErrorNoError;
    }
    
    *flags = calFlag;                       //��ʱ��flags�Ǳ�ʾflagGroup->flags �� requestFlag �����λ
    
    return vErrorResourceUnavaliable;
}

/**********************************************************************************************************
** Function name        :   vFlagGroupWait
** Descriptions         :   �ȴ��¼���־�����ض��ı�־
** parameters           :   flagGroup �ȴ����¼���־��
** parameters           :   waitType �ȴ����¼�����
** parameters           :   requstFlag ������¼���־
** parameters           :   resultFlag �ȴ���־���
** parameters           :   waitTicks ���ȴ��ı�־û����������ʱ���ȴ���ticks����Ϊ0ʱ��ʾ��Զ�ȴ�
** Returned value       :   �ȴ����, vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vFlagGroupWait(vFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag, uint32_t waitTicks)
{
    uint32_t result;
    uint32_t flags = requestFlag;
    
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    result = vFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
    
    if (result != vErrorNoError)
    {
        // ����¼���־����������������뵽�ȴ�������
        currentTask->waitFlagType = waitType;
        currentTask->requestEventFlags = requestFlag;
        vEventWait(&flagGroup->event, currentTask, (void *)0, vEventTypeFlagGroup, waitTicks);
        
        // �˳��ٽ���
        vTaskCriticalExit(status);
        
        // ��ִ��һ���¼����ȣ��Ա����л�����������
        vTaskSched();
        
        // ���л�����ʱ����vTask��ȡ����õ���Ϣ
        *resultFlag = currentTask->requestEventFlags;
        
        // ȡ���ȴ����
        result = currentTask->waitEventResult;       
    }
    else
    {
        *resultFlag = flags;
        
        // �˳��ٽ���
        vTaskCriticalExit(status);
    }
    
    return result;
}

/**********************************************************************************************************
** Function name        :   vFlagGroupNoWaitGet
** Descriptions         :   ��ȡ�¼���־�����ض��ı�־
** parameters           :   flagGroup ��ȡ���¼���־��
** parameters           :   waitType ��ȡ���¼�����
** parameters           :   requstFlag ������¼���־
** parameters           :   resultFlag �ȴ���־���
** Returned value       :   ��ȡ���, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vFlagGroupNoWaitGet(vFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag)  //�ú���ֻ��������ѯ��ǰ��־λ�������־λ�����λ
{
    uint32_t result;
    uint32_t flags = requestFlag;
    
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    result = vFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
    
    *resultFlag = flags;
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
    
    return result;
}

/**********************************************************************************************************
** Function name        :   vFlagGroupNotify
** Descriptions         :   ֪ͨ�¼���־���е��������µı�־����
** parameters           :   flagGroup �¼���־��
** parameters           :   isSet �Ƿ�����1�¼�
** parameters           :   flags �������¼���־
***********************************************************************************************************/
void vFlagGroupNotify(vFlagGroup *flagGroup, uint8_t isSet, uint32_t flags)
{
    vList *waitList;
    vNode *node;
    vNode *nextNode;
    uint8_t sched = 0;
    
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    if (isSet)
    {
        flagGroup->flags |= flags;          // ��1�¼�
    }
    else
    {
        flagGroup->flags &= ~flags;         // ��0�¼�
    }
    
    waitList = &flagGroup->event.waitList;
    
    // �������еĵȴ�����, ��ȡ�������������񣬼��뵽���Ƴ��б���
    for (node = waitList->headNode.nextNode; node != &(waitList->headNode); node = nextNode)
    {
        uint32_t result;
        vTask *task = vNodeParent(node, vTask, linkNode);
        uint32_t flags = task->requestEventFlags;
        nextNode = node->nextNode;
        
        result = vFlagGroupCheckAndConsume(flagGroup, task->waitFlagType, &flags); 
        
        if (result == vErrorNoError)
        {
            // ��������
            task->requestEventFlags = flags;
            vEventWakeup(&flagGroup->event, task, (void *)0, vErrorNoError);
            sched = 1;
        }
    }
    
    // ����������������ִ��һ�ε���
    if (sched)
    {
        vTaskSched();
    }
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vFlagGroupGetInfo
** Descriptions         :   ��ѯ�¼���־���״̬��Ϣ
** parameters           :   flagGroup �¼���־��
** parameters           :   info ״̬��ѯ�洢��λ��
** Returned value       :   ��
***********************************************************************************************************/
void vFlagGroupGetInfo(vFlagGroup *flagGroup, vFlagGroupInfo * info)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    info->flags = flagGroup->flags;
    info->taskCount = vEventWaitCount(&flagGroup->event);
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vFlagGroupDestroy
** Descriptions         :   �����¼���־��
** parameters           :   flagGroup �¼���־��
** Returned value       :   �����ٸô洢���ƿ�����ѵ���������
***********************************************************************************************************/
uint32_t vFlagGroupDestroy(vFlagGroup *flagGroup)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ����¼����ƿ��е�����
    uint32_t count = vEventWakeupAll(&flagGroup->event, (void *)0, vErrorDel);
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
    
    // ��չ����п��������������ִ��һ�ε���
    if (count > 0)
    {
        vTaskSched();
    }
    
    return count;
}

#endif
