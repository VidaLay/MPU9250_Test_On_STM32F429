#include "VidaOS.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "exti.h"
#include "iwdg.h"
#include "timer.h"
#include "tpad.h"
#include "rtc.h"
#include "lcd.h"
#include "ltdc.h"
#include "can.h"
#include "mpu9250.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h" 
#include "wkup.h"

vTask MPUTask;
vTask LCDTask;
vTask CANTask;
vTask USARTTask;

vTaskInfo MPUTaskInfo;
vTaskInfo LCDTaskInfo;
vTaskInfo CANTaskInfo;
vTaskInfo USARTTaskInfo;

vTaskStack MPUTaskStack[512];
vTaskStack LCDTaskStack[256];
vTaskStack CANTaskStack[256];
vTaskStack USARTTaskStack[256];

float cpuUsage = 0.0;

vMbox mbox_UPLOAD;
vMbox mbox_LCD;
void * mboxMsgBuffer_UPLOAD[20];
void * mboxMsgBuffer_LCD[20];

typedef struct _MPU_MBOX_MSG
{
	float pitch, roll, yaw; 	    // 欧拉角
	short aacx, aacy, aacz;	        // 加速度传感器原始数据
	short gyrox, gyroy, gyroz;      // 陀螺仪原始数据 
	short temp;		                // 温度    
}MPU_MBOX_MSG;

MPU_MBOX_MSG mbox_msg;

u8 report_CAN = 1;	                // 默认开启CAN上报
u8 report_USART = 1;                // 默认开启USART上报
u8 update_LCD = 1;                  // 默认开启LCD更新

// 串口1发送1个字符 
// c:要发送的字符
void usart1_send_char(u8 c)
{
    while (__HAL_UART_GET_FLAG(&UART1_Handler, UART_FLAG_TC) == RESET){}; 
    USART1->DR = c;
} 

// 传送数据给匿名四轴上位机软件(V2.6版本)
// fun:功能字. 0X01~0X1C
// data:数据缓存区,最多28字节!!
// len:data区有效数据个数
void usart1_niming_report(u8 fun, u8 *data, u8 len)
{
	u8 send_buf[32];
	u8 i;
    
	if (len > 28) return;	// 最多28字节数据 
    
	send_buf[len + 3] = 0;	// 校验数置零
	send_buf[0] = 0XAA;	// 帧头
	send_buf[1] = 0XAA;	// 帧头
	send_buf[2] = fun;	// 功能字
	send_buf[3] = len;	// 数据长度
    
	for (i = 0; i < len; i++)
    {
        send_buf[4 + i] = data[i];			// 复制数据
    }
    
    send_buf[len + 4] = 0;
    
	for (i = 0;i < len + 4; i++)
    {
        send_buf[len + 4] += send_buf[i];	// 计算校验和
    }
    
	for (i = 0;i < len + 5; i++)
    {
        usart1_send_char(send_buf[i]);	// 发送数据到串口1 
    }
}

// 发送加速度传感器数据+陀螺仪数据(传感器帧)
// aacx,aacy,aacz:x,y,z三个方向上面的加速度值
// gyrox,gyroy,gyroz:x,y,z三个方向上面的陀螺仪值 
void mpu6050_send_data(short aacx, short aacy, short aacz, short gyrox, short gyroy, short gyroz)
{
	u8 tbuf[18]; 
	tbuf[0] = (aacx >> 8) & 0XFF;
	tbuf[1] = aacx & 0XFF;
	tbuf[2] = (aacy >> 8) & 0XFF;
	tbuf[3] = aacy & 0XFF;
	tbuf[4] = (aacz >> 8) & 0XFF;
	tbuf[5] = aacz & 0XFF; 
	tbuf[6] = (gyrox >> 8) & 0XFF;
	tbuf[7] = gyrox & 0XFF;
	tbuf[8] = (gyroy >> 8) & 0XFF;
	tbuf[9] = gyroy & 0XFF;
	tbuf[10] = (gyroz >> 8) & 0XFF;
	tbuf[11] = gyroz & 0XFF;
	tbuf[12] = 0;// 因为开启MPL后,无法直接读取磁力计数据,所以这里直接屏蔽掉.用0替代.
	tbuf[13] = 0;
	tbuf[14] = 0;
	tbuf[15] = 0;
	tbuf[16] = 0;
	tbuf[17] = 0;
    
    usart1_niming_report(0X02, tbuf, 18);// 传感器帧,0X02
}

// 通过串口1上报结算后的姿态数据给电脑(状态帧)
// roll:横滚角.单位0.01度。 -18000 -> 18000 对应 -180.00  ->  180.00度
// pitch:俯仰角.单位 0.01度。-9000 - 9000 对应 -90.00 -> 90.00 度
// yaw:航向角.单位为0.1度 0 -> 3600  对应 0 -> 360.0度
// csb:超声波高度,单位:cm
// prs:气压计高度,单位:mm
void usart1_report_imu(short roll, short pitch, short yaw, short csb, int prs)
{
	u8 tbuf[12];   	
	tbuf[0] = (roll >> 8) & 0XFF;
	tbuf[1] = roll & 0XFF;
	tbuf[2] = (pitch >> 8) & 0XFF;
	tbuf[3] = pitch & 0XFF;
	tbuf[4] = (yaw >> 8) & 0XFF;
	tbuf[5] = yaw & 0XFF;
	tbuf[6] = (csb >> 8) & 0XFF;
	tbuf[7] = csb & 0XFF;
	tbuf[8] = (prs >> 24) & 0XFF;
	tbuf[9] = (prs >> 16) & 0XFF;
	tbuf[10] = (prs >> 8) & 0XFF;
	tbuf[11] = prs & 0XFF;
	usart1_niming_report(0X01, tbuf, 12);// 状态帧,0X01
}  

void MPUTaskEntry(void *param)
{
    vMboxInit(&mbox_UPLOAD, mboxMsgBuffer_UPLOAD, 20);
    vMboxInit(&mbox_LCD, mboxMsgBuffer_LCD, 20);
    
    EXTI_Init();                            // 外部中断初始化
    IWDG_Init(IWDG_PRESCALER_64, 2500);  	// 分频数为64,重载值为2500,溢出时间为5s	

    TIM3_Init(200-1,9000-1);        //定时器3初始化，定时器时钟为90M，分频系数为9000-1，
                                    //所以定时器3的频率为90M/9000=10K，自动重装载为200-1，那么定时器周期就是20ms
    TIM4_Init(1000-1,9000-1);       //定时器3初始化，定时器时钟为90M，分频系数为9000-1，
                                    //所以定时器3的频率为90M/9000=10K，自动重装载为1000-1，那么定时器周期就是100ms
 	 
	while (mpu_dmp_init())   
	{
		LCD_ShowString(30, 130, 200, 16, 16, "MPU9250 Error");
		delay_ms(200);
		LCD_Fill(30, 130, 239, 130 + 16, WHITE);
 		delay_ms(200);
		LED0 = !LED0;// DS0闪烁 
	}

    while (1)
    {            
        if (report_CAN || report_USART)
        {
            HAL_NVIC_EnableIRQ(TIM3_IRQn);               // 开启ITM3中断   
        }
        else
        {
            HAL_NVIC_DisableIRQ(TIM3_IRQn);              // 开启ITM3中断   
        }   

        if (update_LCD)
        {
            HAL_NVIC_EnableIRQ(TIM4_IRQn);               // 开启ITM3中断   
        }
        else
        {
            HAL_NVIC_DisableIRQ(TIM4_IRQn);              // 开启ITM3中断   
        }
                
        LED0 = !LED0;// DS0闪烁 

        vTaskGetInfo(currentTask, &MPUTaskInfo);
        IWDG_Feed();    //喂狗
        vTaskDelay_ms(100);
	 } 
}

void LCDTaskEntry(void *param)
{
    short temp;
    void * msg;
    uint32_t err;
    u8 LCD_STATUS = 1;
    
    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;
    u8 tbuf[40];
    
    TPAD_Init(2);                   //初始化触摸按键,以90/8=11.25Mhz频率计数
    RTC_Init();                     //初始化RTC 
    RTC_Set_WakeUp(RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 0); //配置WAKE UP中断,1秒钟中断一次  
    
    POINT_COLOR = BLACK;
    LCD_ShowString(30, 30, 200, 16, 16, "STM32F429 MPU9250 TEST"); 
    
    POINT_COLOR = RED;
    LCD_ShowString(30, 60, 200, 16, 16,  "TPAD:LCD          ON/OFF");
    LCD_ShowString(30, 80, 200, 16, 16,  "KEY0:CAN UPLOAD   ON/OFF");
    LCD_ShowString(30, 100, 200, 16, 16, "KEY1:USART UPLOAD ON/OFF");
    LCD_ShowString(30, 120, 200, 16, 16, "KEY2:LCD REFRESH  ON/OFF");
    
    POINT_COLOR = BLUE;
    LCD_ShowString(30, 150, 200, 16, 16, "CAN UPLOAD   ON ");
    LCD_ShowString(30, 170, 200, 16, 16, "USART UPLOAD ON ");
    LCD_ShowString(30, 190, 200, 16, 16, "LCD REFRESH  ON ");
    
    LCD_ShowString(70, 220, 200, 16, 16, " Temp:    .  C");	
    LCD_ShowString(70, 240, 200, 16, 16, " Roll:    .  deg");
    LCD_ShowString(70, 260, 200, 16, 16, "Pitch:    .  deg");	 
    LCD_ShowString(70, 280, 200, 16, 16, " Yaw :    .  deg");
    
    for (;;)
    {
        err = vMboxWait(&mbox_LCD, &msg, 0);
        
        if (err == vErrorNoError) 
        {
            MPU_MBOX_MSG mbox_msg = *(MPU_MBOX_MSG*)msg;
            // 显示温度
            if (mbox_msg.temp < 0)
            {
                LCD_ShowChar(70 + 48, 220, '-', 16, 0);		// 显示负号
                mbox_msg.temp = -mbox_msg.temp;		// 转为正数
            }
            else 
            {
                LCD_ShowChar(70 + 48, 220, ' ', 16, 0);		// 去掉负号 
            }
            
            LCD_ShowNum(70 + 48 + 8, 220, mbox_msg.temp / 100, 3, 16);		// 显示整数部分	    
            LCD_ShowNum(70 + 48 + 40, 220, mbox_msg.temp % 10, 1, 16);		// 显示小数部分 
            
            // 显示roll
            temp = mbox_msg.roll * 10;
            
            if (temp < 0)
            {
                LCD_ShowChar(70 + 48, 240, '-', 16, 0);		// 显示负号
                temp = -temp;		// 转为正数
            }
            else
            {
                LCD_ShowChar(70 + 48, 240, ' ', 16, 0);		// 去掉负号 
            }
            
            LCD_ShowNum(70 + 48 + 8, 240, temp / 10, 3, 16);		// 显示整数部分	    
            LCD_ShowNum(70 + 48 + 40, 240, temp % 10, 1, 16);		// 显示小数部分 
            
            // 显示pitch
            temp = mbox_msg.pitch * 10;
            
            if (temp < 0)
            {
                LCD_ShowChar(70 + 48, 260, '-', 16, 0);		// 显示负号
                temp = -temp;		// 转为正数
            }
            else 
            {
                LCD_ShowChar(70 + 48, 260, ' ', 16, 0);		// 去掉负号 
            }
               
            LCD_ShowNum(70 + 48 + 8, 260, temp / 10, 3, 16);		// 显示整数部分	    
            LCD_ShowNum(70 + 48 + 40, 260, temp % 10, 1, 16);		// 显示小数部分 
            
            // 显示yaw
            temp = mbox_msg.yaw * 10;
            
            if (temp < 0)
            {
                LCD_ShowChar(70 + 48, 280, '-', 16, 0);		// 显示负号
                temp = -mbox_msg.temp;		// 转为正数
            }
            else
            {
                LCD_ShowChar(70 + 48, 280, ' ', 16, 0);		// 去掉负号
            }
            
            LCD_ShowNum(70 + 48 + 8, 280, temp / 10, 3, 16);		// 显示整数部分	    
            LCD_ShowNum(70 + 48 + 40, 280, temp % 10, 1, 16);		// 显示小数部分  
        }
        
        cpuUsage = vCPUUsageGet();
        temp = cpuUsage * 10;
        LCD_ShowString(70, 310, 200, 16, 16, "  CPU:    .  %");	
        LCD_ShowNum(70 + 48 + 8, 310, temp / 10, 3, 16);		// 显示整数部分	    
        LCD_ShowNum(70 + 48 + 40, 310, temp % 10, 1, 16);		// 显示小数部分
        

        HAL_RTC_GetTime(&RTC_Handler, &RTC_TimeStruct, RTC_FORMAT_BIN);
        sprintf ((char *)tbuf, "Time:%02d:%02d:%02d", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds); 
        LCD_ShowString(30, 370, 200, 16, 16, tbuf);	
        HAL_RTC_GetDate(&RTC_Handler, &RTC_DateStruct, RTC_FORMAT_BIN);
        sprintf((char *)tbuf, "Date:20%02d-%02d-%02d", RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date); 
        LCD_ShowString(30, 390, 200, 16, 16, tbuf);	
        
        switch (RTC_DateStruct.WeekDay)
        {
            case 1:
                sprintf((char *)tbuf, "WeekDay:Monday"); 
                break;
            case 2:
                sprintf((char *)tbuf, "WeekDay:Tuesday"); 
                break;
            case 3:
                sprintf((char *)tbuf, "WeekDay:Wednesday");
                break;
            case 4:
                sprintf((char *)tbuf, "WeekDay:Thursday");
                break;
            case 5:
                sprintf((char *)tbuf, "WeekDay:Friday");
                break;
            case 6:
                sprintf((char *)tbuf, "WeekDay:Saturday");
                break;
            case 7:
                sprintf((char *)tbuf, "WeekDay:Sunday");
                break;
        }
        
        LCD_ShowString(30, 410, 200, 16, 16, tbuf);
        
        if (TPAD_Scan(0))    //成功捕获到了一次上升沿(此函数执行时间至少15ms)
        {
            LCD_STATUS = !LCD_STATUS;
            
            if (LCD_STATUS)
            {
                LTDC_Layer_Switch(0,1);
                LTDC_Layer_Switch(1,1);
            }
            else
            {
                LTDC_Layer_Switch(0,0);
                LTDC_Layer_Switch(1,0);
            }       
        }
        
        vTaskGetInfo(currentTask, &LCDTaskInfo);
    }
}

void CANTaskEntry(void *param) 
{
    void *msg;
    uint32_t err;
    u8 res;
    
    CAN1_Mode_Init(CAN_SJW_1TQ, CAN_BS2_6TQ, CAN_BS1_8TQ, 6, CAN_MODE_NORMAL); // CAN初始化,波特率500Kbps    
    
    for (;;)
    {
        err = vMboxWait(&mbox_UPLOAD, &msg, 0);
        if (err == vErrorNoError) 
        {
            MPU_MBOX_MSG mbox_msg = *(MPU_MBOX_MSG*)msg;
            
            msg = &mbox_msg.aacx;
            res = CAN1_Send_Msg((u8 *)msg, 2, 0x01, 0x01);// 发送8个字节 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send Failed");		// 提示发送失败
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send OK    ");	 		// 提示发送成功	
            }
            
            msg = &mbox_msg.aacy;
            res = CAN1_Send_Msg((u8 *)msg, 2, 0x02, 0x02);// 发送8个字节 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send Failed");		// 提示发送失败
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send OK    ");	 		// 提示发送成功	
            }
            
            msg = &mbox_msg.aacz;
            res = CAN1_Send_Msg((u8 *)msg, 2, 0x03, 0x03);// 发送8个字节 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send Failed");		// 提示发送失败
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send OK    ");	 		// 提示发送成功	
            }
            
            msg = &mbox_msg.gyrox;
            res = CAN1_Send_Msg((u8 *)msg, 2, 0x04, 0x04);// 发送8个字节 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send Failed");		// 提示发送失败
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send OK    ");	 		// 提示发送成功	
            }
            
            msg = &mbox_msg.gyroy;
            res = CAN1_Send_Msg((u8 *)msg, 2, 0x05, 0x05);// 发送8个字节 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send Failed");		// 提示发送失败
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send OK    ");	 		// 提示发送成功	
            }
            
            msg = &mbox_msg.gyroz;
            res = CAN1_Send_Msg((u8 *)msg, 2, 0x06, 0x06);// 发送8个字节 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send Failed");		// 提示发送失败
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send OK    ");	 		// 提示发送成功	
            }
            
            msg = &mbox_msg.roll;
            res = CAN1_Send_Msg((u8 *)msg, 4, 0x07, 0x07);// 发送8个字节 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16,"CAN Send Failed");		// 提示发送失败
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16,"CAN Send OK    ");	 		// 提示发送成功	
            }
            
            msg = &mbox_msg.pitch;
            res = CAN1_Send_Msg((u8 *)msg, 4, 0x08, 0x08);// 发送8个字节 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16,"CAN Send Failed");		// 提示发送失败
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16,"CAN Send OK    ");	 		// 提示发送成功	
            }
            
            msg = &mbox_msg.yaw;
            res = CAN1_Send_Msg((u8 *)msg, 4, 0x09, 0x09);// 发送8个字节 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16,"CAN Send Failed");		// 提示发送失败
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16,"CAN Send OK    ");	 		// 提示发送成功	
            }
        }
        
        vTaskGetInfo(currentTask, &CANTaskInfo);
    }
}

void USARTTaskEntry(void *param)
{
    void * msg;
    uint32_t err;
    
    for (;;)
    {
        err = vMboxWait(&mbox_UPLOAD, &msg, 0);
        
        if (err == vErrorNoError) 
        {
            MPU_MBOX_MSG mbox_msg = *(MPU_MBOX_MSG*)msg;
            mpu6050_send_data(mbox_msg.aacx, mbox_msg.aacy, mbox_msg.aacz, mbox_msg.gyrox, mbox_msg.gyroy, mbox_msg.gyroz);// 发送加速度+陀螺仪原始数据
            usart1_report_imu((int)(mbox_msg.roll * 100), (int)(mbox_msg.pitch * 100), (int)(mbox_msg.yaw * 100), 0, 0);
        }
        
        vTaskGetInfo(currentTask, &USARTTaskInfo);
    }
}

// 中断服务程序中需要做的事情
// 在HAL库中所有的外部中断服务函数都会调用此函数
// GPIO_Pin:中断引脚号
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    delay_ms(100);      // 消抖
    switch(GPIO_Pin)
    {
        case GPIO_PIN_0:
            if (WK_UP == 1) 
            {
                if(Check_WKUP())
                {
                    Sys_Enter_Standby();//进入待机模式
                }
            }
            break;
        case GPIO_PIN_2:
            if (KEY1 == 0)  
            {
                report_USART = !report_USART;
            
                if (report_USART)
                {
                    vTaskWakeup(&USARTTask);
                    LCD_ShowString(30, 170, 200, 16, 16, "USART_UPLOAD ON ");
                }
                else
                {
                    if (USARTTask.waitEvent)
                    {
                        vEventRemoveTask(&USARTTask, (void *)0, vErrorDel);
                        vTaskSchedRdy(&USARTTask);
                    }
                    
                    if (USARTTask.state == VIDAOS_TASK_STATE_DELAYED)
                    {
                        vTDlistTaskWakeup(&USARTTask);
                        USARTTask.delayTicks = 0;
                    }
                    
                    vTaskSuspend(&USARTTask); 
                    LCD_ShowString(30, 170, 200, 16, 16, "USART_UPLOAD OFF");
                }
            }
            break;
        case GPIO_PIN_3:
            if (KEY0 == 0)  
            {
                report_CAN = !report_CAN;
            
                if (report_CAN)
                {
                    vTaskWakeup(&CANTask);
                    LCD_ShowString(30, 150, 200, 16, 16, "CAN_UPLOAD   ON ");
                }
                else 
                {
                    if (CANTask.waitEvent)
                    {
                        vEventRemoveTask(&CANTask, (void *)0, vErrorDel);
                        vTaskSchedRdy(&CANTask);
                    }
                    
                    if (CANTask.state == VIDAOS_TASK_STATE_DELAYED)
                    {
                        vTDlistTaskWakeup(&CANTask);
                        CANTask.delayTicks = 0;
                    }
                    
                    vTaskSuspend(&CANTask); 
                    LCD_ShowString(30, 150, 200, 16, 16, "CAN_UPLOAD   OFF");
                    LCD_ShowString(30, 340, 200, 16, 16, "               ");
                }
            }
            break;

        case GPIO_PIN_13:
            if (KEY2 == 0)  
            {
				update_LCD = !update_LCD;
            
                if (update_LCD)
                {
                    vTaskWakeup(&LCDTask);
                    LCD_ShowString(30, 190, 200, 16, 16, "LCD REFRESH  ON ");
                }
                else 
                {
                    if (LCDTask.waitEvent)
                    {
                        vEventRemoveTask(&LCDTask, (void *)0, vErrorDel);
                        vTaskSchedRdy(&LCDTask);
                    }
                    
                    if (LCDTask.state == VIDAOS_TASK_STATE_DELAYED)
                    {
                        vTDlistTaskWakeup(&LCDTask);
                        LCDTask.delayTicks = 0;
                    }
                    
                    vTaskSuspend(&LCDTask); 
                    LCD_ShowString(30, 190, 200, 16, 16, "LCD REFRESH  OFF");
                }
            }
            break;
    }
}

//回调函数，定时器中断服务函数调用
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (mpu_mpl_get_data(&mbox_msg.pitch, &mbox_msg.roll, &mbox_msg.yaw) == 0)
    {
        mbox_msg.temp = MPU_Get_Temperature();	// 得到温度值
        MPU_Get_Accelerometer(&mbox_msg.aacx, &mbox_msg.aacy, &mbox_msg.aacz);	// 得到加速度传感器数据
        MPU_Get_Gyroscope(&mbox_msg.gyrox, &mbox_msg.gyroy, &mbox_msg.gyroz);	// 得到陀螺仪数据
        
        if (htim->Instance == TIM3)
        {
            vMboxNotify(&mbox_UPLOAD, &mbox_msg, vMboxSentToAll);
        }
         
        // 更新LCD
        else if (htim->Instance == TIM4)
        {
            vMboxNotify(&mbox_LCD, &mbox_msg, vMboxStoreFront);
        }
    }
}
/**********************************************************************************************************
** Function name        :   vInitApp
** Descriptions         :   初始化应用接口
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/

void vInitApp(void)
{
    vTaskInit(&MPUTask, MPUTaskEntry, (void *)0x1, 0, MPUTaskStack, sizeof(MPUTaskStack));
    vTaskInit(&LCDTask, LCDTaskEntry, (void *)0x2, 1, LCDTaskStack, sizeof(LCDTaskStack));
    vTaskInit(&CANTask, CANTaskEntry, (void *)0x3, 1, CANTaskStack, sizeof(CANTaskStack));
    vTaskInit(&USARTTask, USARTTaskEntry, (void *)0x4, 1, USARTTaskStack, sizeof(USARTTaskStack));
}

