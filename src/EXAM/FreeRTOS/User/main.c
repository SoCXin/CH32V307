/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2020/12/1
* Description        : Main program body.
*******************************************************************************/

/*
 *@Note
 ���ڴ�ӡ�������̣�
 USART1_Tx(PA9)��
 ��������ʾʹ�� USART1(PA9) ����ӡ���Կ������

*/

#include "debug.h"
#include "Config.h"
#include "FreeRTOS.h"
#include "task.h"
/* Global typedef */

/* Global define */
//-------------���������ڲ���-------------
#define TASK1_TASK_PRIO     2           //---���ȼ�---
#define TASK1_STK_SIZE      128         //----����ջ���ȣ���Ҫע������M3��ͬ������ջͬʱҲ��Ҫ���ж�ʹ��----
TaskHandle_t Task1Task_Handler;         //---������---
#define TASK2_TASK_PRIO     2
#define TASK2_STK_SIZE      128
TaskHandle_t Task2Task_Handler;

/* Global Variable */

/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Return         : None
*******************************************************************************/
void task1_task(void *pvParameters)
{
    while(1)
    {
        printf("task1 entry\n");
        GPIO_SetBits(GPIOA, GPIO_Pin_0);
        vTaskDelay(250);
        GPIO_ResetBits(GPIOA, GPIO_Pin_0);
        vTaskDelay(250);
    }
}
void task2_task(void *pvParameters)
{
    while(1)
    {
        printf("task2 entry\n");
        GPIO_ResetBits(GPIOA, GPIO_Pin_1);
        vTaskDelay(250);
        GPIO_SetBits(GPIOA, GPIO_Pin_1);
        vTaskDelay(250);
    }
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);    //------����ʹ�÷���1��ֱ�Ӳ������ȼ��Ĵ���------
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n",SystemCoreClock);
	init_keyAndLed();                   //-----�������У�����Ϊ��ʼ��LED-----
    //---------------������������--------------------
	xTaskCreate((TaskFunction_t )task2_task,
	                    (const char*    )"task2_task",
	                    (uint16_t       )TASK2_STK_SIZE,
	                    (void*          )NULL,
	                    (UBaseType_t    )TASK2_TASK_PRIO,
	                    (TaskHandle_t*  )&Task2Task_Handler);

	xTaskCreate((TaskFunction_t )task1_task,
                    (const char*    )"task1_task",
                    (uint16_t       )TASK1_STK_SIZE,
                    (void*          )NULL,
                    (UBaseType_t    )TASK1_TASK_PRIO,
                    (TaskHandle_t*  )&Task1Task_Handler);
	//--------����������--------
	printf("StartScheduler\n");
	vTaskStartScheduler();
	while(1)
	{
	    ;
	}
}

