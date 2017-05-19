#include "VidaOS.h"

#if (VIDAOS_ENABLE_MUTEX == 1)

/**********************************************************************************************************
** Function name        :   vMutexInit
** Descriptions         :   ��ʼ�������ź���
** parameters           :   mutex �ȴ���ʼ���Ļ����ź���
** Returned value       :   ��
***********************************************************************************************************/
void vMutexInit(vMutex *mutex)
{
    vEventInit(&mutex->event, vEventTypeMutex);
    
    mutex->lockedCount = 0;
    mutex->owner = (vTask *)0;
    mutex->ownerOriginalPrio = VIDAOS_PRO_COUNT;
}

/**********************************************************************************************************
** Function name        :   vMutexWait
** Descriptions         :   �ȴ��ź���
** parameters           :   mutex �ȴ����ź���
** parameters           :   waitTicks ���ȴ���ticks����Ϊ0��ʾ���޵ȴ�
** Returned value       :   �ȴ����, vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vMutexWait(vMutex *mutex, uint32_t waitTicks)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    if (mutex->lockedCount <= 0)
    {
        // ���û����������ʹ�õ�ǰ��������
        mutex->owner = currentTask;
        mutex->ownerOriginalPrio = currentTask->prio;
        mutex->lockedCount ++;
        
        // �˳��ٽ���
        vTaskCriticalExit(status);
        
        return vErrorNoError;
    }
    else
    {
        // �ź����Ѿ�������
        if (currentTask == mutex->owner)
        {
            // ������ź�����ӵ�����ٴ�wait�������Ӽ���
            mutex->lockedCount++;
            
            // �˳��ٽ���
            vTaskCriticalExit(status);
            
            return vErrorNoError;
        }
        else
        {
            // ������ź���ӵ����֮�������wait����Ҫ������Ƿ���Ҫʹ�����ȼ��̳з�ʽ����
            if (currentTask->prio < mutex->owner->prio)
            {
                // �����ǰ��������ȼ���ӵ�������ȼ����ߣ���ʹ�����ȼ��̳�����ԭӵ���ߵ�����
                if (mutex->owner->state & VIDAOS_TASK_STATE_RDY)
                {
                    // �����ھ���״̬ʱ�����������ھ������е�λ��
                    vTaskSchedUnRdy(mutex->owner);
                    mutex->owner->prio = currentTask->prio;
                    vTaskSchedRdy(mutex->owner);
                }
                else
                {
                    // ����״̬��ֻ��Ҫ�޸����ȼ�
                    mutex->owner->prio = currentTask->prio;
                }
            }
            
            // ��ǰ�������ȴ�������
            vEventWait(&mutex->event, currentTask, (void *)0, vEventTypeMutex, waitTicks);
            
            // �˳��ٽ���
            vTaskCriticalExit(status);
            
            // ִ�е��ȣ� �л�����������
            vTaskSched();
            
            // ���л�����ʱ��ȡ���ȴ����
            return currentTask->waitEventResult;
        }
    }
}

/**********************************************************************************************************
** Function name        :   vMutexNoWaitGet
** Descriptions         :   ��ȡ�ź���������Ѿ�����������������
** parameters           :   mutex �ȴ����ź���
** Returned value       :   ��ȡ���, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vMutexNoWaitGet(vMutex *mutex)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    if (mutex->lockedCount <= 0)
    {
        // ���û����������ʹ�õ�ǰ��������
        mutex->owner = currentTask;
        mutex->ownerOriginalPrio = currentTask->prio;
        mutex->lockedCount ++;
        
        // �˳��ٽ���
        vTaskCriticalExit(status);
        
        return vErrorNoError;
    }
    else
    {
        // �ź����Ѿ�������
        if (currentTask == mutex->owner)
        {
            // ������ź�����ӵ�����ٴ�wait�������Ӽ���
            mutex->lockedCount++;
            
            // �˳��ٽ���
            vTaskCriticalExit(status);
            
            return vErrorNoError;
        }
        else
        {
            // �˳��ٽ���
            vTaskCriticalExit(status);
            
            return vErrorResourceUnavaliable;
        }
    }
}

/**********************************************************************************************************
** Function name        :   vMutexNotify
** Descriptions         :   ֪ͨ�����ź�������
** parameters           :   mbox �������ź���
** parameters           :   msg ���͵���Ϣ
** parameters           :   notifyOption ���͵�ѡ��
** Returned value       :   vErrorResourceFull
***********************************************************************************************************/
uint32_t vMutexNotify(vMutex *mutex)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    if (mutex->lockedCount <= 0)
    {
        // ��������Ϊ0���ź���δ��������ֱ���˳�
        // �˳��ٽ���
        vTaskCriticalExit(status);
        
        return vErrorNoError;
    }

    if (mutex->owner != currentTask)         // ��ռ���߲��ǵ�ǰ����ʱ����,�ͷ��߱�����ռ����
    {
        // ����ӵ�����ͷţ���Ϊ�ǷǷ�
        // �˳��ٽ���
        vTaskCriticalExit(status);
        
        return vErrorOwner;
    }

    if (--mutex->lockedCount > 0)
    {
        // ��1������Բ�Ϊ0, ֱ���˳�������Ҫ���ѵȴ�������
        // �˳��ٽ���
        vTaskCriticalExit(status);
        
        return vErrorNoError;
    }

    // �Ƿ��з������ȼ��̳�
    if (mutex->ownerOriginalPrio != mutex->owner->prio)
    {
        // �з������ȼ��̳У��ָ�ӵ���ߵ����ȼ�
        if (mutex->owner->state & VIDAOS_TASK_STATE_RDY)
        {
            // �����ھ���״̬ʱ�����������ھ������е�λ��
            vTaskSchedUnRdy(mutex->owner);
            mutex->owner->prio = mutex->ownerOriginalPrio;
            vTaskSchedRdy(mutex->owner);
        }
        else
        {
            // ����״̬��ֻ��Ҫ�޸����ȼ�
            mutex->owner->prio = mutex->ownerOriginalPrio;
        }
    }

    // ����Ƿ�������ȴ�
    if (vEventWaitCount(&mutex->event) > 0)
    {
        // ����еĻ�����ֱ�ӻ���λ�ڶ����ײ������ȵȴ���������
        vTask *task = vEventWakeupFirst(&mutex->event, (void *)0, vErrorNoError);
        
        mutex->owner = task;
        mutex->ownerOriginalPrio = task->prio;
        mutex->lockedCount++;
        
        // ��������������ȼ����ߣ���ִ�е��ȣ��л���ȥ
        if (task->prio < currentTask->prio)
        {
            vTaskSched();
        }
        
    }
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
    
    return vErrorNoError;
}

/**********************************************************************************************************
** Function name        :   vMutexDestroy
** Descriptions         :   �����ź���
** parameters           :   mutex �����ٻ��ĳ��ź���
** Returned value       :   �����ٸ��ź��������ѵ���������
***********************************************************************************************************/
uint32_t vMutexDestroy(vMutex *mutex)
{
    uint32_t count;
    
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // �ź����Ƿ��Ѿ���������δ����ʱû������ȴ������ش���
    if (mutex->lockedCount >0)
    {
        // �Ƿ��з������ȼ��̳�? �������, ��Ҫ�ָ�ӵ���ߵ�ԭ���ȼ�
        if (mutex->ownerOriginalPrio != mutex->owner->prio)
        {
            // �з������ȼ��̳У��ָ�ӵ���ߵ����ȼ�
            if (mutex->owner->state & VIDAOS_TASK_STATE_RDY)
            {
                // �����ھ���״̬ʱ�����������ھ������е�λ��
                vTaskSchedUnRdy(mutex->owner);
                mutex->owner->prio = mutex->ownerOriginalPrio;
                vTaskSchedRdy(mutex->owner);
            }
            else
            {
                // ����״̬��ֻ��Ҫ�޸����ȼ�
                mutex->owner->prio = mutex->ownerOriginalPrio;
            }
        }
        
        // Ȼ������¼����ƿ��е�����
        count = vEventWakeupAll(&mutex->event, (void *)0, vErrorDel);
        
        // ��չ�������������������ִ��һ�ε���
        if (count > 0)
        {
            vTaskSched();
        }
    }
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
    
    return count;
}

/**********************************************************************************************************
** Function name        :   vMutexGetInfo
** Descriptions         :   ��ѯ״̬��Ϣ
** parameters           :   mutex ��ѯ�Ļ����ź���
** parameters           :   info ״̬��ѯ�洢��λ��
** Returned value       :   ��
***********************************************************************************************************/
void vMutexGetInfo(vMutex *mutex, vMutexInfo *info)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    info->taskCount = vEventWaitCount(&mutex->event);
    info->ownerPrio = mutex->ownerOriginalPrio;
    
    if (mutex->owner != (vTask *)0)
    {
        info->inheritedPrio = mutex->owner->prio;
    }
    else
    {
        info->inheritedPrio = VIDAOS_PRO_COUNT;
    }
    
    info->owner = mutex->owner;
    info->lockedCount = mutex->lockedCount;
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}

#endif
