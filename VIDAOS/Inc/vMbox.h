#ifndef _VMBOX_H
#define _VMBOX_H

#include "vEvent.h"

#define vMboxStoreNormal         0x00        // �������ͷ�����������β��
#define vMboxStoreFront          0x01        // ��Ϣ������������ͷ��    
#define vMboxSentToAll           0x02        // ��Ϣ���͸����еȴ�����

// ��������
typedef struct _vMbox
{
    // �¼����ƿ�
	// �ýṹ������ŵ���ʼ������ʵ��tSemͬʱ��һ��tEvent��Ŀ��
    vEvent event;
    
    // ��ǰ����Ϣ����
    uint32_t count;
    
    // ��ȡ��Ϣ������
    uint32_t read;
    
    // д��Ϣ������
    uint32_t write;
    
    // ����������ɵ���Ϣ����
    uint32_t maxCount;
    
    // ��Ϣ�洢������
    void **msgBuffer;
}vMbox;

// ����״̬����
typedef struct _vMboxInfo
{
    // ��ǰ����Ϣ����
    uint32_t count;
    
    // ����������ɵ���Ϣ����
    uint32_t maxCount;
    
    // ��ǰ�ȴ����������
    uint32_t taskCount;
}vMboxInfo;

/**********************************************************************************************************
** Function name        :   vMboxInit
** Descriptions         :   ��ʼ������
** parameters           :   mbox �ȴ���ʼ��������
** parameters           :   msgBuffer ��Ϣ�洢������
** parameters           :   maxCount ������
** Returned value       :   ��
***********************************************************************************************************/
void vMboxInit(vMbox *mbox, void **msgBuffer, uint32_t maxCount);

/**********************************************************************************************************
** Function name        :   vMboxWait
** Descriptions         :   �ȴ�����, ��ȡһ����Ϣ
** parameters           :   mbox �ȴ�������
** parameters           :   msg ��Ϣ�洢������
** parameters           :   waitTicks ���ȴ���ticks����Ϊ0��ʾ���޵ȴ�
** Returned value       :   �ȴ����,vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vMboxWait(vMbox *mbox, void **msg, uint32_t waitTicks);

/**********************************************************************************************************
** Function name        :   vMboxNoWaitGet
** Descriptions         :   ��ȡһ����Ϣ�����û����Ϣ���������˻�
** parameters           :   mbox ��ȡ��Ϣ������
** parameters           :   msg ��Ϣ�洢������
** Returned value       :   ��ȡ���, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vMboxNoWaitGet(vMbox *mbox, void **msg);

/**********************************************************************************************************
** Function name        :   vMboxNotify
** Descriptions         :   ֪ͨ��Ϣ���ã����ѵȴ������е�һ�����񣬻��߽���Ϣ���뵽������
** parameters           :   mbox �������ź���
** parameters           :   msg ���͵���Ϣ
** parameters           :   notifyOption ���͵�ѡ�� vMboxStoreNormal/vMboxStoreFront/vMboxSentToAll
** Returned value       :   vErrorNoError/ vErrorResourceFull
***********************************************************************************************************/
uint32_t vMboxNotify(vMbox *mbox, void *msg, uint32_t notifyOption);

/**********************************************************************************************************
** Function name        :   vMboxFlush
** Descriptions         :   ���������������Ϣ
** parameters           :   mbox �ȴ���յ�����
** Returned value       :   ��
***********************************************************************************************************/
void vMboxFlush(vMbox *mbox);

/**********************************************************************************************************
** Function name        :   vMboxDestroy
** Descriptions         :   ��������
** parameters           :   mbox ��Ҫ���ٵ�����
** Returned value       :   �����ٸ��ź��������ѵ���������
***********************************************************************************************************/
uint32_t vMboxDestroy(vMbox *mbox);

/**********************************************************************************************************
** Function name        :   vMboxGetInfo
** Descriptions         :   ��ѯ״̬��Ϣ
** parameters           :   mbox ��ѯ������
** parameters           :   info ״̬��ѯ�洢��λ��
** Returned value       :   ��
***********************************************************************************************************/
void vMboxGetInfo(vMbox * mbox, vMboxInfo *info);

#endif
