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

#define IN_TRANSFER           0
#define OUT_TRANSFER          0
#define INISO_TRANSFER        0
#define OUTISO_TRANSFER       0
UINT32 Recv_seq=0;
UINT32 checknum=0;
UINT16 packnum=0;
UINT8 usbtestbuf[100];
UINT8 CompareBuff[64*256];
extern PUSBHS_HOST Host_Status;

/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Return         : None
*******************************************************************************/
int main(void)
{

    UINT32 ReLen;
    UINT32 RxLen=0;

	Delay_Init();
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n",SystemCoreClock);

	USBHS_HostInit(ENABLE);

	Host_Status=(PUSBHS_HOST)usbtestbuf;
	Host_Status->DEVENDP.InTog=0;
	Host_Status->DEVENDP.OutTog=0;
	while(1)
	{
	    if(FoundNewDev==1)
	    {
	        FoundNewDev=0;
	        printf("new device in\n");
	        USBHS_HostEnum();
	    }
	    if (DevEnum==1)
	    {
	        DevEnum=0;
            break;
        }
	}
	while(1)
	{
#if IN_TRANSFER
	  if(USBHS->STATUS &USBHS_SOF_ACT)
	  {
            USBHS->STATUS =USBHS_SOF_ACT;
            USBHS->UEP2_RX_DMA = (UINT32)&endpRXbuff[0];                     // set host RX DMA address
            ReLen=USBHS_INTransact(DEF_ENDP_1,Host_Status->DEVENDP.InTog);
            if((ReLen ==USBHS_TOGGLE_ERR)||(ReLen==USBHS_HARDTIMEOUT))               //hardware timeout
            {
                printf("error\n");
                break;
            }
            RxLen+=ReLen;
            Host_Status->DEVENDP.InTog=(Host_Status->DEVENDP.InTog)?0:1;

            packnum++;
            if(packnum>10000)
            {
                printf("success\n");
                break;
            }
	  }
#endif
#if OUT_TRANSFER
	  if(USBHS->STATUS &USBHS_SOF_ACT)
      {
            USBHS->UEP3_TX_DMA = (UINT32)&endpTXbuff[0];                     // set host TX DMA address
            ReLen=USBHS_OUTTransact(DEF_ENDP_1,Host_Status->DEVENDP.OutEndpMaxSize,Host_Status->DEVENDP.OutTog);
            if(ReLen != TRANS_SUCCESS)               //hardware timeout
            {
               break;
            }
            Host_Status->DEVENDP.OutTog=Host_Status->DEVENDP.OutTog?0:1;
            packnum++;
            if(packnum>10000)
            {
                printf("success\n");
                break;
            }
	    }
#endif
	}
	while(1);
}







