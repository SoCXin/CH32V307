/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2020/12/1
* Description        : Main program body.
*******************************************************************************/



#include "debug.h"
#include "Config.h"




/*******************************************************************************
* Function Name  : init_keyAndLed
* Description    : key led��ʼ��
* Input          : None
* Return         : None
*******************************************************************************/
void init_keyAndLed(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    //-----------LED INIT----------
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    //-------LED1 1_1 2 3 4-----
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //-----------touch key-----------
}




