#ifndef _VFLAGGROUP_H
#define _VFLAGGROUP_H

#include "vEvent.h"

typedef struct _vFlagGroup
{
    // 事件控制块
    vEvent event;
    
    // 当前事件标志
    uint32_t flags;
}vFlagGroup;

typedef struct _vFlagGroupInfo
{
    // 当前的事件标志
    uint32_t flags;
    
    // 当前等待的任务计数
    uint32_t taskCount;
}vFlagGroupInfo;

#define VFLAGGROUP_CLEAR            (0x0 << 0)
#define VFLAGGROUP_SET              (0x1 << 0)
#define VFLAGGROUP_ANY              (0x0 << 1)
#define VFLAGGROUP_ALL              (0x1 << 1)

#define VFLAGGROUP_SET_ALL          (VFLAGGROUP_SET | VFLAGGROUP_ALL)
#define VFLAGGROUP_SET_ANY          (VFLAGGROUP_SET | VFLAGGROUP_ANY)
#define VFLAGGROUP_CLEAR_ALL        (VFLAGGROUP_CLEAR | VFLAGGROUP_ALL)
#define VFLAGGROUP_CLEAR_ANY        (VFLAGGROUP_CLEAR | VFLAGGROUP_ANY)

#define VFLAGGROUP_CONSUME          (1 << 7)

/**********************************************************************************************************
** Function name        :   vFlagGroupInit
** Descriptions         :   初始化事件标志组
** parameters           :   flagGroup 等待初始化的事件标志组
** parameters           :   flags 初始的事件标志
** Returned value       :   无
***********************************************************************************************************/
void vFlagGroupInit(vFlagGroup * flagGroup, uint32_t flags);

/**********************************************************************************************************
** Function name        :   vFlagGroupWait
** Descriptions         :   等待事件标志组中特定的标志
** parameters           :   flagGroup 等待的事件标志组
** parameters           :   waitType 等待的事件类型
** parameters           :   requstFlag 请求的事件标志
** parameters           :   resultFlag 等待标志结果
** parameters           :   waitTicks 当等待的标志没有满足条件时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果, vErrorNoError/ vErrorTimeout/ vErrorResourceUnavaliable/ vErrorDel
***********************************************************************************************************/
uint32_t vFlagGroupWait(vFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag, uint32_t waitTicks);

/**********************************************************************************************************
** Function name        :   vFlagGroupNoWaitGet
** Descriptions         :   获取事件标志组中特定的标志
** parameters           :   flagGroup 获取的事件标志组
** parameters           :   waitType 获取的事件类型
** parameters           :   requstFlag 请求的事件标志
** parameters           :   resultFlag 等待标志结果
** Returned value       :   获取结果, vErrorNoError/ vErrorResourceUnavaliable
***********************************************************************************************************/
uint32_t vFlagGroupNoWaitGet(vFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag);

/**********************************************************************************************************
** Function name        :   vFlagGroupNotify
** Descriptions         :   通知事件标志组中的任务有新的标志发生
** parameters           :   flagGroup 事件标志组
** parameters           :   isSet 是否是置1事件
** parameters           :   flags 产生的事件标志
***********************************************************************************************************/
void vFlagGroupNotify(vFlagGroup *flagGroup, uint8_t isSet, uint32_t flags);

/**********************************************************************************************************
** Function name        :   vFlagGroupGetInfo
** Descriptions         :   查询事件标志组的状态信息
** parameters           :   flagGroup 事件标志组
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void vFlagGroupGetInfo(vFlagGroup *flagGroup, vFlagGroupInfo * info);

/**********************************************************************************************************
** Function name        :   vFlagGroupDestroy
** Descriptions         :   销毁事件标志组
** parameters           :   flagGroup 事件标志组
** Returned value       :   因销毁该存储控制块而唤醒的任务数量
***********************************************************************************************************/
uint32_t vFlagGroupDestroy(vFlagGroup *flagGroup);


#endif
