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
	float pitch, roll, yaw; 	    // ŷ����
	short aacx, aacy, aacz;	        // ���ٶȴ�����ԭʼ����
	short gyrox, gyroy, gyroz;      // ������ԭʼ���� 
	short temp;		                // �¶�    
}MPU_MBOX_MSG;

MPU_MBOX_MSG mbox_msg;

u8 report_CAN = 1;	                // Ĭ�Ͽ���CAN�ϱ�
u8 report_USART = 1;                // Ĭ�Ͽ���USART�ϱ�
u8 update_LCD = 1;                  // Ĭ�Ͽ���LCD����

// ����1����1���ַ� 
// c:Ҫ���͵��ַ�
void usart1_send_char(u8 c)
{
    while (__HAL_UART_GET_FLAG(&UART1_Handler, UART_FLAG_TC) == RESET){}; 
    USART1->DR = c;
} 

// �������ݸ�����������λ�����(V2.6�汾)
// fun:������. 0X01~0X1C
// data:���ݻ�����,���28�ֽ�!!
// len:data����Ч���ݸ���
void usart1_niming_report(u8 fun, u8 *data, u8 len)
{
	u8 send_buf[32];
	u8 i;
    
	if (len > 28) return;	// ���28�ֽ����� 
    
	send_buf[len + 3] = 0;	// У��������
	send_buf[0] = 0XAA;	// ֡ͷ
	send_buf[1] = 0XAA;	// ֡ͷ
	send_buf[2] = fun;	// ������
	send_buf[3] = len;	// ���ݳ���
    
	for (i = 0; i < len; i++)
    {
        send_buf[4 + i] = data[i];			// ��������
    }
    
    send_buf[len + 4] = 0;
    
	for (i = 0;i < len + 4; i++)
    {
        send_buf[len + 4] += send_buf[i];	// ����У���
    }
    
	for (i = 0;i < len + 5; i++)
    {
        usart1_send_char(send_buf[i]);	// �������ݵ�����1 
    }
}

// ���ͼ��ٶȴ���������+����������(������֡)
// aacx,aacy,aacz:x,y,z������������ļ��ٶ�ֵ
// gyrox,gyroy,gyroz:x,y,z�������������������ֵ 
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
	tbuf[12] = 0;// ��Ϊ����MPL��,�޷�ֱ�Ӷ�ȡ����������,��������ֱ�����ε�.��0���.
	tbuf[13] = 0;
	tbuf[14] = 0;
	tbuf[15] = 0;
	tbuf[16] = 0;
	tbuf[17] = 0;
    
    usart1_niming_report(0X02, tbuf, 18);// ������֡,0X02
}

// ͨ������1�ϱ���������̬���ݸ�����(״̬֡)
// roll:�����.��λ0.01�ȡ� -18000 -> 18000 ��Ӧ -180.00  ->  180.00��
// pitch:������.��λ 0.01�ȡ�-9000 - 9000 ��Ӧ -90.00 -> 90.00 ��
// yaw:�����.��λΪ0.1�� 0 -> 3600  ��Ӧ 0 -> 360.0��
// csb:�������߶�,��λ:cm
// prs:��ѹ�Ƹ߶�,��λ:mm
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
	usart1_niming_report(0X01, tbuf, 12);// ״̬֡,0X01
}  

void MPUTaskEntry(void *param)
{
    vMboxInit(&mbox_UPLOAD, mboxMsgBuffer_UPLOAD, 20);
    vMboxInit(&mbox_LCD, mboxMsgBuffer_LCD, 20);
    
    EXTI_Init();                            // �ⲿ�жϳ�ʼ��
    IWDG_Init(IWDG_PRESCALER_64, 2500);  	// ��Ƶ��Ϊ64,����ֵΪ2500,���ʱ��Ϊ5s	

    TIM3_Init(200-1,9000-1);        //��ʱ��3��ʼ������ʱ��ʱ��Ϊ90M����Ƶϵ��Ϊ9000-1��
                                    //���Զ�ʱ��3��Ƶ��Ϊ90M/9000=10K���Զ���װ��Ϊ200-1����ô��ʱ�����ھ���20ms
    TIM4_Init(1000-1,9000-1);       //��ʱ��3��ʼ������ʱ��ʱ��Ϊ90M����Ƶϵ��Ϊ9000-1��
                                    //���Զ�ʱ��3��Ƶ��Ϊ90M/9000=10K���Զ���װ��Ϊ1000-1����ô��ʱ�����ھ���100ms
 	 
	while (mpu_dmp_init())   
	{
		LCD_ShowString(30, 130, 200, 16, 16, "MPU9250 Error");
		delay_ms(200);
		LCD_Fill(30, 130, 239, 130 + 16, WHITE);
 		delay_ms(200);
		LED0 = !LED0;// DS0��˸ 
	}

    while (1)
    {            
        if (report_CAN || report_USART)
        {
            HAL_NVIC_EnableIRQ(TIM3_IRQn);               // ����ITM3�ж�   
        }
        else
        {
            HAL_NVIC_DisableIRQ(TIM3_IRQn);              // ����ITM3�ж�   
        }   

        if (update_LCD)
        {
            HAL_NVIC_EnableIRQ(TIM4_IRQn);               // ����ITM3�ж�   
        }
        else
        {
            HAL_NVIC_DisableIRQ(TIM4_IRQn);              // ����ITM3�ж�   
        }
                
        LED0 = !LED0;// DS0��˸ 

        vTaskGetInfo(currentTask, &MPUTaskInfo);
        IWDG_Feed();    //ι��
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
    
    TPAD_Init(2);                   //��ʼ����������,��90/8=11.25MhzƵ�ʼ���
    RTC_Init();                     //��ʼ��RTC 
    RTC_Set_WakeUp(RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 0); //����WAKE UP�ж�,1�����ж�һ��  
    
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
            // ��ʾ�¶�
            if (mbox_msg.temp < 0)
            {
                LCD_ShowChar(70 + 48, 220, '-', 16, 0);		// ��ʾ����
                mbox_msg.temp = -mbox_msg.temp;		// תΪ����
            }
            else 
            {
                LCD_ShowChar(70 + 48, 220, ' ', 16, 0);		// ȥ������ 
            }
            
            LCD_ShowNum(70 + 48 + 8, 220, mbox_msg.temp / 100, 3, 16);		// ��ʾ��������	    
            LCD_ShowNum(70 + 48 + 40, 220, mbox_msg.temp % 10, 1, 16);		// ��ʾС������ 
            
            // ��ʾroll
            temp = mbox_msg.roll * 10;
            
            if (temp < 0)
            {
                LCD_ShowChar(70 + 48, 240, '-', 16, 0);		// ��ʾ����
                temp = -temp;		// תΪ����
            }
            else
            {
                LCD_ShowChar(70 + 48, 240, ' ', 16, 0);		// ȥ������ 
            }
            
            LCD_ShowNum(70 + 48 + 8, 240, temp / 10, 3, 16);		// ��ʾ��������	    
            LCD_ShowNum(70 + 48 + 40, 240, temp % 10, 1, 16);		// ��ʾС������ 
            
            // ��ʾpitch
            temp = mbox_msg.pitch * 10;
            
            if (temp < 0)
            {
                LCD_ShowChar(70 + 48, 260, '-', 16, 0);		// ��ʾ����
                temp = -temp;		// תΪ����
            }
            else 
            {
                LCD_ShowChar(70 + 48, 260, ' ', 16, 0);		// ȥ������ 
            }
               
            LCD_ShowNum(70 + 48 + 8, 260, temp / 10, 3, 16);		// ��ʾ��������	    
            LCD_ShowNum(70 + 48 + 40, 260, temp % 10, 1, 16);		// ��ʾС������ 
            
            // ��ʾyaw
            temp = mbox_msg.yaw * 10;
            
            if (temp < 0)
            {
                LCD_ShowChar(70 + 48, 280, '-', 16, 0);		// ��ʾ����
                temp = -mbox_msg.temp;		// תΪ����
            }
            else
            {
                LCD_ShowChar(70 + 48, 280, ' ', 16, 0);		// ȥ������
            }
            
            LCD_ShowNum(70 + 48 + 8, 280, temp / 10, 3, 16);		// ��ʾ��������	    
            LCD_ShowNum(70 + 48 + 40, 280, temp % 10, 1, 16);		// ��ʾС������  
        }
        
        cpuUsage = vCPUUsageGet();
        temp = cpuUsage * 10;
        LCD_ShowString(70, 310, 200, 16, 16, "  CPU:    .  %");	
        LCD_ShowNum(70 + 48 + 8, 310, temp / 10, 3, 16);		// ��ʾ��������	    
        LCD_ShowNum(70 + 48 + 40, 310, temp % 10, 1, 16);		// ��ʾС������
        

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
        
        if (TPAD_Scan(0))    //�ɹ�������һ��������(�˺���ִ��ʱ������15ms)
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
    
    CAN1_Mode_Init(CAN_SJW_1TQ, CAN_BS2_6TQ, CAN_BS1_8TQ, 6, CAN_MODE_NORMAL); // CAN��ʼ��,������500Kbps    
    
    for (;;)
    {
        err = vMboxWait(&mbox_UPLOAD, &msg, 0);
        if (err == vErrorNoError) 
        {
            MPU_MBOX_MSG mbox_msg = *(MPU_MBOX_MSG*)msg;
            
            msg = &mbox_msg.aacx;
            res = CAN1_Send_Msg((u8 *)msg, 2, 0x01, 0x01);// ����8���ֽ� 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send Failed");		// ��ʾ����ʧ��
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send OK    ");	 		// ��ʾ���ͳɹ�	
            }
            
            msg = &mbox_msg.aacy;
            res = CAN1_Send_Msg((u8 *)msg, 2, 0x02, 0x02);// ����8���ֽ� 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send Failed");		// ��ʾ����ʧ��
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send OK    ");	 		// ��ʾ���ͳɹ�	
            }
            
            msg = &mbox_msg.aacz;
            res = CAN1_Send_Msg((u8 *)msg, 2, 0x03, 0x03);// ����8���ֽ� 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send Failed");		// ��ʾ����ʧ��
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send OK    ");	 		// ��ʾ���ͳɹ�	
            }
            
            msg = &mbox_msg.gyrox;
            res = CAN1_Send_Msg((u8 *)msg, 2, 0x04, 0x04);// ����8���ֽ� 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send Failed");		// ��ʾ����ʧ��
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send OK    ");	 		// ��ʾ���ͳɹ�	
            }
            
            msg = &mbox_msg.gyroy;
            res = CAN1_Send_Msg((u8 *)msg, 2, 0x05, 0x05);// ����8���ֽ� 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send Failed");		// ��ʾ����ʧ��
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send OK    ");	 		// ��ʾ���ͳɹ�	
            }
            
            msg = &mbox_msg.gyroz;
            res = CAN1_Send_Msg((u8 *)msg, 2, 0x06, 0x06);// ����8���ֽ� 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send Failed");		// ��ʾ����ʧ��
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16, "CAN Send OK    ");	 		// ��ʾ���ͳɹ�	
            }
            
            msg = &mbox_msg.roll;
            res = CAN1_Send_Msg((u8 *)msg, 4, 0x07, 0x07);// ����8���ֽ� 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16,"CAN Send Failed");		// ��ʾ����ʧ��
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16,"CAN Send OK    ");	 		// ��ʾ���ͳɹ�	
            }
            
            msg = &mbox_msg.pitch;
            res = CAN1_Send_Msg((u8 *)msg, 4, 0x08, 0x08);// ����8���ֽ� 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16,"CAN Send Failed");		// ��ʾ����ʧ��
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16,"CAN Send OK    ");	 		// ��ʾ���ͳɹ�	
            }
            
            msg = &mbox_msg.yaw;
            res = CAN1_Send_Msg((u8 *)msg, 4, 0x09, 0x09);// ����8���ֽ� 
            
			if (res)
            {
                LCD_ShowString(30, 340, 200, 16, 16,"CAN Send Failed");		// ��ʾ����ʧ��
            }
			else 
            {
                LCD_ShowString(30, 340, 200, 16, 16,"CAN Send OK    ");	 		// ��ʾ���ͳɹ�	
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
            mpu6050_send_data(mbox_msg.aacx, mbox_msg.aacy, mbox_msg.aacz, mbox_msg.gyrox, mbox_msg.gyroy, mbox_msg.gyroz);// ���ͼ��ٶ�+������ԭʼ����
            usart1_report_imu((int)(mbox_msg.roll * 100), (int)(mbox_msg.pitch * 100), (int)(mbox_msg.yaw * 100), 0, 0);
        }
        
        vTaskGetInfo(currentTask, &USARTTaskInfo);
    }
}

// �жϷ����������Ҫ��������
// ��HAL�������е��ⲿ�жϷ�����������ô˺���
// GPIO_Pin:�ж����ź�
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    delay_ms(100);      // ����
    switch(GPIO_Pin)
    {
        case GPIO_PIN_0:
            if (WK_UP == 1) 
            {
                if(Check_WKUP())
                {
                    Sys_Enter_Standby();//�������ģʽ
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

//�ص���������ʱ���жϷ���������
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (mpu_mpl_get_data(&mbox_msg.pitch, &mbox_msg.roll, &mbox_msg.yaw) == 0)
    {
        mbox_msg.temp = MPU_Get_Temperature();	// �õ��¶�ֵ
        MPU_Get_Accelerometer(&mbox_msg.aacx, &mbox_msg.aacy, &mbox_msg.aacz);	// �õ����ٶȴ���������
        MPU_Get_Gyroscope(&mbox_msg.gyrox, &mbox_msg.gyroy, &mbox_msg.gyroz);	// �õ�����������
        
        if (htim->Instance == TIM3)
        {
            vMboxNotify(&mbox_UPLOAD, &mbox_msg, vMboxSentToAll);
        }
         
        // ����LCD
        else if (htim->Instance == TIM4)
        {
            vMboxNotify(&mbox_LCD, &mbox_msg, vMboxStoreFront);
        }
    }
}
/**********************************************************************************************************
** Function name        :   vInitApp
** Descriptions         :   ��ʼ��Ӧ�ýӿ�
** parameters           :   ��
** Returned value       :   ��
***********************************************************************************************************/

void vInitApp(void)
{
    vTaskInit(&MPUTask, MPUTaskEntry, (void *)0x1, 0, MPUTaskStack, sizeof(MPUTaskStack));
    vTaskInit(&LCDTask, LCDTaskEntry, (void *)0x2, 1, LCDTaskStack, sizeof(LCDTaskStack));
    vTaskInit(&CANTask, CANTaskEntry, (void *)0x3, 1, CANTaskStack, sizeof(CANTaskStack));
    vTaskInit(&USARTTask, USARTTaskEntry, (void *)0x4, 1, USARTTaskStack, sizeof(USARTTaskStack));
}

