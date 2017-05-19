#ifndef _VMEMBLOCK_H
#define _VMEMBLOCK_H

#include "vEvent.h"

typedef struct _vMemBlock
{
    // �¼����ƿ�
    vEvent event;
    
    // �洢����׵�ַ
    void *memstart;
    
    // ÿ���洢��Ĵ�С
    uint32_t blockSize;
    
    // �ܵĴ洢��ĸ���
    uint32_t maxCount;
    
    // �洢���б�
    vList blockList;
}vMemBlock;

typedef struct _vMemBlockInfo
{
    // ��ǰ�洢��ļ���
    uint32_t count;
    
    // �����������
    uint32_t maxCount;
    
    // ÿ���洢��Ĵ�С
    uint32_t blockSize;
    
    // ��ǰ�ȴ����������
    uint32_t taskCount;
}vMemBlockInfo;

/**********************************************************************************************************
** Function name        :   vMemBlockInit
** Descriptions         :   ��ʼ���洢���ƿ�
** parameters           :   memBlock �ȴ���ʼ���Ĵ洢��ṹ
** parameters           :   memStart �洢������ʼ��ַ
** parameters           :   blockSize ÿ����Ĵ�С
** parameters           :   blockCnt �ܵĿ�����
** Returned value       :   ���ѵ���������
***********************************************************************************************************/
void vMemBlockInit(vMemBlock *memBlock, uint8_t *memStart, uint32_t blockSize, uint32_t blockCnt);

/**********************************************************************************************************
** Function name        :   vMemBlockWait
** Descriptions         :   �ȴ��洢��
** parameters           :   memBlock �ȴ��Ĵ洢��ṹ
** parameters			:   mem �洢��
** parameters           :   waitTicks ��û�д洢��ʱ���ȴ���ticks����Ϊ0ʱ��ʾ��Զ�ȴ�
** Returned value       :   �ȴ����, vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vMemBlockWait(vMemBlock *memBlock, uint8_t **mem, uint32_t waitTicks);

/**********************************************************************************************************
** Function name        :   vMemBlockNoWaitGet
** Descriptions         :   ��ȡ�洢�飬���û�д洢�飬�������˻�
** parameters           :   memBlock �ȴ��Ĵ洢��ṹ
** parameters			:   mem �洢��
** Returned value       :   ��ȡ���, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vMemBlockNoWaitGet(vMemBlock *memBlock, uint8_t **mem);

/**********************************************************************************************************
** Function name        :   vMemBlockNotify
** Descriptions         :   ֪ͨ�洢����ã����ѵȴ������е�һ�����񣬻��߽��洢����������
** parameters           :   memBlock �����Ĵ洢��ṹ
** parameters			:   mem �洢��
** Returned value       :   ��
***********************************************************************************************************/
void vMemBlockNotify(vMemBlock *memBlock, uint8_t *mem);

/**********************************************************************************************************
** Function name        :   vMemBlockGetInfo
** Descriptions         :   ��ѯ�洢���ƿ��״̬��Ϣ
** parameters           :   memBlock �洢��ṹ
** parameters           :   info ״̬��ѯ�洢��λ��
** Returned value       :   ��
***********************************************************************************************************/
void vMemBlockGetInfo(vMemBlock *memBlock, vMemBlockInfo *info);

/**********************************************************************************************************
** Function name        :   vMemBlockDestroy
** Descriptions         :   ���ٴ洢���ƿ�
** parameters           :   memBlock ��Ҫ���ٵĴ洢��ṹ
** Returned value       :   �����ٸô洢���ƿ�����ѵ���������
***********************************************************************************************************/
uint32_t vMemBlockDestroy(vMemBlock *memblock);


#endif
