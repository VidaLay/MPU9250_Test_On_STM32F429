#ifndef _VMEMBLOCK_H
#define _VMEMBLOCK_H

#include "vEvent.h"

typedef struct _vMemBlock
{
    // 事件控制块
    vEvent event;
    
    // 存储块的首地址
    void *memstart;
    
    // 每个存储块的大小
    uint32_t blockSize;
    
    // 总的存储块的个数
    uint32_t maxCount;
    
    // 存储块列表
    vList blockList;
}vMemBlock;

typedef struct _vMemBlockInfo
{
    // 当前存储块的计数
    uint32_t count;
    
    // 允许的最大计数
    uint32_t maxCount;
    
    // 每个存储块的大小
    uint32_t blockSize;
    
    // 当前等待的任务计数
    uint32_t taskCount;
}vMemBlockInfo;

/**********************************************************************************************************
** Function name        :   vMemBlockInit
** Descriptions         :   初始化存储控制块
** parameters           :   memBlock 等待初始化的存储块结构
** parameters           :   memStart 存储区的起始地址
** parameters           :   blockSize 每个块的大小
** parameters           :   blockCnt 总的块数量
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
void vMemBlockInit(vMemBlock *memBlock, uint8_t *memStart, uint32_t blockSize, uint32_t blockCnt);

/**********************************************************************************************************
** Function name        :   vMemBlockWait
** Descriptions         :   等待存储块
** parameters           :   memBlock 等待的存储块结构
** parameters			:   mem 存储块
** parameters           :   waitTicks 当没有存储块时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果, vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vMemBlockWait(vMemBlock *memBlock, uint8_t **mem, uint32_t waitTicks);

/**********************************************************************************************************
** Function name        :   vMemBlockNoWaitGet
** Descriptions         :   获取存储块，如果没有存储块，则立即退回
** parameters           :   memBlock 等待的存储块结构
** parameters			:   mem 存储块
** Returned value       :   获取结果, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vMemBlockNoWaitGet(vMemBlock *memBlock, uint8_t **mem);

/**********************************************************************************************************
** Function name        :   vMemBlockNotify
** Descriptions         :   通知存储块可用，唤醒等待队列中的一个任务，或者将存储块加入队列中
** parameters           :   memBlock 操作的存储块结构
** parameters			:   mem 存储块
** Returned value       :   无
***********************************************************************************************************/
void vMemBlockNotify(vMemBlock *memBlock, uint8_t *mem);

/**********************************************************************************************************
** Function name        :   vMemBlockGetInfo
** Descriptions         :   查询存储控制块的状态信息
** parameters           :   memBlock 存储块结构
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vMemBlockGetInfo(vMemBlock *memBlock, vMemBlockInfo *info);

/**********************************************************************************************************
** Function name        :   vMemBlockDestroy
** Descriptions         :   销毁存储控制块
** parameters           :   memBlock 需要销毁的存储块结构
** Returned value       :   因销毁该存储控制块而唤醒的任务数量
***********************************************************************************************************/
uint32_t vMemBlockDestroy(vMemBlock *memblock);


#endif
