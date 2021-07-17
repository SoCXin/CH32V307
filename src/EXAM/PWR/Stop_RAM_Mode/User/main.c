/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/24
* Description        : Main program body.
*******************************************************************************/

/*
 *@Note
 停止模式不开启LV时，RAM 2K、30K低功耗数据保持例程：
 本例程演示在 2K RAM和30K RAM指定位置处写入数据，之后WFI进入STOP睡眠模式，通过PA2引脚
 输入低电平触发外部中断 EXTI_Line2(PA2) 退出停止模式，唤醒后打印RAM数据，测试RAM是否保持住
 数据。

*/

#include "debug.h"

/* Global define */

/* Global Variable */

/*******************************************************************************
* Function Name  : TestDataWrite
* Description    : Write 0x11111111 to certain address of 2K RAM and 30K RAM.
* Input          : None
* Return         : None
*******************************************************************************/
void TestDataWrite(void)
{
    uint32_t myAddr=0x20000000+10*1024;
    uint32_t myAddr2=0x20000000+1*1024+512;
    uint32_t i=0;
    for(i=0;i<1024;i++)//Write 30K RAM
    {
        *(uint32_t volatile *)(myAddr+(i<<2)) = 0x11111111;
    }
    for(i=0;i<1024;i++)//Check 30K RAM
    {
        if((*(uint32_t volatile *)(myAddr+(i<<2))) != 0x11111111)
        {
            printf("Write 30K RAM Error!\r\n");
            break;
        }
    }
    printf("Write 30K RAM Successfully!\r\n");
    for(i=0;i<10;i++)//Write 2K RAM
    {
        *(uint32_t volatile *)(myAddr2+(i<<2)) = 0x11111111;
    }
    for(i=0;i<10;i++)//Check 30K RAM
    {
        if((*(uint32_t volatile *)(myAddr2+(i<<2))) != 0x11111111)
        {
            printf("Write 2K RAM Error!\r\n");
            break;
        }
    }
    printf("Write 2K RAM Successfully!\r\n");

}

/*******************************************************************************
* Function Name  : TestDataRead
* Description    : Print the data of certain address of 2K RAM and 30K RAM.
* Input          : None
* Return         : None
*******************************************************************************/
void TestDataRead(void)
{
    uint32_t myAddr=0x20000000+10*1024;
    uint32_t myAddr2=0x20000000+1*1024+512;
    uint32_t i=0;
    printf("2K RAM:\r\n");
    for(i=0;i<10;i++)
    {
        printf("0x%08lx=0x%08lx\r\n",myAddr2+(i<<2),*(uint32_t volatile *)(myAddr2+(i<<2)));
    }
    printf("32K RAM:\r\n");
    for(i=0;i<1024;i++)
    {
        printf("0x%08lx=0x%08lx\r\n",myAddr+(i<<2),*(uint32_t volatile *)(myAddr+(i<<2)));
    }

}

/*******************************************************************************
* Function Name  : EXTI_INT_INIT
* Description    : Initializes EXTI0 collection.
* Input          : None
* Return         : None
*******************************************************************************/
void EXTI2_INT_INIT(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOA,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* GPIOA.0 ----> EXTI_Line0 */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource2);
  EXTI_InitStructure.EXTI_Line=EXTI_Line2;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Return         : None
*******************************************************************************/
int main(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure unused GPIO as IPU or IPD to reduce power consumption */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|
             RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;

    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    Delay_Init();
    USART_Printf_Init(115200);

    EXTI2_INT_INIT();

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* Delay 1.5s to avoid entering low power mode immediately after reset*/
    Delay_Ms(1500);
    while(1)
    {
        TestDataRead();
        printf("1.Stop Mode Test\r\n");
        TestDataWrite();
        PWR_EnterSTOPMode_RAM(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
        printf("\r\n1.Out \r\n");
        printf("\r\n ########## \r\n");

    }
}
