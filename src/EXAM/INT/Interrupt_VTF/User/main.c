/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
*******************************************************************************/

/*
 *@Note
  使用VTF IRQ中断例程：
   本例用于演示VTF IRQ
*/
#include "debug.h"

/* Global define */


/* Global Variable */ 
uint32_t time=0;

void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*******************************************************************************
* Function Name  : Interrupt_VTF_Init
* Description    : Initializes VTF.
* Input          : None
* Return         : None
*******************************************************************************/
void Interrupt_VTF_Init(void)
{
    NVIC_EnableIRQ(SysTicK_IRQn);
    SetVTFIRQ((u32)SysTick_Handler,SysTicK_IRQn,0,ENABLE);
}

/*******************************************************************************
* Function Name  : Systick_Init
* Description    : Initializes Systick.
* Input          : None
* Return         : None
*******************************************************************************/
void Systick_Init(void)
{
    SysTick->SR=0;
    SysTick->CNT=0;
    SysTick->CMP=0x20;
    SysTick->CTLR=0x7;
}

/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Return         : None
*******************************************************************************/
int main(void)
{
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n",SystemCoreClock);
	printf("Interrupt VTF Test\r\n");

	Interrupt_VTF_Init();
	Systick_Init();
    while(1)
   {

   }
}


/*******************************************************************************
* Function Name  : SysTick_Handler
* Description    : This function handles SysTick exception.
* Input          : None
* Return         : None
*******************************************************************************/
void SysTick_Handler(void)
{
    time=SysTick->CNT;
    SysTick->CTLR=0;
    SysTick->SR=0;
    printf("delta time:%d\r\n",time-0x20);
}


