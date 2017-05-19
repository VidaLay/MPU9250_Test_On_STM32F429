#ifndef _VEVENT_H
#define _VEVENT_H

#include "vLib.h"
#include "vTask.h"

// Event����
typedef enum _vEventType
{
    vEventTypeUnknown   = (1 << 16), 				// δ֪����
    vEventTypeSem   	= (2 << 16), 				// �ź�������
    vEventTypeMbox  	= (3 << 16), 				// ��������
	vEventTypeMemBlock  = (4 << 16),				// �洢������
	vEventTypeFlagGroup = (5 << 16),				// �¼���־��
	vEventTypeMutex     = (6 << 16),				// �����ź�������
}vEventType;

// Event���ƽṹ
typedef struct _vEvent
{
    vEventType type;						// Event����

    vList waitList;							// ����ȴ��б�
}vEvent;

/**********************************************************************************************************
** Function name        :   vEventInit
** Descriptions         :   ��ʼ���¼����ƿ�
** parameters           :   event �¼����ƿ�
** parameters           :   type �¼����ƿ������
** Returned value       :   ��
***********************************************************************************************************/
void vEventInit(vEvent *event, vEventType type);

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
void vEventWait(vEvent *event, vTask *task, void *msg, uint32_t state, uint32_t timeout);

/**********************************************************************************************************
** Function name        :   vEventWakeupFirst
** Descriptions         :   ���¼����ƿ��л����׸��ȴ�������
** parameters           :   event �¼����ƿ�
** parameters           :   msg �¼���Ϣ
** parameters           :   result ��֪�¼��ĵȴ����
** Returned value       :   �׸��ȴ����������û������ȴ����򷵻�0
***********************************************************************************************************/
vTask *vEventWakeupFirst(vEvent *event, void *msg, uint32_t result);

/**********************************************************************************************************
** Function name        :   vEventWakeup
** Descriptions         :   ���¼����ƿ��л���ָ������
** parameters           :   event �¼����ƿ�
** parameters           :   task �ȴ����ѵ�����
** parameters           :   msg �¼���Ϣ
** parameters           :   result ��֪�¼��ĵȴ����
** Returned value       :   �׸��ȴ����������û������ȴ����򷵻�0
***********************************************************************************************************/
void vEventWakeup(vEvent *event, vTask *task, void *msg, uint32_t result);

/**********************************************************************************************************
** Function name        :   vEventRemoveTask
** Descriptions         :   ���������ȴ�������ǿ���Ƴ�
** parameters           :   task ���Ƴ�������
** parameters           :   result ��֪�¼��ĵȴ����
** Returned value       :   ��
***********************************************************************************************************/
void vEventRemoveTask(vTask *task, void *msg, uint32_t result);

/**********************************************************************************************************
** Function name        :   vEventWakeupAll
** Descriptions         :   ������еȴ��е����񣬽��¼����͸���������
** parameters           :   event �¼����ƿ�
** parameters           :   msg �¼���Ϣ
** parameters           :   result ��֪�¼��ĵȴ����
** Returned value       :   ���ѵ���������
***********************************************************************************************************/
uint32_t vEventWakeupAll(vEvent *event, void *msg, uint32_t result);

/**********************************************************************************************************
** Function name        :   vEventWaitCount
** Descriptions         :   �¼����ƿ��еȴ�����������
** parameters           :   event �¼����ƿ�
** parameters           :   msg �¼���Ϣ
** parameters           :   result ��֪�¼��ĵȴ����
** Returned value       :   ���ѵ���������
***********************************************************************************************************/
uint32_t vEventWaitCount(vEvent *event);

#endif
