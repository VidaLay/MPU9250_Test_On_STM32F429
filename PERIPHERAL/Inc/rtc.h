#ifndef _RTC_H
#define _RTC_H
#include "sys.h"

extern RTC_HandleTypeDef RTC_Handler;  // RTC���
    
u8 RTC_Init(void);              // RTC��ʼ��
HAL_StatusTypeDef RTC_Set_Time(u8 hour, u8 min, u8 sec, u8 ampm);       // RTCʱ������
HAL_StatusTypeDef RTC_Set_Date(u8 year, u8 month, u8 date, u8 week);	// RTC��������
void RTC_Set_AlarmA(u8 week, u8 hour, u8 min, u8 sec); // ��������ʱ��(����������,24Сʱ��)
void RTC_Set_WakeUp(u32 wksel, u16 cnt);               // �����Ի��Ѷ�ʱ������
#endif
