#ifndef _VIDAOS_H
#define _VIDAOS_H

// ÿ����SysTick�����жϴ���
#define TICKS_PER_SEC           (1000 / VIDAOS_SYSTICK_MS)

// ��׼ͷ�ļ�����������˳��õ����Ͷ��壬��uint32_t
#include <stdint.h>

// VidaOS���ں˿��ļ�
#include "vLib.h"

// VidaOS�������ļ�
#include "vConfig.h"

// �������ͷ�ļ�
#include "vTask.h"

// �������ͷ�ļ�
#include "vSched.h"

// ������ʱ����ͷ�ļ�
#include "vDelay.h"

// �¼�����ͷ�ļ�
#include "vEvent.h"

// �ź���ͷ�ļ�
#include "vSem.h"

// ����ͷ�ļ�
#include "vMbox.h"

// �洢��ͷ�ļ�
#include "vMemBlock.h"

// �¼���־��ͷ�ļ�
#include "vFlagGroup.h"

// �����ź���ͷ�ļ�
#include "vMutex.h"

// ��ʱ��
#include "vTimer.h"

// Hooks��չ
#include "vHooks.h"

typedef enum _vError
{
    vErrorNoError = 0,                  // û�д���
    vErrorTimeout,                      // �ȴ���ʱ
    vErrorResourceUnavaliable,          // ��Դ������
    vErrorDel,                          // ��ɾ��
    vErrorResourceFull,                 // ��Դ��������
    vErrorOwner,                        // ��ƥ���������
}vError;

typedef enum _vOSState
{
    vStopped = 0,                       // OSֹͣ
    vRunning,                           // OS����
}vOSState;

// OS״̬
extern uint8_t vOS_State;

// ��ǰ���񣺼�¼��ǰ���ĸ�������������
extern vTask *currentTask;

// ��һ���������е������ڽ��������л�ǰ�������úø�ֵ��Ȼ�������л������л���ж�ȡ��һ������Ϣ
extern vTask *nextTask;

// ���������
extern vList OSRdyTable[VIDAOS_PRO_COUNT];

// ������ʱ����
extern vList vTaskDelayedList;

/**********************************************************************************************************
** Function name        :   vTastCriticalEnter
** Descriptions         :   �����ٽ���
** parameters           :   ��
** Returned value       :   ����֮ǰ���ٽ���״ֵ̬
***********************************************************************************************************/
uint32_t vTastCriticalEnter(void);

/**********************************************************************************************************
** Function name        :   vTaskCriticalExit
** Descriptions         :   �˳��ٽ���,�ָ�֮ǰ���ٽ���״̬
** parameters           :   status �����ٽ���֮ǰ��CPU
** Returned value       :   �����ٽ���֮ǰ���ٽ���״ֵ̬
***********************************************************************************************************/
void vTaskCriticalExit(uint32_t status);

/**********************************************************************************************************
** Function name        :   vOSStart
** Descriptions         :   ����ϵͳ
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vOSStart(void);

/**********************************************************************************************************
** Function name        :   vTaskSwitch
** Descriptions         :   ����һ�������л���VidaOS��Ԥ�����ú�currentTask��nextTask, Ȼ����øú������л���
**                          nextTask����
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vTaskSwitch(void);

/**********************************************************************************************************
** Function name        :   vTaskSystemTickHandler
** Descriptions         :   ϵͳʱ�ӽ��Ĵ���
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vTaskSystemTickHandler(void);

/**********************************************************************************************************
** Function name        :   vSetSysTickPeriod
** Descriptions         :   �趨SysTick�ж����ڡ�
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vSetSysTickPeriod(uint32_t ms);

/**********************************************************************************************************
** Function name        :   vInitApp
** Descriptions         :   ��ʼ��Ӧ�ýӿ�
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vInitApp(void);

/**********************************************************************************************************
** Function name        :   vCPUUsageGet
** Descriptions         :   ��õ�ǰ����CPUռ����
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
float vCPUUsageGet(void);

#endif
