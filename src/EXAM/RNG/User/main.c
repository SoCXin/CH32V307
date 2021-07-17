/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
*******************************************************************************/

/*
 *@Note
    ���������������
   ����32λ�����ͨ������(PA9)��ӡ��

*/

#include "debug.h"

/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Return         : None
*******************************************************************************/
int main(void)
{
    u32 random = 0;

    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n",SystemCoreClock);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_RNG, ENABLE);
    RNG_Cmd(ENABLE);

    while(1)
    {
        while(RNG_GetFlagStatus(RNG_FLAG_DRDY)==RESET)
        {
        }
        random=RNG_GetRandomNumber();
        printf("random:0x%08x\r\n",random);\
        Delay_Ms(500);
    }
}
