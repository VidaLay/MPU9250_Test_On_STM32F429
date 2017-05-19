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
	HAL_Init();                     // 初始化HAL库
	Stm32_Clock_Init(360,25,2,8);   // 设置时钟,180Mhz
    delay_init(180);
    uart_init(500000);              // 初始化USART
	LED_Init();                     // 初始化LED 
	KEY_Init();                     // 初始化按键
    WKUP_Init();	                // 待机唤醒初始化
	SDRAM_Init();                   // 初始化SDRAM
	LCD_Init();                     // 初始化LCD
    
	vOSStart();

    return 0;
}

