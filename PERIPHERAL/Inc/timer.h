#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"

extern TIM_HandleTypeDef TIM3_Handler;      // 定时器句柄 
extern TIM_OC_InitTypeDef TIM3_CH4Handler;  // 定时器3通道4句柄
extern TIM_HandleTypeDef TIM5_Handler;      // 定时器5句柄

void TIM3_Init(u16 arr, u16 psc);
void TIM4_Init(u16 arr, u16 psc);
void TIM3_PWM_Init(u16 arr, u16 psc);
void TIM_SetTIM3Compare4(u32 compare);
//void TIM_SetTIM3Compare4(TIM_HandleTypeDef *htim, u32 compare);
u32 TIM_GetTIM3Capture4(void);
void TIM5_CH1_Cap_Init(u32 arr, u16 psc);

#endif

