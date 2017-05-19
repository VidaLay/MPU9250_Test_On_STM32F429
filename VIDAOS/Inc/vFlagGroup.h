#ifndef _VFLAGGROUP_H
#define _VFLAGGROUP_H

#include "vEvent.h"

typedef struct _vFlagGroup
{
    // �¼����ƿ�
    vEvent event;
    
    // ��ǰ�¼���־
    uint32_t flags;
}vFlagGroup;

typedef struct _vFlagGroupInfo
{
    // ��ǰ���¼���־
    uint32_t flags;
    
    // ��ǰ�ȴ����������
    uint32_t taskCount;
}vFlagGroupInfo;

#define VFLAGGROUP_CLEAR            (0x0 << 0)
#define VFLAGGROUP_SET              (0x1 << 0)
#define VFLAGGROUP_ANY              (0x0 << 1)
#define VFLAGGROUP_ALL              (0x1 << 1)

#define VFLAGGROUP_SET_ALL          (VFLAGGROUP_SET | VFLAGGROUP_ALL)
#define VFLAGGROUP_SET_ANY          (VFLAGGROUP_SET | VFLAGGROUP_ANY)
#define VFLAGGROUP_CLEAR_ALL        (VFLAGGROUP_CLEAR | VFLAGGROUP_ALL)
#define VFLAGGROUP_CLEAR_ANY        (VFLAGGROUP_CLEAR | VFLAGGROUP_ANY)

#define VFLAGGROUP_CONSUME          (1 << 7)

/**********************************************************************************************************
** Function name        :   vFlagGroupInit
** Descriptions         :   ��ʼ���¼���־��
** parameters           :   flagGroup �ȴ���ʼ�����¼���־��
** parameters           :   flags ��ʼ���¼���־
** Returned value       :   ��
***********************************************************************************************************/
void vFlagGroupInit(vFlagGroup * flagGroup, uint32_t flags);

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
uint32_t vFlagGroupWait(vFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag, uint32_t waitTicks);

/**********************************************************************************************************
** Function name        :   vFlagGroupNoWaitGet
** Descriptions         :   ��ȡ�¼���־�����ض��ı�־
** parameters           :   flagGroup ��ȡ���¼���־��
** parameters           :   waitType ��ȡ���¼�����
** parameters           :   requstFlag ������¼���־
** parameters           :   resultFlag �ȴ���־���
** Returned value       :   ��ȡ���, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vFlagGroupNoWaitGet(vFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag);

/**********************************************************************************************************
** Function name        :   vFlagGroupNotify
** Descriptions         :   ֪ͨ�¼���־���е��������µı�־����
** parameters           :   flagGroup �¼���־��
** parameters           :   isSet �Ƿ�����1�¼�
** parameters           :   flags �������¼���־
***********************************************************************************************************/
void vFlagGroupNotify(vFlagGroup *flagGroup, uint8_t isSet, uint32_t flags);

/**********************************************************************************************************
** Function name        :   vFlagGroupGetInfo
** Descriptions         :   ��ѯ�¼���־���״̬��Ϣ
** parameters           :   flagGroup �¼���־��
** parameters           :   info ״̬��ѯ�洢��λ��
** Returned value       :   ��
***********************************************************************************************************/
void vFlagGroupGetInfo(vFlagGroup *flagGroup, vFlagGroupInfo * info);

/**********************************************************************************************************
** Function name        :   vFlagGroupDestroy
** Descriptions         :   �����¼���־��
** parameters           :   flagGroup �¼���־��
** Returned value       :   �����ٸô洢���ƿ�����ѵ���������
***********************************************************************************************************/
uint32_t vFlagGroupDestroy(vFlagGroup *flagGroup);


#endif
