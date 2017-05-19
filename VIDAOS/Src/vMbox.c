#include "VidaOS.h"

#if (VIDAOS_ENABLE_MBOX == 1)

/**********************************************************************************************************
** Function name        :   tMboxInit
** Descriptions         :   ��ʼ������
** parameters           :   mbox �ȴ���ʼ��������
** parameters           :   msgBuffer ��Ϣ�洢������
** parameters           :   maxCount ������
** Returned value       :   ��
***********************************************************************************************************/
void vMboxInit(vMbox *mbox, void **msgBuffer, uint32_t maxCount)
{
    vEventInit(&mbox->event, vEventTypeMbox);
    
    mbox->msgBuffer = msgBuffer;
    mbox->maxCount = maxCount;
    mbox->read = 0;
    mbox->write = 0;
    mbox->count = 0;
}

/**********************************************************************************************************
** Function name        :   vMboxWait
** Descriptions         :   �ȴ�����, ��ȡһ����Ϣ
** parameters           :   mbox �ȴ�������
** parameters           :   msg ��Ϣ�洢������
** parameters           :   waitTicks ���ȴ���ticks����Ϊ0��ʾ���޵ȴ�
** Returned value       :   �ȴ����,vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vMboxWait(vMbox *mbox, void **msg, uint32_t waitTicks)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ���ȼ����Ϣ�����Ƿ����0
    if (mbox->count > 0)
    {
        // �������0�Ļ���ȡ��һ��
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];
        
        // ͬʱ��ȡ����ǰ�ƣ���������߽������
        if (mbox->read >= mbox->maxCount)
        {
            mbox->read = 0;
        }
        
        // �˳��ٽ���
        vTaskCriticalExit(status);
        
        return vErrorNoError;
    }
    else
    {
        // ���� ����������¼�������
        vEventWait(&mbox->event, currentTask, (void *)0, vEventTypeMbox, waitTicks);
        
        // �˳��ٽ���
        vTaskCriticalExit(status);
        
        // �����ִ��һ���¼����ȣ��Ա����л�����������
        vTaskSched();
        
        // ���л�����ʱ����vTask��ȡ����õ���Ϣ
        *msg = (uint8_t *)currentTask->eventMsg;
        
        // ȡ���ȴ����
        return currentTask->waitEventResult;
    }
}

/**********************************************************************************************************
** Function name        :   vMboxNoWaitGet
** Descriptions         :   ��ȡһ����Ϣ�����û����Ϣ���������˻�
** parameters           :   mbox ��ȡ��Ϣ������
** parameters           :   msg ��Ϣ�洢������
** Returned value       :   ��ȡ���, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vMboxNoWaitGet(vMbox *mbox, void **msg)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ���ȼ����Ϣ�����Ƿ����0
    if (mbox->count > 0)
    {
        // �������0�Ļ���ȡ��һ��
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];
        
        // ͬʱ��ȡ����ǰ�ƣ���������߽������
        if (mbox->read >= mbox->maxCount)
        {
            mbox->read = 0;
        }
        
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
** Function name        :   vMboxNotify
** Descriptions         :   ֪ͨ��Ϣ���ã����ѵȴ������е�һ�����񣬻��߽���Ϣ���뵽������
** parameters           :   mbox �������ź���
** parameters           :   msg ���͵���Ϣ
** parameters           :   notifyOption ���͵�ѡ�� vMboxStoreNormal/vMboxStoreFront/vMboxSentToAll
** Returned value       :   vErrorNoError/ vErrorResourceFull
***********************************************************************************************************/
uint32_t vMboxNotify(vMbox *mbox, void *msg, uint32_t notifyOption)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ����Ƿ�������ȴ�
    if (vEventWaitCount(&mbox->event) > 0)
    {
        if (notifyOption & vMboxSentToAll)
        {
            // �����vMboxSentToAll�Ļ�����ֱ�ӻ�����������
            vEventWakeupAll(&mbox->event, (void *)msg, vErrorNoError);
            
            vTaskSched();
        }
        else
        {
            // �������vMboxSentToAll�Ļ�����ֱ�ӻ���λ�ڶ����ײ������ȵȴ���������
            vTask *task = vEventWakeupFirst(&mbox->event, (void *)msg, vErrorNoError);
            
            // ��������������ȼ����ߣ���ִ�е��ȣ��л���ȥ
            if (task->prio < currentTask->prio)
            {
                vTaskSched();
            }
        }
    }
    else
    {
        // ���û������ȴ��Ļ�������Ϣ���뵽��������
        if (mbox->count >= mbox->maxCount)
        {
            // �˳��ٽ���
            vTaskCriticalExit(status);
            
            return vErrorResourceFull;
        }
        
        // ����ѡ����Ϣ���뵽ͷ���������������ȡ����Ϣ��ʱ�����Ȼ�ȡ����Ϣ
        if (notifyOption & vMboxStoreFront)
        {
            if (mbox->read <= 0)
            {
                mbox->read = mbox->maxCount - 1;
            }
            else
            {
                --mbox->read;
            }
            
            mbox->msgBuffer[mbox->read] = msg;
        }
        else
        {
            mbox->msgBuffer[mbox->write++] = msg;
            if (mbox->write >= mbox->maxCount)
            {
                mbox->write = 0;
            }
        }
        
        // ������Ϣ����
        mbox->count++;
    }
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
    
    return vErrorNoError;
}

/**********************************************************************************************************
** Function name        :   vMboxFlush
** Descriptions         :   ���������������Ϣ
** parameters           :   mbox �ȴ���յ�����
** Returned value       :   ��
***********************************************************************************************************/
void vMboxFlush(vMbox *mbox)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ���������������ȴ���˵�������Ѿ��ǿյ��ˣ�����Ҫ�����
    //if (vEventWaitCount(&mbox->event) == 0)
    if (mbox->count > 0)
    {
        mbox->read = 0;
        mbox->write = 0;
        mbox->count = 0;
    }
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vMboxDestroy
** Descriptions         :   ��������
** parameters           :   mbox ��Ҫ���ٵ�����
** Returned value       :   �����ٸ��ź��������ѵ���������
***********************************************************************************************************/
uint32_t vMboxDestroy(vMbox *mbox)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ����¼����ƿ��е�����
    uint32_t count = vEventWakeupAll(&mbox->event, (void *)0, vErrorDel);
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
    
    // ��չ����п��������������ִ��һ�ε���
    if (count > 0)
    {
        vTaskSched();
    }
    
    return count;
}

/**********************************************************************************************************
** Function name        :   vMboxGetInfo
** Descriptions         :   ��ѯ״̬��Ϣ
** parameters           :   mbox ��ѯ������
** parameters           :   info ״̬��ѯ�洢��λ��
** Returned value       :   ��
***********************************************************************************************************/
void vMboxGetInfo(vMbox *mbox, vMboxInfo *info)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    info->count = mbox->count;
    info->maxCount = mbox->maxCount;
    info->taskCount = vEventWaitCount(&mbox->event);
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}

#endif
