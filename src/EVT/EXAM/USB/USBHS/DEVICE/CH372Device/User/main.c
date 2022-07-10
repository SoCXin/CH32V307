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
  模拟自定义USB设备（CH372设备）例程：
  本例程演示使用 USBHS模拟自定义设备 CH372，端点1，2可自由上下传，下传数据将复制到上传，批量设备，最大包长512，
  可使用Bushund或其它上位机软件对设备进行操作。其它端点可以参考端点1,2操作。
*/

#include "ch32v30x_usbhs_device.h"
#include "debug.h"

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

    /* USB20 device init */
    USBHS_RCC_Init( );                                                         /* USB2.0高速设备RCC初始化 */
    USBHS_Device_Init( ENABLE );
    NVIC_EnableIRQ( USBHS_IRQn );

	while(1)
	{ }
}
