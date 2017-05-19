#ifndef _VSEM_H
#define _VSEM_H

#include "vEvent.h"

// �ź�������
typedef struct _vSem
{
    // �¼����ƿ�
	// �ýṹ������ŵ���ʼ������ʵ��tSemͬʱ��һ��tEvent��Ŀ��
    vEvent event;
    
    // ��ǰ�ļ���
    uint32_t count;
    
    // ������
    uint32_t maxCount;
}vSem;

// �ź�������Ϣ����
typedef struct _vSemInfo
{
    // ��ǰ�ź����ļ���
    uint32_t count;
    
    // �ź��������������
    uint32_t maxCount;
    
    // ��ǰ�ȴ����������
    uint32_t taskCount;
}vSemInfo;

/**********************************************************************************************************
** Function name        :   vSemInit
** Descriptions         :   ��ʼ���ź���
** parameters           :   startCount ��ʼ�ļ���
** parameters           :   maxCount �����������Ϊ0����������
** Returned value       :   ��
***********************************************************************************************************/
void vSemInit(vSem *sem, uint32_t startCount, uint32_t maxCount);

/**********************************************************************************************************
** Function name        :   vSemWait
** Descriptions         :   �ȴ��ź���
** parameters           :   sem �ȴ����ź���
** parameters           :   waitTicks ���ź�������Ϊ0ʱ���ȴ���ticks����Ϊ0ʱ��ʾ��Զ�ȴ�
** Returned value       :   �ȴ����, vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vSemWait(vSem *sem, uint32_t waitTicks);

/**********************************************************************************************************
** Function name        :   vSemNoWaitGet
** Descriptions         :   ��ȡ�ź���������ź������������ã��������˻�
** parameters           :   sem �ȴ����ź���
** Returned value       :   ��ȡ���, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vSemNoWaitGet(vSem *sem);

/**********************************************************************************************************
** Function name        :   vSemNotify
** Descriptions         :   ֪ͨ�ź������ã����ѵȴ������е�һ�����񣬻��߽�����+1
** parameters           :   sem �������ź���
** Returned value       :   ��
***********************************************************************************************************/
void vSemNotify(vSem *sem);

/**********************************************************************************************************
** Function name        :   vSemGetInfo
** Descriptions         :   ��ѯ�ź�����״̬��Ϣ
** parameters           :   sem ��ѯ���ź���
** parameters           :   info ״̬��ѯ�洢��λ��
** Returned value       :   ��
***********************************************************************************************************/
void vSemGetInfo(vSem *sem, vSemInfo *info);

/**********************************************************************************************************
** Function name        :   vSemDestroy
** Descriptions         :   �����ź���
** parameters           :   sem ��Ҫ���ٵ��ź���
** Returned value       :   �����ٸ��ź��������ѵ���������
***********************************************************************************************************/
uint32_t vSemDestroy(vSem *sem);

#endif
