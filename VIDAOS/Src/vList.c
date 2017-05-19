#include "vLib.h"

#define firstNode headNode.nextNode
#define lastNode headNode.preNode

/**********************************************************************************************************
** Function name        :   vNodeInit
** Descriptions         :   ��ʼ��˫��������
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vNodeInit(vNode *node)
{
    node->nextNode = node;
    node->preNode = node;
}

/**********************************************************************************************************
** Function name        :   vListInit
** Descriptions         :   ˫�������ʼ��
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void vListInit(vList *list)
{
    list->firstNode = &(list->headNode);
    list->lastNode = &(list->headNode);
    list->nodeCount = 0;
}

/**********************************************************************************************************
** Function name        :   vListCount
** Descriptions         :   ����˫�������н�������
** parameters           :   ��
** Returned value       :   �������
***********************************************************************************************************/
uint32_t vListCount(vList *list)
{
    return list->nodeCount;
}

/**********************************************************************************************************
** Function name        :   vListFirst
** Descriptions         :   ����˫��������׸����
** parameters           :   list ��ѯ������
** Returned value       :   �׸���㣬�������Ϊ�գ��򷵻�0
***********************************************************************************************************/
vNode *vListFirst(vList *list)
{
    vNode *node = (vNode *)0;
    
    if (list->nodeCount != 0)
    {
        node = list->firstNode;
    }
    return node;
}

/**********************************************************************************************************
** Function name        :   vListLast
** Descriptions         :   ����˫����������һ�����
** parameters           :   list ��ѯ������
** Returned value       :   ���Ľ�㣬�������Ϊ�գ��򷵻�0
***********************************************************************************************************/
vNode *vListLast(vList *list)
{
    vNode *node = (vNode *)0;
    
    if (list->nodeCount != 0)
    {
        node = list->lastNode;
    }
    return node;
}

/**********************************************************************************************************
** Function name        :   vListPre
** Descriptions         :   ����˫��������ָ������ǰһ���
** parameters           :   list ��ѯ������
** parameters           :   node �ο����
** Returned value       :   ǰһ����㣬������û��ǰ��㣨����Ϊ�գ����򷵻�0
***********************************************************************************************************/
vNode *vListPre(vList *list, vNode *node)
{ 
    if (node->preNode == node)
    {
        return (vNode *)0;
    }
    else
    {
        return node->preNode;
    }
}

/**********************************************************************************************************
** Function name        :   vListNext
** Descriptions         :   ����˫��������ָ�����ĺ�һ���
** parameters           :   list ��ѯ������
** parameters           :   node �ο����
** Returned value       :   ��һ����㣬������û��ǰ��㣨����Ϊ�գ����򷵻�0
***********************************************************************************************************/
vNode *vListNext(vList *list, vNode *node)
{ 
    if (node->nextNode == node)
    {
        return (vNode *)0;
    }
    else
    {
        return node->nextNode;
    }
}

/**********************************************************************************************************
** Function name        :   vListRemoveAll
** Descriptions         :   �Ƴ�˫�������е����н��
** parameters           :   list ����յ�����
** Returned value       :   ��
***********************************************************************************************************/
void vListRemoveAll(vList *list)
{
    uint32_t count;
    vNode *nextNode;
    
    nextNode = list->firstNode;
    
    for (count = list->nodeCount; count !=0; count--)
    {
        vNode *currentNode = nextNode;
        nextNode = nextNode->nextNode;
        
        vNodeInit(currentNode);
        
//        currentNode->nextNode = currentNode;
//        currentNode->preNode = currentNode;
    }
    
    vListInit(list);
    
//    list->firstNode = &(list->headNode);
//    list->lastNode = &(list->headNode);
//    list->nodeCount = 0;
}

/**********************************************************************************************************
** Function name        :   vListAddFirst
** Descriptions         :   ��ָ�������ӵ�˫�������ͷ��
** parameters           :   list ����������
** parameters			:   node ������Ľ��
** Returned value       :   ��
***********************************************************************************************************/
void vListAddFirst(vList *list, vNode *node)
{
//    vListInsertAfter(list, &(list->headNode), node);    
    node->preNode = list->firstNode->preNode;
    node->nextNode = list->firstNode;
    
    list->firstNode->preNode = node;
    list->firstNode = node;
    list->nodeCount++;
}

/**********************************************************************************************************
** Function name        :   vListAddLast
** Descriptions         :   ��ָ�������ӵ�˫�������ĩβ
** parameters           :   list ����������
** parameters			:   node ������Ľ��
** Returned value       :   ��
***********************************************************************************************************/
void vListAddLast(vList *list, vNode *node)
{
//    vListInsertAfter(list, list->lastNode, node);
    node->preNode = list->lastNode;
    node->nextNode = list->lastNode->nextNode;
    
    list->lastNode->nextNode = node;
    list->lastNode = node;
    list->nodeCount++;
}

/**********************************************************************************************************
** Function name        :   vListRemoveFirst
** Descriptions         :   �Ƴ�˫�������еĵ�1�����
** parameters           :   list ���Ƴ�����
** Returned value       :   �������Ϊ�գ�����0������Ļ������ص�1�����
***********************************************************************************************************/
vNode *vListRemoveFirst(vList *list)
{
    vNode *node = (vNode *)0;
    
    if (list->nodeCount != 0)
    {
        node = list->firstNode;
        
        node->nextNode->preNode = node->preNode;
        list->firstNode = node->nextNode;
        list->nodeCount--;
        
        vNodeInit(node);
    }
    return node;
}

/**********************************************************************************************************
** Function name        :   vListInsertAfter
** Descriptions         :   ��ָ���Ľ����뵽ĳ��������
** parameters           :   list 			�����������
** parameters           :   refNode 		�ο����
** parameters           :   nodeToInsert 	������Ľṹ
** Returned value       :   ��
***********************************************************************************************************/
void vListInsertAfter(vList * list, vNode *refNode, vNode *nodeToInsert)
{
    nodeToInsert->preNode = refNode;
    nodeToInsert->nextNode = refNode->nextNode;
    
    refNode->nextNode->preNode = nodeToInsert;
    refNode->nextNode = nodeToInsert;
    
    list->nodeCount++;
}

/**********************************************************************************************************
** Function name        :   vListInsertBefore
** Descriptions         :   ��ָ���Ľ����뵽ĳ�����ǰ��
** parameters           :   list 			�����������
** parameters           :   refNode 		�ο����
** parameters           :   nodeToInsert 	������Ľṹ
** Returned value       :   ��
***********************************************************************************************************/
void vListInsertBefore(vList * list, vNode *refNode, vNode *nodeToInsert)
{
    nodeToInsert->preNode = refNode->preNode;
    nodeToInsert->nextNode = refNode;
    
    refNode->preNode->nextNode = nodeToInsert;
    refNode->preNode = nodeToInsert;
    
    list->nodeCount++;
}

/**********************************************************************************************************
** Function name        :   vListRemove
** Descriptions         :   ��ָ������˫���������Ƴ�
** parameters           :   list 	���Ƴ�������
** parameters           :   node 	���Ƴ��Ľ��
** Returned value       :   ��
***********************************************************************************************************/
void vListRemove(vList *list, vNode *node)
{
    node->preNode->nextNode = node->nextNode;
    node->nextNode->preNode = node->preNode;
    
    vNodeInit(node);
    
    list->nodeCount--;
}

