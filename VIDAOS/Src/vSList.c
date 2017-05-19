#include "vLib.h"

#define firstNode headNode.nextNode

/**********************************************************************************************************
** Function name        :   vSNodeInit
** Descriptions         :   ��ʼ������������
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vSNodeInit(vSNode *sNode)
{
    sNode->nextNode = sNode;
}

/**********************************************************************************************************
** Function name        :   vSListInit
** Descriptions         :   ���������ʼ��
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vSListInit(vSList *sList)
{
    sList->firstNode = &(sList->headNode);
    sList->nodeCount = 0;
}

/**********************************************************************************************************
** Function name        :   vSListCount
** Descriptions         :   ���ص��������н�������
** parameters           :   ��
** Returned value       :   �������
***********************************************************************************************************/
uint32_t vSListCount(vSList *sList)
{
    return sList->nodeCount;
}

/**********************************************************************************************************
** Function name        :   vSListFirst
** Descriptions         :   ���ص���������׸����
** parameters           :   sList ��ѯ������
** Returned value       :   �׸���㣬�������Ϊ�գ��򷵻�0
***********************************************************************************************************/
vSNode *vSListFirst(vSList *sList)
{
    vSNode *sNode = (vSNode *)0;
    
    if (sList->nodeCount != 0)
    {
        sNode = sList->firstNode;
    }
    return sNode;
}

/**********************************************************************************************************
** Function name        :   vListNext
** Descriptions         :   ���ص���������ָ�����ĺ�һ���
** parameters           :   sList ��ѯ������
** parameters           :   sNode �ο����
** Returned value       :   ��һ����㣬������û��ǰ��㣨����Ϊ�գ����򷵻�0
***********************************************************************************************************/
vSNode *vSListNext(vSList *sList, vSNode *sNode)
{ 
    if (sNode->nextNode == sNode)
    {
        return (vSNode *)0;
    }
    else
    {
        return sNode->nextNode;
    }
}

/**********************************************************************************************************
** Function name        :   vSListRemoveAll
** Descriptions         :   �Ƴ����������е����н��
** parameters           :   sList ����յ�����
** Returned value       :   ��
***********************************************************************************************************/
void vSListRemoveAll(vSList *sList)
{
    uint32_t count;
    vSNode *nextNode;
    
    nextNode = sList->firstNode;
    
    for (count = sList->nodeCount; count !=0; count--)
    {
        vSNode *currentNode = nextNode;
        nextNode = nextNode->nextNode;
        
        vSNodeInit(currentNode);
    }
    
    vSListInit(sList);
}

/**********************************************************************************************************
** Function name        :   vSListAddFirst
** Descriptions         :   ��ָ�������ӵ����������ͷ��
** parameters           :   sList ����������
** parameters			:   sNode ������Ľ��
** Returned value       :   ��
***********************************************************************************************************/
void vSListAddFirst(vSList *sList, vSNode *sNode)
{
    sNode->nextNode = sList->firstNode;
    
    sList->firstNode = sNode;
    sList->nodeCount++;
}

/**********************************************************************************************************
** Function name        :   vSListRemoveFirst
** Descriptions         :   �Ƴ����������еĵ�1�����
** parameters           :   sList ���Ƴ�����
** Returned value       :   �������Ϊ�գ�����0������Ļ������ص�1�����
***********************************************************************************************************/
vSNode *vSListRemoveFirst(vSList *sList)
{
    vSNode *sNode = (vSNode *)0;
    
    if (sList->nodeCount != 0)
    {
        sNode = sList->firstNode;
        
        sList->firstNode = sNode->nextNode;
        sList->nodeCount--;
        
        vSNodeInit(sNode);
    }
    return sNode;
}

