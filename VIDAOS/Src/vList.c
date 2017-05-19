#include "vLib.h"

#define firstNode headNode.nextNode
#define lastNode headNode.preNode

/**********************************************************************************************************
** Function name        :   vNodeInit
** Descriptions         :   初始化双向结点类型
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vNodeInit(vNode *node)
{
    node->nextNode = node;
    node->preNode = node;
}

/**********************************************************************************************************
** Function name        :   vListInit
** Descriptions         :   双向链表初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void vListInit(vList *list)
{
    list->firstNode = &(list->headNode);
    list->lastNode = &(list->headNode);
    list->nodeCount = 0;
}

/**********************************************************************************************************
** Function name        :   vListCount
** Descriptions         :   返回双向链表中结点的数量
** parameters           :   无
** Returned value       :   结点数量
***********************************************************************************************************/
uint32_t vListCount(vList *list)
{
    return list->nodeCount;
}

/**********************************************************************************************************
** Function name        :   vListFirst
** Descriptions         :   返回双向链表的首个结点
** parameters           :   list 查询的链表
** Returned value       :   首个结点，如果链表为空，则返回0
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
** Descriptions         :   返回双向链表的最后一个结点
** parameters           :   list 查询的链表
** Returned value       :   最后的结点，如果链表为空，则返回0
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
** Descriptions         :   返回双向链表中指定结点的前一结点
** parameters           :   list 查询的链表
** parameters           :   node 参考结点
** Returned value       :   前一结点结点，如果结点没有前结点（链表为空），则返回0
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
** Descriptions         :   返回双向链表中指定结点的后一结点
** parameters           :   list 查询的链表
** parameters           :   node 参考结点
** Returned value       :   后一结点结点，如果结点没有前结点（链表为空），则返回0
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
** Descriptions         :   移除双向链表中的所有结点
** parameters           :   list 待清空的链表
** Returned value       :   无
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
** Descriptions         :   将指定结点添加到双向链表的头部
** parameters           :   list 待插入链表
** parameters			:   node 待插入的结点
** Returned value       :   无
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
** Descriptions         :   将指定结点添加到双向链表的末尾
** parameters           :   list 待插入链表
** parameters			:   node 待插入的结点
** Returned value       :   无
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
** Descriptions         :   移除双向链表中的第1个结点
** parameters           :   list 待移除链表
** Returned value       :   如果链表为空，返回0，否则的话，返回第1个结点
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
** Descriptions         :   将指定的结点插入到某个结点后面
** parameters           :   list 			待插入的链表
** parameters           :   refNode 		参考结点
** parameters           :   nodeToInsert 	待插入的结构
** Returned value       :   无
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
** Descriptions         :   将指定的结点插入到某个结点前面
** parameters           :   list 			待插入的链表
** parameters           :   refNode 		参考结点
** parameters           :   nodeToInsert 	待插入的结构
** Returned value       :   无
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
** Descriptions         :   将指定结点从双向链表中移除
** parameters           :   list 	待移除的链表
** parameters           :   node 	待移除的结点
** Returned value       :   无
***********************************************************************************************************/
void vListRemove(vList *list, vNode *node)
{
    node->preNode->nextNode = node->nextNode;
    node->nextNode->preNode = node->preNode;
    
    vNodeInit(node);
    
    list->nodeCount--;
}

