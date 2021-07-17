/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
*******************************************************************************/

/*
 *@Note
 FPUӲ�������������̣�
   ��������ʾӲ���������㡣

    ע������Ӳ������M-RS���òο� ����������
    ��������-Properties -> C/C++ Build -> Setting -> Target Processor
    -> Floating point ѡ�����ó� Single precision extension ��RVF��
       Floating point ABI ѡ�����ó� Single precision(f)

*/

#include "debug.h"

float val1=33.14;
/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Return         : None
*******************************************************************************/
int main(void)
{
    int t,t1;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n",SystemCoreClock);

	val1 = (val1/2+11.12)*2;
    t=(int)(val1*10)%10;
    t1=(int)(val1*100)%10;

	printf("%d.%d%d\n", (int)val1, t, t1);

	while(1);
}


