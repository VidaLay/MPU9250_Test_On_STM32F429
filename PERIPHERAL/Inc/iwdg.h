#ifndef _IWDG_H
#define _IWDG_H
#include "sys.h"

void IWDG_Init(u8 prer, u16 rlr);	// 初始化IWDG，并使能IWDG
void IWDG_Feed(void);				// 喂狗
#endif
