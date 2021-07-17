/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
*******************************************************************************/

/*
 *@Note
  1.本例程采用同步触发，演示中断嵌套的运用，最大嵌套8级，硬件压栈只
           保存低三级，高优先级的5级中断需要使用软件压栈，低优先级的3级可以
           使用硬件压栈或软件压栈。

  2.若只使用硬件压栈，可配置嵌套等级为4和硬件压栈溢出时关闭全局
           中断(启动文件中配置CSR(0x804),配置值由0x1f修改为0x0b)

  3.若不使用硬件压栈，配置CSR(0x804)的bit0清0，且中断函数声明去掉
   "WCH-Interrupt-fast"关键字

*/
#include "debug.h"

/* Global define */


/* Global Variable */ 


void WWDG_IRQHandler(void)   __attribute__((interrupt("WCH-Interrupt-fast")));
void PVD_IRQHandler(void)    __attribute__((interrupt("WCH-Interrupt-fast")));
void TAMPER_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void RTC_IRQHandler(void)    __attribute__((interrupt(/*"WCH-Interrupt-fast"*/)));
void FLASH_IRQHandler(void)  __attribute__((interrupt(/*"WCH-Interrupt-fast"*/)));
void RCC_IRQHandler(void)    __attribute__((interrupt(/*"WCH-Interrupt-fast"*/)));
void EXTI0_IRQHandler(void)  __attribute__((interrupt(/*"WCH-Interrupt-fast"*/)));
void EXTI1_IRQHandler(void)  __attribute__((interrupt(/*"WCH-Interrupt-fast"*/)));

/*******************************************************************************
* Function Name  : Interrupt_Init
* Description    : Initializes interruption.
* Input          : None
* Return         : None
*******************************************************************************/
void Interrupt_Init(void)
{
    NVIC_EnableIRQ(WWDG_IRQn);
    NVIC_EnableIRQ(PVD_IRQn);
    NVIC_EnableIRQ(TAMPER_IRQn);
    NVIC_EnableIRQ(RTC_IRQn);
    NVIC_EnableIRQ(FLASH_IRQn);
    NVIC_EnableIRQ(RCC_IRQn);
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);

    NVIC_SetPriority(WWDG_IRQn,  (7<<5) | (0x01<<4)); /* 组优先级7，子优先级1，总优先级较低 */
    NVIC_SetPriority(PVD_IRQn,   (6<<5) | (0x01<<4));
    NVIC_SetPriority(TAMPER_IRQn,(5<<5) | (0x01<<4));
    NVIC_SetPriority(RTC_IRQn,   (4<<5) | (0x01<<4));
    NVIC_SetPriority(FLASH_IRQn, (3<<5) | (0x01<<4));
    NVIC_SetPriority(RCC_IRQn,   (2<<5) | (0x01<<4));
    NVIC_SetPriority(EXTI0_IRQn, (1<<5) | (0x01<<4));
    NVIC_SetPriority(EXTI1_IRQn, (0<<5) | (0x01<<4));/* 组优先级0，子优先级1，总优先级较高 */
}



/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Return         : None
*******************************************************************************/
uint8_t step=1;
int main(void)
{
    Delay_Init();
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n",SystemCoreClock);
	printf("Interrupt Nest Test\r\n");
	Interrupt_Init();
	printf("Enter lowest interrupt\r\n");
    NVIC_SetPendingIRQ(WWDG_IRQn);

    printf("Quit lowest interrupt\r\n");
    printf("Quit step:%d\r\n",step);

    while(1)
   {

   }
}

void WWDG_IRQHandler(void)
{
    step++;
    printf(" 1.WWDG_IRQHandler");
    printf("  step:%d\r\n",step);

    NVIC_SetPendingIRQ(PVD_IRQn);
}

void PVD_IRQHandler(void)
{
    step++;
    printf(" 2.PVD_IRQHandler");
    printf("  step:%d\r\n",step);

    NVIC_SetPendingIRQ(TAMPER_IRQn);
}
void TAMPER_IRQHandler(void)
{
    step++;
    printf(" 3.TAMPER_IRQHandler");
    printf("  step:%d\r\n",step);

    NVIC_SetPendingIRQ(RTC_IRQn);
}
void RTC_IRQHandler(void)
{
    step++;
    printf(" 4.RTC_IRQHandler");
    printf("  step:%d\r\n",step);

    NVIC_SetPendingIRQ(FLASH_IRQn);
}
void FLASH_IRQHandler(void)
{
    step++;
    printf(" 5.RTC_IRQHandler");
    printf("  step:%d\r\n",step);

    NVIC_SetPendingIRQ(RCC_IRQn);
}
void RCC_IRQHandler(void)
{
    step++;
    printf(" 6.RCC_IRQHandler");
    printf("  step:%d\r\n",step);

    NVIC_SetPendingIRQ(EXTI0_IRQn);
}
void EXTI0_IRQHandler(void)
{
    step++;
    printf(" 7.EXTI0_IRQHandler");
    printf("  step:%d\r\n",step);

    NVIC_SetPendingIRQ(EXTI1_IRQn);
}
void EXTI1_IRQHandler(void)
{
    step++;
    printf(" 8.EXTI1_IRQHandler");
    printf("  step:%d\r\n",step);

}




