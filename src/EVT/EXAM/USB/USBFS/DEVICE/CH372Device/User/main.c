/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* SPDX-License-Identifier: Apache-2.0
*******************************************************************************/

/*
 *@Note
  ģ���Զ���USB�豸��CH372�豸�����̣�
 OTG_FS_DM(PA11)��OTG_FS_DP(PA12)
  ��������ʾʹ�� USBOTG-FSģ���Զ����豸 CH372���˵�1~7���������´����´����ݽ����Ƶ��ϴ��������豸��������64��
  ��ʹ��Bushund��������λ��������豸���в�����
*/

#include "debug.h"
#include "ch32v30x_usbotg_device.h"

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
	Delay_Init();
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n",SystemCoreClock);

    /* USBOTG_FS device init */
	printf( "CH372Device Running On USBOTG-FS Controller\n" );
	Delay_Ms(10);
	USBOTG_Init( );

	while(1)
	{ }
}
