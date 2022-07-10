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
  USBHS�ļ���ö�ٹ�������U���ļ�������дʾ������
    ��ʾ����/C51/CH559HFT.C�ļ�������ҵ������������ļ����ݣ�
    ��ʾ����/LIUNEWFILE.TXT�ļ���ѭ������д��ָ���ַ��������޸��ļ����ԣ�
    ��ʾɾ��/OLD.TXT�ļ���
    ֧��: FAT12/FAT16/FAT32
*/

#include "debug.h"
#include "ch32v30x_usbhs_host.h"
#include "Desc_Analysis.h"

__attribute__ ((aligned(4))) UINT8   endpTXbuf[ MAX_PACKET_SIZE ];  // OUT, must even address
__attribute__ ((aligned(4))) UINT8   endpRXbuf[ MAX_PACKET_SIZE ];  // OUT, must even addres

/*********************************************************************
 * @fn      Timer3_Init
 *
 * @brief
 *
 * @return  none
 */
void Timer3_Init( )
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure = { 0 };
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM3, ENABLE ); /* ��TIM3ʱ��  */

    TIM_TimeBaseStructure.TIM_Period = 99; /* ����ֵ */
    TIM_TimeBaseStructure.TIM_Prescaler = 1440; /* Ԥ��Ƶֵ */
    TIM_TimeBaseStructure.TIM_ClockDivision = 0; /* ʱ�ӷָ�  */
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  /* ��������  */
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ARRPreloadConfig(TIM3, DISABLE);                               //��ֹARRԤװ�ػ�����
    TIM_ITConfig(TIM3, TIM_IT_Update ,ENABLE );                        /* ʹ�ܸ����ж� */
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    TIM_Cmd(TIM3, ENABLE);
}

/*********************************************************************
 * @fn      Test_IO_INIT
 *
 * @brief   Configuring GPIO For Test
 *
 * @return  none
 */
void Test_IO_INIT( )
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_SetBits( GPIOE, GPIO_Pin_8 );
}

/*********************************************************************
 * @fn      Test_IO_INIT
 *
 * @brief   Configuring GPIO For Test
 *
 * @return  none
 */
void Clear_HOST_CTL_Struct( UINT8 index )
{
    UINT8 i, j;

    HostCtl[index].DeviceAddr = 0xFF;
    HostCtl[index].DeviceState = 0xFF;
    HostCtl[index].DeviceSpeed = 0xFF;
    HostCtl[index].DeviceType = 0xFF;

    HostCtl[index].InterfaceNum = 0x00;
    HostCtl[index].HubPortNum = 0x00;
    HostCtl[index].HubInAddr = 0x00;
    HostCtl[index].ErrorCount = 0x00;

    for( i=0; i<DEF_INTERFACE_NUM_MAX; i++ )
    {
        HostCtl[index].Interface[i].Type = 0x00;
        HostCtl[index].Interface[i].HidReportID = 0x00;
        HostCtl[index].Interface[i].Full_KB_Flag = 0x00;
        HostCtl[index].Interface[i].HidDescLen = 0x00;

        HostCtl[index].Interface[i].InEndpNum = 0x00;
        HostCtl[index].Interface[i].OutEndpNum = 0x00;
        for( j=0; j<DEF_ENDP_NUM_MAX; j++ )
        {
            HostCtl[index].Interface[i].InEndpAddr[j] = 0x00;
            HostCtl[index].Interface[i].InEndpSize[j] = 0x00;
            HostCtl[index].Interface[i].InEndpType[j] = 0x00;
            HostCtl[index].Interface[i].InEndpTog[j] = 0x00;
            HostCtl[index].Interface[i].InEndpInterval[j] = 0x00;
            HostCtl[index].Interface[i].InEndpTimeCount[j] = 0x00;

            HostCtl[index].Interface[i].OutEndpAddr[j] = 0x00;
            HostCtl[index].Interface[i].OutEndpType[j] = 0x00;
            HostCtl[index].Interface[i].OutEndpSize[j] = 0x00;
            HostCtl[index].Interface[i].OutEndpTog[j] = 0x00;
        }

        HostCtl[index].Interface[i].IDFlag = 0x00;
        HostCtl[index].Interface[i].Media_Sp_Flag = 0x00;

        HostCtl[index].Interface[i].Disbility = 0x00;
        HostCtl[index].Interface[i].Button = 0x00;
        HostCtl[index].Interface[i].Desktop_X = 0x00;
        HostCtl[index].Interface[i].Desktop_Y = 0x00;
        HostCtl[index].Interface[i].Wheel = 0x00;
        HostCtl[index].Interface[i].Asolute = 0x00;
        HostCtl[index].Interface[i].Mouse_ReportID = 0x00;

        HostCtl[index].Interface[i].NoSetReport_Flag = 0x00;
    }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    UINT8  ret;
    UINT8  endp0outdata[16];
    UINT8  keybufdata[16];
    UINT16 i, j, k;
    UINT16 desclen;
    UINT8  lockflag = 0;
    UINT8  len;
	Delay_Init();
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n",SystemCoreClock);
	printf("USBHS HOST KM Test\r\n");
	Timer3_Init( );
	USBHS_HostInit(ENABLE);
	while(1)
	{
        if( USBHSH->INT_FG & USBHS_DETECT_FLAG )
        {
            USBHSH->INT_FG = USBHS_DETECT_FLAG;
            if( USBHSH->MIS_ST & USBHS_ATTCH )
            {
                printf("Device In\r\n");
                /* Init Device structure */
                lockflag = 0;
                FoundNewDev = 1;
                for( i=0; i<16; i++ )
                {
                    keybufdata[i] = 0;
                    endp0outdata[i] = 0;
                }
                endp0outdata[ 0 ] = 0x01;
                Clear_HOST_CTL_Struct( 0 );
                ret = USBHS_HostEnum( endpRXbuf );
                HostCtl[0].DeviceState = ret;
                if( ret == ERR_SUCCESS )
                {
                    /* Analysis Device */
                    TIM_Cmd(TIM3, ENABLE);
                    printf( "Enum Succeed\r\n" );
                    printf( "Device Desc\r\n" );
                    desclen = DeviceDescriptor[0];
                    for( i=0; i<desclen ; i++ )
                    {
                        printf( "%02x ", DeviceDescriptor[i] );
                    }
                    printf( "\r\n" );
                    /* Analysis Configuration Descriptor */
                    printf( "Configuration Desc\r\n" );
                    desclen = (UINT16)ConfigDescriptor[2] + (UINT16)(ConfigDescriptor[3]<<8);
                    for( i=0; i<desclen ; i++ )
                    {
                        printf( "%02x ", ConfigDescriptor[i] );
                    }
                    printf( "\r\n" );
                    /* Analysis Device Descriptor */
                    KM_Analyse_ConfigDesc( 0 );
                    printf( "Device Status %02x\r\n", HostCtl[0].DeviceState );
                    printf( "Device Speed  %02x\r\n", HostCtl[0].DeviceSpeed );
                    printf( "Device Type   %02x\r\n", HostCtl[0].DeviceType );
                    printf( "Device Addr   %02x\r\n", HostCtl[0].DeviceAddr );
                    for( i=0; i<HostCtl[0].InterfaceNum; i++ )
                    {
                        printf( "InterFace Type %02x\r\n", HostCtl[0].Interface[i].Type );
                        for( j=0; j<HostCtl[0].Interface[i].InEndpNum; j++ )
                        {
                            printf( "EndpIn   %02x\r\n", j );
                            printf( "Addr     %02x\r\n", HostCtl[0].Interface[i].InEndpAddr[j] );
                            printf( "Type     %02x\r\n", HostCtl[0].Interface[i].InEndpType[j] );
                            printf( "Size     %02x\r\n", HostCtl[0].Interface[i].InEndpSize[j] );
                            printf( "InterVal %02x\r\n", HostCtl[0].Interface[i].InEndpInterval[j] );
                        }

                        for( j=0; j<HostCtl[0].Interface[i].OutEndpNum; j++ )
                        {
                            printf( "EndpOut  %02x\r\n", j );
                            printf( "Addr     %02x\r\n", HostCtl[0].Interface[i].OutEndpAddr[j] );
                            printf( "Type     %02x\r\n", HostCtl[0].Interface[i].OutEndpType[j] );
                            printf( "Size     %02x\r\n", HostCtl[0].Interface[i].OutEndpSize[j] );
                        }
                    }
                }
                else
                {
                    printf("enum error\r\n");
                    Clear_HOST_CTL_Struct( 0 );
                }
            }
            else
            {
                Clear_HOST_CTL_Struct( 0 );
                USBHS_HostInit(DISABLE);
                USBHS_HostInit(ENABLE);
                printf("disconnect\r\n");
                USBHSH->HOST_RX_DMA = (UINT32)&endpRXbuf[0];                 //host rx DMA address
                USBHSH->HOST_TX_DMA = (UINT32)&endpTXbuf[0];                 //host tx DMA address
            }
        }
        if( (HostCtl[0].DeviceState == 0x00) && (HostCtl[0].DeviceType = USB_DEV_CLASS_HID) )
        {
            /* Enter HID */
            if( TIM_GetITStatus(TIM3, TIM_IT_Update)  )
            {
                TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
                /* HID Data  Trans */
                for( i=0; i<HostCtl[0].InterfaceNum; i++ )
                {
                    for( j=0; j<HostCtl[0].Interface[i].InEndpNum; j++ )
                    {
                        HostCtl[0].Interface[i].InEndpTimeCount[j] ++;
                        if( HostCtl[0].Interface[i].InEndpTimeCount[j] >= HostCtl[0].Interface[i].InEndpInterval[j] )
                        {
                            HostCtl[0].Interface[i].InEndpTimeCount[j] = 0;
                            /* ʹ�õ����Ļ�������ַ */
                            USBHSH->HOST_RX_DMA = (UINT32)keybufdata;                                //���ý���DMA��ַ
                            ret = USBHostTransact( USB_PID_IN << 4 | HostCtl[0].Interface[i].InEndpAddr[j], HostCtl[0].Interface[i].InEndpTog[j], 0 );
                            if ( ret == ERR_SUCCESS )
                            {
                                HostCtl[0].Interface[i].InEndpTog[j] ^= UH_R_TOG_1;
                                if( USBHSH->RX_LEN > 0 )
                                {
                                    printf( "Recv " );
                                    for( k=0; k<USBHSH->RX_LEN; k++ )
                                    {
                                        printf( "%02x ", keybufdata[k] );
                                    }
                                    printf( "\r\n" );
                                    /* CapsLock */
//                                  NUMLOCK 0X01
//                                  CAPLOCK 0X02
//                                  SCROLLLOCK 0X04
                                    if( keybufdata[0x02] == 0x39 )
                                    {
                                       endp0outdata[ 0 ] ^= 0x02;
                                       lockflag = 1;
                                    }
                                    /* ScrollLock */
                                    else if( keybufdata[0x02] == 0x47 )
                                    {
                                       endp0outdata[ 0 ] ^= 0x04;
                                       lockflag = 1;
                                    }
                                    /* NumLock */
                                    else if( keybufdata[0x02] == 0x53 )
                                    {
                                       endp0outdata[ 0 ] ^= 0x01;
                                       lockflag = 1;
                                    }
                                    if( lockflag )
                                    {
                                        lockflag = 0;
                                        /* SetReport */
                                        CopySetupReqPkg( HIDSetReport );
                                        USBHSH->HOST_TX_DMA = (UINT32)endpTXbuf;
                                        USBHSH->HOST_TX_LEN = 8;
                                        ret = HostCtrlTransfer( endp0outdata, &len );
                                        if ( ret != ERR_SUCCESS )
                                        {
                                           printf( "err %02x\r\n", ret );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
	}
}




