#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "wkup.h"
#include "sdram.h"
#include "lcd.h"
#include "VidaOS.h"

int main(void)
{
	HAL_Init();                     // ��ʼ��HAL��
	Stm32_Clock_Init(360,25,2,8);   // ����ʱ��,180Mhz
    delay_init(180);
    uart_init(500000);              // ��ʼ��USART
	LED_Init();                     // ��ʼ��LED 
	KEY_Init();                     // ��ʼ������
    WKUP_Init();	                // �������ѳ�ʼ��
	SDRAM_Init();                   // ��ʼ��SDRAM
	LCD_Init();                     // ��ʼ��LCD
    
	vOSStart();

    return 0;
}

