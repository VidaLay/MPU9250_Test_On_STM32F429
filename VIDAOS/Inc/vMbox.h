#ifndef _VMBOX_H
#define _VMBOX_H

#include "vEvent.h"

#define vMboxStoreNormal         0x00        // 正常发送发送至缓冲区尾部
#define vMboxStoreFront          0x01        // 消息发送至缓冲区头部    
#define vMboxSentToAll           0x02        // 消息发送给所有等待任务

// 邮箱类型
typedef struct _vMbox
{
    // 事件控制块
	// 该结构被特意放到起始处，以实现tSem同时是一个tEvent的目的
    vEvent event;
    
    // 当前的消息数量
    uint32_t count;
    
    // 读取消息的索引
    uint32_t read;
    
    // 写消息的索引
    uint32_t write;
    
    // 最大允许容纳的消息数量
    uint32_t maxCount;
    
    // 消息存储缓冲区
    void **msgBuffer;
}vMbox;

// 邮箱状态类型
typedef struct _vMboxInfo
{
    // 当前的消息数量
    uint32_t count;
    
    // 最大允许容纳的消息数量
    uint32_t maxCount;
    
    // 当前等待的任务计数
    uint32_t taskCount;
}vMboxInfo;

/**********************************************************************************************************
** Function name        :   vMboxInit
** Descriptions         :   初始化邮箱
** parameters           :   mbox 等待初始化的邮箱
** parameters           :   msgBuffer 消息存储缓冲区
** parameters           :   maxCount 最大计数
** Returned value       :   无
***********************************************************************************************************/
void vMboxInit(vMbox *mbox, void **msgBuffer, uint32_t maxCount);

/**********************************************************************************************************
** Function name        :   vMboxWait
** Descriptions         :   等待邮箱, 获取一则消息
** parameters           :   mbox 等待的邮箱
** parameters           :   msg 消息存储缓存区
** parameters           :   waitTicks 最大等待的ticks数，为0表示无限等待
** Returned value       :   等待结果,vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vMboxWait(vMbox *mbox, void **msg, uint32_t waitTicks);

/**********************************************************************************************************
** Function name        :   vMboxNoWaitGet
** Descriptions         :   获取一则消息，如果没有消息，则立即退回
** parameters           :   mbox 获取消息的邮箱
** parameters           :   msg 消息存储缓存区
** Returned value       :   获取结果, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vMboxNoWaitGet(vMbox *mbox, void **msg);

/**********************************************************************************************************
** Function name        :   vMboxNotify
** Descriptions         :   通知消息可用，唤醒等待队列中的一个任务，或者将消息插入到邮箱中
** parameters           :   mbox 操作的信号量
** parameters           :   msg 发送的消息
** parameters           :   notifyOption 发送的选项 vMboxStoreNormal/vMboxStoreFront/vMboxSentToAll
** Returned value       :   vErrorNoError/ vErrorResourceFull
***********************************************************************************************************/
uint32_t vMboxNotify(vMbox *mbox, void *msg, uint32_t notifyOption);

/**********************************************************************************************************
** Function name        :   vMboxFlush
** Descriptions         :   清空邮箱中所有消息
** parameters           :   mbox 等待清空的邮箱
** Returned value       :   无
***********************************************************************************************************/
void vMboxFlush(vMbox *mbox);

/**********************************************************************************************************
** Function name        :   vMboxDestroy
** Descriptions         :   销毁邮箱
** parameters           :   mbox 需要销毁的邮箱
** Returned value       :   因销毁该信号量而唤醒的任务数量
***********************************************************************************************************/
uint32_t vMboxDestroy(vMbox *mbox);

/**********************************************************************************************************
** Function name        :   vMboxGetInfo
** Descriptions         :   查询状态信息
** parameters           :   mbox 查询的邮箱
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vMboxGetInfo(vMbox * mbox, vMboxInfo *info);

#endif
