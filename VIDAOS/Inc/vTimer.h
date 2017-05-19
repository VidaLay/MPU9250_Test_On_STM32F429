#ifndef _VTIMER_H
#define _VTIMER_H

#include "vEvent.h"

typedef enum _vTimerState
{
    vTimerCreated,          // ��ʱ���Ѿ�����
    vTimerStarted,          // ��ʱ���Ѿ�����
    vTimerFuncRunning,          // ��ʱ������ִ�лص�����
    vTimerStopped,          // ��ʱ���Ѿ�ֹͣ
    vTimerDestroyed,        // ��ʱ���Ѿ�����
}vTimerState;

typedef struct _vTimer
{
    // ������
    vNode linkNode;
    
    // ���������Ӻ��ticks��
    uint32_t startDelayTicks;
    
    // ���ڶ�ʱʱ������tick��
    uint32_t durationTicks;
    
    // ��ǰ��ʱ�ݼ�����ֵ
    uint32_t delayTicks;
    
    // ��ʱ�ص�����
    void (*timerFunc)(void *arg);
    
    // ���ݸ��ص������Ĳ���
    void *arg;
    
    // ��ʱ�����ò���
    uint32_t config;
    
    // ��ʱ��״̬
    vTimerState state;
}vTimer;

typedef struct _vTimerInfo
{
    // ���������Ӻ��ticks��
    uint32_t startDelayTicks;
    
    // ���ڶ�ʱʱ������tick��
    uint32_t durationTicks;
    
    // ��ʱ�ص�����
    void (*timerFunc)(void *arg);
    
    // ���ݸ��ص������Ĳ���
    void *arg;
    
    // ��ʱ�����ò���
    uint32_t config;
    
    // ��ʱ��״̬
    vTimerState state;
}vTimerInfo;

#define TIMER_CONFIG_TYPE_HARD          (1 << 0)
#define TIMER_CONFIG_TYPE_SOFT          (0 << 0)

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
void vTimerInit(vTimer *timer, uint32_t delayTicks, uint32_t durationTicks, void (*timerFunc)(void *arg), void *arg, uint32_t config);

/**********************************************************************************************************
** Function name        :   vTimerStart
** Descriptions         :   ������ʱ��
** parameters           :   timer �ȴ������Ķ�ʱ��
** Returned value       :   ��
***********************************************************************************************************/
void vTimerStart(vTimer *timer);

/**********************************************************************************************************
** Function name        :   vTimerStop
** Descriptions         :   ��ֹ��ʱ��
** parameters           :   timer �ȴ������Ķ�ʱ��
** Returned value       :   ��
***********************************************************************************************************/
void vTimerStop(vTimer *timer);

/**********************************************************************************************************
** Function name        :   vTimerModuleTickNotify
** Descriptions         :   ֪ͨ��ʱģ�飬ϵͳ����tick����
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vTimerModuleTickNotify(void);

/**********************************************************************************************************
** Function name        :   vTimerModuleInit
** Descriptions         :   ��ʱ��ģ���ʼ��
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vTimerModuleInit(void);

/**********************************************************************************************************
** Function name        :   vTimerTaskInit
** Descriptions         :   ��ʼ����ʱ������
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vTimerTaskInit(void);

/**********************************************************************************************************
** Function name        :   vTimerDestroy
** Descriptions         :   ���ٶ�ʱ��
** parameters           :   timer ���ٵĶ�ʱ��
** Returned value       :   ��
***********************************************************************************************************/
void vTimerDestroy (vTimer * timer);

/**********************************************************************************************************
** Function name        :   vTimerGetInfo
** Descriptions         :   ��ѯ״̬��Ϣ
** parameters           :   timer ��ѯ�Ķ�ʱ��
** parameters           :   info ״̬��ѯ�洢��λ��
** Returned value       :   ��
***********************************************************************************************************/
void vTimerGetInfo (vTimer * timer, vTimerInfo * info);


#endif
