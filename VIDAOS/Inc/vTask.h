#ifndef _VTASK_H
#define _VTASK_H

#define VIDAOS_TASK_STATE_RDY       	1
#define VIDAOS_TASK_STATE_DESTROYED		(1 << 1)
#define VIDAOS_TASK_STATE_DELAYED   	(1 << 2)
#define VIDAOS_TASK_STATE_SUSPEND		(1 << 3)

#define VIDAOS_TASK_WAIT_MASK           (0xFF << 16)


// ǰ������
struct _vEvent;

// Cortex-M�Ķ�ջ��Ԫ���ͣ���ջ��Ԫ�Ĵ�СΪ32λ������ʹ��uint32_t
typedef uint32_t vTaskStack;

// ����ṹ��������һ�������������Ϣ
typedef struct _vTask
{
    // �������ö�ջ�ĵ�ǰ��ջָ�롣ÿ�����������Լ��Ķ�ջ�����������й����д洢��ʱ������һЩ��������
	// ��VidaOS���и�����ǰ�����stackָ���λ�ô������ȡ��ջ�еĻ��������ָ���CPU�Ĵ����У�Ȼ��ʼ����
	// ���л�����������ʱ���Ὣ��ǰCPU�Ĵ���ֵ���浽��ջ�У��ȴ���һ�����и�����ʱ�ٻָ���
	// stack��������󱣴滷�������ĵ�ַλ�ã����ں����ָ�
    vTaskStack *stack;
    
    // ��ջ���𼴵�ַ
    uint32_t *stackBase;
    
    // ��ջ��������
    uint32_t stackSize;
    
    // ��������ȼ�
    uint8_t prio;
    
    // ��ǰʣ���ʱ��Ƭ
    uint32_t slice;
    
    // ����ǰ״̬
    uint32_t state;
    
    // ������Ĵ���
	uint32_t suspendCount;
    
    // ������ʱ������
    uint32_t delayTicks;
    
    // ���ӽ��
	vNode linkNode;
    
    // ��ʱ��㣺ͨ��delayNode�Ϳ��Խ�tTask���õ���ʱ������
    vNode delayNode;
    
    // ����ɾ��ʱ���õ�������
	void (*clean)(void *param);
    
    // ���ݸ��������Ĳ���
	void *cleanParam;
    
    // ����ɾ����־����0��ʾ����ɾ��
	uint8_t requestDeleteFlag;
    
    // �������ڵȴ����¼�����
    struct _vEvent *waitEvent;
    
    // �ȴ��¼�����Ϣ�洢λ��
    void *eventMsg;
    
    // �ȴ��¼��Ľ�� vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel/ vErrorResourceFull/ vErrorOwner
    uint32_t waitEventResult;
    
    // �ȴ����¼���ʽ
    uint32_t waitFlagType;
    
    // �ȴ����¼���־
    uint32_t requestEventFlags;
    
    // ��ջ��С������
    uint32_t stackFreeMin;
}vTask;

// ���������Ϣ�ṹ
typedef struct _vTaskInfo
{
    // ������ʱ������
	uint32_t delayTicks;
    
    // ��������ȼ�
	uint8_t prio;
    
    // ����ǰ״̬
	uint32_t state;
    
    // ��ǰʣ���ʱ��Ƭ
	uint32_t slice;
    
    // ������Ĵ���
	uint32_t suspendCount;
    
    // ��ջ��������
    uint32_t stackSize;
    
    // ��ջ��ǰ������
    uint32_t stackFreeRT;
    
    // ��ջ��С������
    uint32_t stackFreeMin;
}vTaskInfo;

/**********************************************************************************************************
** Function name        :   vTaskInit
** Descriptions         :   ��ʼ������ṹ
** parameters           :   task        Ҫ��ʼ��������ṹ
** parameters           :   entry       �������ں���
** parameters           :   param       ���ݸ���������в���
** Returned value       :   ��
***********************************************************************************************************/
void vTaskInit (vTask * task, void (*entry)(void *), void *param, uint8_t prio, vTaskStack *stackBase, uint32_t size);

/**********************************************************************************************************
** Function name        :   vTaskSuspend
** Descriptions         :   ����ָ��������
** parameters           :   task        �����������
** Returned value       :   ��
***********************************************************************************************************/
void vTaskSuspend(vTask *task);

/**********************************************************************************************************
** Function name        :   vTaskWakeup
** Descriptions         :   ���ѱ����������
** parameters           :   task        �����ѵ�����
** Returned value       :   ��
***********************************************************************************************************/
void vTaskWakeup(vTask *task);

/**********************************************************************************************************
** Function name        :   vTaskSetCleanCallFunc
** Descriptions         :   ��������ɾ��ʱ���õ�������
** parameters           :   task  �����õ�����
** parameters           :   clean  ��������ڵ�ַ
** parameters           :   param  ���ݸ��������Ĳ���
** Returned value       :   ��
***********************************************************************************************************/
void vTaskSetCleanCallFunc(vTask *task, void (*clean)(void * param), void *param);

/**********************************************************************************************************
** Function name        :   vTaskForceDelete
** Descriptions         :   ǿ��ɾ��ָ��������
** parameters           :   task  ��Ҫɾ��������
** Returned value       :   ��
***********************************************************************************************************/
void vTaskForceDelete(vTask *task);

/**********************************************************************************************************
** Function name        :   vTaskRequestDelete
** Descriptions         :   ����ɾ��ĳ�������������Լ������Ƿ�ɾ���Լ�
** parameters           :   task  ��Ҫɾ��������
** Returned value       :   ��
***********************************************************************************************************/
void vTaskRequestDelete(vTask *task);

/**********************************************************************************************************
** Function name        :   vTaskIsRequestedDelete
** Descriptions         :   �Ƿ��Ѿ�������ɾ���Լ�
** parameters           :   ��
** Returned value       :   ��0��ʾ����ɾ����0��ʾ������
***********************************************************************************************************/
uint8_t vTaskIsRequestedDelete(void);

/**********************************************************************************************************
** Function name        :   vTaskDeleteSelf
** Descriptions         :   ɾ���Լ�
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vTaskDeleteSelf(void);

/**********************************************************************************************************
** Function name        :   vTaskGetInfo
** Descriptions         :   ��ȡ���������Ϣ
** parameters           :   task ��Ҫ��ѯ������
** parameters           :   info ������Ϣ�洢�ṹ
** Returned value       :   ��
***********************************************************************************************************/
void vTaskGetInfo(vTask *task, vTaskInfo *info);

#endif
