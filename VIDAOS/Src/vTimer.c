#include "VidaOS.h"

#if (VIDAOS_ENABLE_TIMER == 1)

// "Ӳ"��ʱ���б�
static vList vTimerHardList;

// "��"��ʱ���б�
static vList vTimerSoftList;

// ���ڷ�����ʱ���б���ź���
static vSem vTimerProtectSem;

// ������ʱ���������ж�ͬ���ļ����ź���
static vSem vTimerTickSem;

/**********************************************************************************************************
** Function name        :   vTimerInit
** Descriptions         :   ��ʼ����ʱ��
** parameters           :   timer �ȴ���ʼ���Ķ�ʱ��
** parameters           :   delayTicks ��ʱ����ʼ��������ʱticks����
** parameters           :   durationTicks �������Զ�ʱ���õ�����tick����һ���Զ�ʱ����Ч
** parameters           :   timerFunc ��ʱ���ص�����
** parameters           :   arg ���ݸ���ʱ���ص������Ĳ���
** parameters           :   timerFunc ��ʱ���ص�����
** parameters           :   config ��ʱ���ĳ�ʼ����
** Returned value       :   ��
***********************************************************************************************************/
void vTimerInit(vTimer *timer, uint32_t delayTicks, uint32_t durationTicks, void (*timerFunc)(void *arg), void *arg, uint32_t config)
{
    vNodeInit(&timer->linkNode);
    timer->startDelayTicks = delayTicks;
    timer->durationTicks = durationTicks;
    timer->timerFunc = timerFunc;
    timer->arg = arg;
    timer->config = config;
    
    // �����ʼ������ʱΪ0����ʹ������ֵ
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
** Descriptions         :   ������ʱ��
** parameters           :   timer �ȴ������Ķ�ʱ��
** Returned value       :   ��
***********************************************************************************************************/
void vTimerStart(vTimer *timer)
{
    switch (timer->state)
    {
    case vTimerCreated:
    case vTimerStopped:
        timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;
        timer->state = vTimerStarted;
        
        // ���ݶ�ʱ�����ͼ�����Ӧ�Ķ�ʱ���б�
        if (timer->config & TIMER_CONFIG_TYPE_HARD)
        {
            // Ӳ��ʱ������ʱ�ӽ����ж��д�������ʹ���ٽ�������
            uint32_t status = vTastCriticalEnter();             // ʱ�ӽ����жϴ����������vTimerHardList,����Ҫ���ٽ�������
            
            // ����Ӳ��ʱ���б�
            vListAddLast(&vTimerHardList, &timer->linkNode);
            
            // �˳��ٽ���
            vTaskCriticalExit(status);
        }
        else
        {
            // ��ʱ�����Ȼ�ȡ�ź������Դ����ʱ��ʱ�������ʱͬʱ�ڷ�����ʱ���б��µĳ�ͻ����
            vSemWait(&vTimerProtectSem, 0);                     // ����ֻ���������ʵ�vTimerSoftList,�жϲ��������,����ֻ��Ҫ���ź���������,��Ϊ�����㹻��,���Բ���Ҫʹ�û����ź���
            
            // ������ʱ���б�
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
** Descriptions         :   ��ֹ��ʱ��
** parameters           :   timer �ȴ������Ķ�ʱ��
** Returned value       :   ��
***********************************************************************************************************/
void vTimerStop(vTimer *timer)
{
    switch (timer->state)
    {
    case vTimerStarted:
    case vTimerFuncRunning:
        // ����Ѿ��������ж϶�ʱ�����ͣ�Ȼ�����Ӧ����ʱ�б����Ƴ�
        if (timer->config & TIMER_CONFIG_TYPE_HARD)
        {
            // Ӳ��ʱ������ʱ�ӽ����ж��д�������ʹ���ٽ�������
            uint32_t status = vTastCriticalEnter();             // ʱ�ӽ����жϴ����������vTimerHardList,����Ҫ���ٽ�������
            
            // ��Ӳ��ʱ���б����Ƴ�
            vListRemove(&vTimerHardList, &timer->linkNode);
            
            // �˳��ٽ���
            vTaskCriticalExit(status);
        }
        else
        {
            vSemWait(&vTimerProtectSem, 0);                     // ����ֻ���������ʵ�vTimerSoftList,�жϲ��������,����ֻ��Ҫ���ź���������,��Ϊ�����㹻��,���Բ���Ҫʹ�û����ź���
            
            // ����ʱ���б����Ƴ�
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
** Descriptions         :   ���ٶ�ʱ��
** parameters           :   timer ���ٵĶ�ʱ��
** Returned value       :   ��
***********************************************************************************************************/
void vTimerDestroy (vTimer * timer)
{
    vTimerStop(timer);
    timer->state = vTimerDestroyed;
}

/**********************************************************************************************************
** Function name        :   vTimerGetInfo
** Descriptions         :   ��ѯ״̬��Ϣ
** parameters           :   timer ��ѯ�Ķ�ʱ��
** parameters           :   info ״̬��ѯ�洢��λ��
** Returned value       :   ��
***********************************************************************************************************/
void vTimerGetInfo (vTimer * timer, vTimerInfo * info)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();

    info->startDelayTicks = timer->startDelayTicks;
    info->durationTicks = timer->durationTicks;
    info->timerFunc = timer->timerFunc;
    info->arg = timer->arg;
    info->config = timer->config;
    info->state = timer->state;

    // �˳��ٽ���
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vTimerListHandler
** Descriptions         :   ����ָ���Ķ�ʱ���б����ø�����ʱ��������
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
static void vTimerListHandler(vList *timerList)
{
    vNode *node;
    vNode *nextNode;
    
    // ������������delayTicks���������0�Ļ�����1
    for (node = timerList->headNode.nextNode; node != &(timerList->headNode); node = nextNode)
    {
        vTimer *timer = vNodeParent(node, vTimer, linkNode);
        nextNode = node->nextNode;
        
        // �����ʱ�ѵ�������ö�ʱ��������
        if ((timer->delayTicks == 0) || (--timer->delayTicks == 0))
        {
            // �л�Ϊ��������״̬
            timer->state = vTimerFuncRunning;
            
            // ���ö�ʱ��������
            timer->timerFunc(timer->arg);
            
            // �л�Ϊ�Ѿ�����״̬
            timer->state = vTimerStarted;
            
            if (timer->durationTicks > 0)
            {
                // ����������Եģ����ظ���ʱ����ֵ
                timer->delayTicks = timer->durationTicks;        
            }
            else
            {
                // ������һ���Լ���������ֹ��ʱ��
                vListRemove(timerList, &timer->linkNode);
                timer->state = vTimerStopped;
            }
        }
    }
}

/**********************************************************************************************************
** Function name        :   vTimerModuleTickNotify
** Descriptions         :   ֪ͨ��ʱģ�飬ϵͳ����tick����
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vTimerModuleTickNotify(void)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ����Ӳ��ʱ���б�
    vTimerListHandler(&vTimerHardList);
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
    
    // ֪ͨ��ʱ�����ı仯
    vSemNotify(&vTimerTickSem);
}

/**********************************************************************************************************
***********************************************************************************************************/
static vTask vTimerTask;
static vTaskStack vTimerTaskStack[VIDAOS_TIMERTASK_STACK_SIZE];

/**********************************************************************************************************
** Function name        :   vTimerSoftTaskEntry
** Descriptions         :   ������ʱ���б������
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
static void vTimerSoftTaskEntry(void *param)
{
    for (;;)
    {
        // �ȴ�ϵͳ���ķ��͵��ж��¼��ź�
        vSemWait(&vTimerTickSem, 0);
        
        // ��ȡ��ʱ���б�ķ���Ȩ��
        vSemWait(&vTimerProtectSem, 0);
        
        // ������ʱ���б�
        vTimerListHandler(&vTimerSoftList);
        
        // �ͷŶ�ʱ���б����Ȩ��
        vSemNotify(&vTimerProtectSem);
    }
}

/**********************************************************************************************************
** Function name        :   vTimerModuleInit
** Descriptions         :   ��ʱ��ģ���ʼ��
** parameters           :   ��
** Returned value       :   ��
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
** Descriptions         :   ��ʼ����ʱ������
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vTimerTaskInit(void)
{
#if VIDAOS_TIMERTASK_PRIO >= (VIDAOS_PRO_COUNT - 1)
    #error "The priority of timer task must be greater than (VIDAOS_PRO_COUNT - 1)"
#endif
    
    vTaskInit(&vTimerTask, vTimerSoftTaskEntry, (void *)0, VIDAOS_TIMERTASK_PRIO, vTimerTaskStack, (VIDAOS_TIMERTASK_STACK_SIZE * sizeof(vTaskStack)));
}

#endif
