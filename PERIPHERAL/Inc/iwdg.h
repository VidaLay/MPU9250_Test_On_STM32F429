#ifndef _IWDG_H
#define _IWDG_H
#include "sys.h"

void IWDG_Init(u8 prer, u16 rlr);	// ��ʼ��IWDG����ʹ��IWDG
void IWDG_Feed(void);				// ι��
#endif
