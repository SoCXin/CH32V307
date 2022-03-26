/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
*******************************************************************************/

/*
 *@Note 
  USBHS设备的简易枚举过程例程：
  USBHS_DM(PB6)、USBHS_DM(PB7)

*/

#include "debug.h"
#include "ch32v30x_usbhs_host.h"


UINT8 usbtestbuf[sizeof(USBHS_HOST)];
extern PUSBHS_HOST Host_Status;
/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    UINT8 ret;

	Delay_Init();
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n",SystemCoreClock);

    Host_Status = (PUSBHS_HOST)usbtestbuf;
	USBHS_HostInit(ENABLE);

	while(1)
	{
        if(USBHSH->INT_FG & USBHS_DETECT_FLAG)
        {
            USBHSH->INT_FG = USBHS_DETECT_FLAG;
            if(USBHSH->MIS_ST & USBHS_ATTCH)
            {
                SetBusReset();
                ret = USBHS_HostEnum();
                if(ret == ERR_SUCCESS )
                {

                }
                else
                {
                    printf("enum error\n");
                }
            }
            else
            {
                USBHS_HostInit(DISABLE);
                USBHS_HostInit(ENABLE);
                printf("disconnect\n");
            }
        }
	}
}




