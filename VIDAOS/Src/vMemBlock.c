#include "VidaOS.h"

#if (VIDAOS_ENABLE_MEMBLOCK == 1)

/**********************************************************************************************************
** Function name        :   vMemBlockInit
** Descriptions         :   ��ʼ���洢���ƿ�
** parameters           :   memBlock �ȴ���ʼ���Ĵ洢��ṹ
** parameters           :   memStart �洢������ʼ��ַ
** parameters           :   blockSize ÿ����Ĵ�С
** parameters           :   blockCnt �ܵĿ�����
** Returned value       :   ���ѵ���������
***********************************************************************************************************/
void vMemBlockInit(vMemBlock *memBlock, uint8_t *memStart, uint32_t blockSize, uint32_t blockCnt)
{
    uint8_t *memBlockStart = (uint8_t *)memStart;                                       //u8 * ��Ϊ����ָһ��ָ��һ���ֽڵ�ָ��
    uint8_t *memBlockEnd = memBlockStart + blockSize * blockCnt;
    
    // ÿ���洢����Ҫ����������ָ�룬���Կռ�����Ҫ��tNode��
	// ������ˣ�ʵ���û����õĿռ䲢û����
    if (blockSize < sizeof(vNode))
    {
        return;
    }

    vEventInit(&memBlock->event, vEventTypeMemBlock);
    memBlock->memstart = memStart;
    memBlock->blockSize = blockSize;
    memBlock->maxCount = blockCnt;
    
    vListInit(&memBlock->blockList);
    
    while (memBlockStart < memBlockEnd)
    {
        vNodeInit((vNode *) memBlockStart);
        vListAddLast(&memBlock->blockList, (vNode *) memBlockStart);
        
        memBlockStart += blockSize;
    }

}

/**********************************************************************************************************
** Function name        :   vMemBlockWait
** Descriptions         :   �ȴ��洢��
** parameters           :   memBlock �ȴ��Ĵ洢��ṹ
** parameters			:   mem �洢��
** parameters           :   waitTicks ��û�д洢��ʱ���ȴ���ticks����Ϊ0ʱ��ʾ��Զ�ȴ�
** Returned value       :   �ȴ����,vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vMemBlockWait(vMemBlock *memBlock, uint8_t **mem, uint32_t waitTicks)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ���ȼ���Ƿ��п��еĴ洢��
    if (vListCount(&memBlock->blockList) > 0)
    {
        // ����еĻ���ȡ��һ��
        *mem = (uint8_t *)vListRemoveFirst(&memBlock->blockList);
        
        // �˳��ٽ���
        vTaskCriticalExit(status);
        
        return vErrorNoError;
    }
    else
    {
        // ����, ����������¼�������
        vEventWait(&memBlock->event, currentTask, (void *)0, vEventTypeMemBlock, waitTicks);
        
        // �˳��ٽ���
        vTaskCriticalExit(status);
        
        // �����ִ��һ���¼����ȣ��Ա����л�����������
        vTaskSched();
        
        // ���л�����ʱ����vTask��ȡ����õ���Ϣ
        *mem = (uint8_t *)currentTask->eventMsg;
        
        // ȡ���ȴ����
        return currentTask->waitEventResult;
    }
}

/**********************************************************************************************************
** Function name        :   vMemBlockNoWaitGet
** Descriptions         :   ��ȡ�洢�飬���û�д洢�飬�������˻�
** parameters           :   memBlock �ȴ��Ĵ洢��ṹ
** parameters			:   mem �洢��
** Returned value       :   ��ȡ���, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vMemBlockNoWaitGet(vMemBlock *memBlock, uint8_t **mem)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ���ȼ���Ƿ��п��еĴ洢��
    if (vListCount(&memBlock->blockList) > 0)
    {
        // ����еĻ���ȡ��һ��
        *mem = (uint8_t *)vListRemoveFirst(&memBlock->blockList);
        
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
** Function name        :   vMemBlockNotify
** Descriptions         :   ֪ͨ�洢����ã����ѵȴ������е�һ�����񣬻��߽��洢����������
** parameters           :   memBlock �����Ĵ洢��ṹ
** parameters			:   mem �洢��
** Returned value       :   ��
***********************************************************************************************************/
void vMemBlockNotify(vMemBlock *memBlock, uint8_t *mem)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ����Ƿ�������ȴ�
    if (vEventWaitCount(&memBlock->event) > 0)
    {
        // ����еĻ�����ֱ�ӻ���λ�ڶ����ײ������ȵȴ���������
        vTask *task = vEventWakeupFirst(&memBlock->event, (void *)mem, vErrorNoError);
        
        // ��������������ȼ����ߣ���ִ�е��ȣ��л���ȥ
        if (task->prio < currentTask->prio)
        {
            vTaskSched();
        }
    }
    else
    {
        // ���û������ȴ��Ļ������洢����뵽������
        vListAddLast(&memBlock->blockList, (vNode *)mem);
    }
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vMemBlockGetInfo
** Descriptions         :   ��ѯ�洢���ƿ��״̬��Ϣ
** parameters           :   memBlock �洢��ṹ
** parameters           :   info ״̬��ѯ�洢��λ��
** Returned value       :   ��
***********************************************************************************************************/
void vMemBlockGetInfo(vMemBlock *memBlock, vMemBlockInfo *info)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    info->count = vListCount(&memBlock->blockList);
    info->maxCount = memBlock->maxCount;
    info->blockSize = memBlock->blockSize;
    info->taskCount = vEventWaitCount(&memBlock->event);
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
}

/**********************************************************************************************************
** Function name        :   vMemBlockDestroy
** Descriptions         :   ���ٴ洢���ƿ�
** parameters           :   memBlock ��Ҫ���ٵĴ洢��ṹ
** Returned value       :   �����ٸô洢���ƿ�����ѵ���������
***********************************************************************************************************/
uint32_t vMemBlockDestroy(vMemBlock *memblock)
{
    // �����ٽ���
    uint32_t status = vTastCriticalEnter();
    
    // ����¼����ƿ��е�����
    uint32_t count = vEventWakeupAll(&memblock->event, (void *)0, vErrorDel);
    
    // �˳��ٽ���
    vTaskCriticalExit(status);
    
    if (count > 0)
    {
        vTaskSched();
    }
    
    return count;
}

#endif
