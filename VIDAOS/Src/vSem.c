#include "VidaOS.h"

#if (VIDAOS_ENABLE_SEM == 1)

/**********************************************************************************************************
** Function name        :   vSemInit
** Descriptions         :   ��ʼ���ź���
** parameters           :   startCount ��ʼ�ļ���
** parameters           :   maxCount �����������Ϊ0����������
** Returned value       :   ��
***********************************************************************************************************/
void vSemInit(vSem *sem, uint32_t startCount, uint32_t maxCount)
{
    vEventInit(&sem->event, vEventTypeSem);
    
    sem->maxCount = maxCount;
    
    if (maxCount == 0)
    {
        sem->count = startCount;                // maxCount == 0 ��ʾû������
    }
    else
    {
        sem->count = (startCount > maxCount) ? maxCount : startCount;
    }
}

/**********************************************************************************************************
** Function name        :   vSemWait
** Descriptions         :   �ȴ��ź���
** parameters           :   sem �ȴ����ź���
** parameters           :   waitTicks ���ź�������Ϊ0ʱ���ȴ���ticks����Ϊ0ʱ��ʾ��Զ�ȴ�
** Returned value       :   �ȴ����, vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vSemWait(vSem *sem, uint32_t waitTicks)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ���ȼ���ź��������Ƿ����0
    if (sem->count > 0)
    {
        // �������0�Ļ������ĵ�һ����Ȼ�������˳�
        --sem->count;
        
        // �˳��ٽ���
        vTaskCriticalExit(status);
        
        return vErrorNoError;
    }
    else
    {
        // ���򣬽���������¼�������
        vEventWait(&sem->event, currentTask, (void *)0, vEventTypeSem, waitTicks);
        
        // �˳��ٽ���
        vTaskCriticalExit(status);
        
        // �����ִ��һ���¼����ȣ��Ա����л�����������
        vTaskSched();
        
        // �����ڵȴ���ʱ���߼�������ʱ��ִ�л᷵�ص����Ȼ��ȡ���ȴ����
        return currentTask->waitEventResult;
    }
}

/**********************************************************************************************************
** Function name        :   vSemNoWaitGet
** Descriptions         :   ��ȡ�ź���������ź������������ã��������˻�
** parameters           :   sem �ȴ����ź���
** Returned value       :   ��ȡ���, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vSemNoWaitGet(vSem *sem)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ���ȼ���ź��������Ƿ����0
    if (sem->count > 0)
    {
        // �������0�Ļ������ĵ�һ����Ȼ�������˳�
        --sem->count;
        
        // �˳��ٽ���
        vTaskCriticalExit(status);
        
        return vErrorNoError;
    }
    else
    {
        // �����˳��ٽ���������Դ������
        vTaskCriticalExit(status);
        
        return vErrorResourceUnavaliable;
    }
}

/**********************************************************************************************************
** Function name        :   vSemNotify
** Descriptions         :   ֪ͨ�ź������ã����ѵȴ������е�һ�����񣬻��߽�����+1
** parameters           :   sem �������ź���
** Returned value       :   ��
***********************************************************************************************************/
void vSemNotify(vSem *sem)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ����Ƿ�������ȴ�
    if (vEventWaitCount(&sem->event) > 0)
    {
        // ����еĻ�����ֱ�ӻ���λ�ڶ����ײ������ȵȴ���������
        vTask *task = vEventWakeupFirst(&sem->event, (void *)0, vErrorNoError);
        
        // ��������������ȼ����ߣ���ִ�е��ȣ��л���ȥ
        if (task->prio < currentTask->prio)
        {
            vTaskSched();
        }
    }
    else
    {
        // ���û������ȴ��Ļ������Ӽ���
        if (sem->count != 0xFFFFFFFF)
        {
            ++sem->count;
        }
        
        // �����������������������ļ�������ݼ�
        if ((sem->maxCount != 0) && (sem->count > sem->maxCount))
        {
            sem->count = sem->maxCount;
        }
    }
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vSemGetInfo
** Descriptions         :   ��ѯ�ź�����״̬��Ϣ
** parameters           :   sem ��ѯ���ź���
** parameters           :   info ״̬��ѯ�洢��λ��
** Returned value       :   ��
***********************************************************************************************************/
void vSemGetInfo(vSem *sem, vSemInfo *info)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ������Ҫ����Ϣ
    info->count = sem->count;
    info->maxCount = sem->maxCount;
    info->taskCount = vEventWaitCount(&sem->event);
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vSemDestroy
** Descriptions         :   �����ź���
** parameters           :   sem ��Ҫ���ٵ��ź���
** Returned value       :   �����ٸ��ź��������ѵ���������
***********************************************************************************************************/
uint32_t vSemDestroy(vSem *sem)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ����¼����ƿ��е�����
    uint32_t count = vEventWakeupAll(&sem->event, (void *)0, vErrorDel);
    sem->count = 0;
    
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
