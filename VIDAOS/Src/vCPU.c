#include "VidaOS.h"
#include "stm32f4xx.h"

/**********************************************************************************************************
** Function name        :   vSetSysTickPeriod
** Descriptions         :   �趨SysTick�ж����ڡ�
** parameters           :   ms ÿms�������һ��SysTick�ж�
** Returned value       :   ��
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
** Descriptions         :   SystemTick���жϴ�������
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/
void SysTick_Handler()
{
    if (vOS_State == vRunning)
    {
        vTaskSystemTickHandler();   
    }
}

