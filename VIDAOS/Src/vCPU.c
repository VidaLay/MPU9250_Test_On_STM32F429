#include "VidaOS.h"
#include "stm32f4xx.h"

/**********************************************************************************************************
** Function name        :   vSetSysTickPeriod
** Descriptions         :   设定SysTick中断周期。
** parameters           :   ms 每ms毫秒产生一次SysTick中断
** Returned value       :   无
***********************************************************************************************************/

void vSetSysTickPeriod(uint32_t ms)
{
    SysTick->LOAD  = ms * VIDAOS_SYSTEM_CORE_CLOCK / 1000 - 1; 
    NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
    SysTick->VAL   = 0;                           
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk; 
}

/**********************************************************************************************************
** Function name        :   SysTick_Handler
** Descriptions         :   SystemTick的中断处理函数。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void SysTick_Handler()
{
    if (vOS_State == vRunning)
    {
        vTaskSystemTickHandler();   
    }
}

