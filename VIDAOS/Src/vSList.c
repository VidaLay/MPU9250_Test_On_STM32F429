#include "vLib.h"

#define firstNode headNode.nextNode

/**********************************************************************************************************
** Function name        :   vSNodeInit
** Descriptions         :   初始化单向结点类型
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vSNodeInit(vSNode *sNode)
{
    sNode->nextNode = sNode;
}

/**********************************************************************************************************
** Function name        :   vSListInit
** Descriptions         :   单向链表初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vSListInit(vSList *sList)
{
    sList->firstNode = &(sList->headNode);
    sList->nodeCount = 0;
}

/**********************************************************************************************************
** Function name        :   vSListCount
** Descriptions         :   返回单向链表中结点的数量
** parameters           :   无
** Returned value       :   结点数量
***********************************************************************************************************/
uint32_t vSListCount(vSList *sList)
{
    return sList->nodeCount;
}

/**********************************************************************************************************
** Function name        :   vSListFirst
** Descriptions         :   返回单向链表的首个结点
** parameters           :   sList 查询的链表
** Returned value       :   首个结点，如果链表为空，则返回0
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
** Descriptions         :   返回单向链表中指定结点的后一结点
** parameters           :   sList 查询的链表
** parameters           :   sNode 参考结点
** Returned value       :   后一结点结点，如果结点没有前结点（链表为空），则返回0
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
** Descriptions         :   移除单向链表中的所有结点
** parameters           :   sList 待清空的链表
** Returned value       :   无
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
** Descriptions         :   将指定结点添加到单向链表的头部
** parameters           :   sList 待插入链表
** parameters			:   sNode 待插入的结点
** Returned value       :   无
***********************************************************************************************************/
void vSListAddFirst(vSList *sList, vSNode *sNode)
{
    sNode->nextNode = sList->firstNode;
    
    sList->firstNode = sNode;
    sList->nodeCount++;
}

/**********************************************************************************************************
** Function name        :   vSListRemoveFirst
** Descriptions         :   移除单向链表中的第1个结点
** parameters           :   sList 待移除链表
** Returned value       :   如果链表为空，返回0，否则的话，返回第1个结点
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

