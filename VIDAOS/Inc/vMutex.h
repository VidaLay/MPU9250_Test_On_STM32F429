#ifndef _VMUTEX_H
#define _VMUTEX_H

#include "vEvent.h"

typedef struct _vMutex
{
    // �¼����ƿ�
    vEvent event;
    
    // �ѱ������Ĵ���
    uint32_t lockedCount;
    
    // ӵ����
    vTask *owner;
    
    // ӵ����ԭʼ�����ȼ�
    uint32_t ownerOriginalPrio;
}vMutex;

typedef struct _vMutexInfo
{
    // �ȴ�����������
    uint32_t taskCount;
    
    // ӵ������������ȼ�
    uint32_t ownerPrio;
    
    // �̳����ȼ�
    uint32_t inheritedPrio;
    
    // ��ǰ�ź�����ӵ����
    vTask *owner;
    
    // ��������
    uint32_t lockedCount;
}vMutexInfo;

/**********************************************************************************************************
** Function name        :   vMutexInit
** Descriptions         :   ��ʼ�������ź���
** parameters           :   mutex �ȴ���ʼ���Ļ����ź���
** Returned value       :   ��
***********************************************************************************************************/
void vMutexInit(vMutex *mutex);

/**********************************************************************************************************
** Function name        :   vMutexWait
** Descriptions         :   �ȴ��ź���
** parameters           :   mutex �ȴ����ź���
** parameters           :   waitTicks ���ȴ���ticks����Ϊ0��ʾ���޵ȴ�
** Returned value       :   �ȴ����, vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vMutexWait(vMutex *mutex, uint32_t waitTicks);

/**********************************************************************************************************
** Function name        :   vMutexNoWaitGet
** Descriptions         :   ��ȡ�ź���������Ѿ�����������������
** parameters           :   mutex �ȴ����ź���
** Returned value       :   ��ȡ���, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vMutexNoWaitGet(vMutex *mutex);

/**********************************************************************************************************
** Function name        :   vMutexNotify
** Descriptions         :   ֪ͨ�����ź�������
** parameters           :   mbox �������ź���
** parameters           :   msg ���͵���Ϣ
** parameters           :   notifyOption ���͵�ѡ��
** Returned value       :   vErrorResourceFull
***********************************************************************************************************/
uint32_t vMutexNotify(vMutex *mutex);

/**********************************************************************************************************
** Function name        :   vMutexDestroy
** Descriptions         :   �����ź���
** parameters           :   mutex �����ٻ��ĳ��ź���
** Returned value       :   �����ٸ��ź��������ѵ���������
***********************************************************************************************************/
uint32_t vMutexDestroy(vMutex *mutex);

/**********************************************************************************************************
** Function name        :   vMutexGetInfo
** Descriptions         :   ��ѯ״̬��Ϣ
** parameters           :   mutex ��ѯ�Ļ����ź���
** parameters           :   info ״̬��ѯ�洢��λ��
** Returned value       :   ��
***********************************************************************************************************/
void vMutexGetInfo(vMutex *mutex, vMutexInfo *info);

#endif
