#include "exti.h"
#include "delay.h"
#include "led.h"
#include "key.h"

// 外部中断初始化
void EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOA_CLK_ENABLE();               // 开启GPIOA时钟
    __HAL_RCC_GPIOC_CLK_ENABLE();               // 开启GPIOC时钟
    __HAL_RCC_GPIOH_CLK_ENABLE();               // 开启GPIOH时钟
    
    GPIO_Initure.Pin = GPIO_PIN_0;                  // PA0
    GPIO_Initure.Mode = GPIO_MODE_IT_RISING;        // 上升沿触发
    GPIO_Initure.Pull = GPIO_PULLDOWN;              // 下拉
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;           // 高速
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);
    
    GPIO_Initure.Pin = GPIO_PIN_13;                 // PC13
    GPIO_Initure.Mode = GPIO_MODE_IT_FALLING;       // 下降沿触发
    GPIO_Initure.Pull = GPIO_PULLUP;                // 上拉
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;           // 高速
    HAL_GPIO_Init(GPIOC, &GPIO_Initure);
    
    GPIO_Initure.Pin = GPIO_PIN_2 | GPIO_PIN_3;       // PH2,3
    HAL_GPIO_Init(GPIOH, &GPIO_Initure);
    
    // 中断线0-PA0
    HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);       // 抢占优先级为2，子优先级为0
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);               // 使能中断线0
    
    // 中断线2-PH2
    HAL_NVIC_SetPriority(EXTI2_IRQn, 2, 1);       // 抢占优先级为2，子优先级为1
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);               // 使能中断线2
    
    // 中断线3-PH3
    HAL_NVIC_SetPriority(EXTI3_IRQn, 2, 2);       // 抢占优先级为2，子优先级为2
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);               // 使能中断线2
    
    // 中断线13-PC13
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 3);   // 抢占优先级为2，子优先级为3
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);           // 使能中断线13
}


// 中断服务函数
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);// 调用中断处理公用函数
}

void EXTI2_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);// 调用中断处理公用函数
}

void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);// 调用中断处理公用函数
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);// 调用中断处理公用函数
}

// 中断服务程序中需要做的事情
// 在HAL库中所有的外部中断服务函数都会调用此函数
// GPIO_Pin:中断引脚号
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//    delay_ms(100);      // 消抖
//    switch(GPIO_Pin)
//    {
//        case GPIO_PIN_0:
//            if (WK_UP == 1) 
//            {
//				LED1 =! LED1;// 控制LED0,LED1互斥点亮
//				LED0 =! LED1;
//            }
//            break;
//        case GPIO_PIN_2:
//            if (KEY1 == 0)  // LED1翻转
//            {
//                LED1 =! LED1;    
//            }
//            break;
//        case GPIO_PIN_3:
//            if (KEY0 == 0)  // 同时控制LED0,LED1翻转 
//            {
//                LED0 =! LED0;
//				LED1 =! LED1;
//            }
//            break;

//        case GPIO_PIN_13:
//            if (KEY2 == 0)  
//            {
//				LED0 =! LED0;// 控制LED0翻转
//            }
//            break;
//    }
//}
