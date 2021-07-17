/********************************** (C) COPYRIGHT *******************************
* File Name          : ch32v30x_usbotg_host.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : USBOTG full speed host operation function.
*******************************************************************************/
#include "ch32vf30x_usbotg_host.h"


__attribute__ ((aligned(4))) const UINT8  SetupGetDevDescr[] = { USB_REQ_TYP_IN, USB_GET_DESCRIPTOR, 0x00, USB_DESCR_TYP_DEVICE, 0x00, 0x00, sizeof( USB_DEV_DESCR ), 0x00 };

__attribute__ ((aligned(4))) const UINT8  SetupGetCfgDescr[] = { USB_REQ_TYP_IN, USB_GET_DESCRIPTOR, 0x00, USB_DESCR_TYP_CONFIG, 0x00, 0x00, 0x04, 0x00 };

__attribute__ ((aligned(4))) const UINT8  SetupSetUsbAddr[] = { USB_REQ_TYP_OUT, USB_SET_ADDRESS, USB_DEVICE_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00 };

__attribute__ ((aligned(4))) const UINT8  SetupSetUsbConfig[] = { USB_REQ_TYP_OUT, USB_SET_CONFIGURATION, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

__attribute__ ((aligned(4))) const UINT8  SetupSetUsbInterface[] = { USB_REQ_RECIP_INTERF, USB_SET_INTERFACE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

__attribute__ ((aligned(4))) const UINT8  SetupClrEndpStall[] = { USB_REQ_TYP_OUT | USB_REQ_RECIP_ENDP, USB_CLEAR_FEATURE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
__attribute__ ((aligned(16))) UINT8 endpRXbuff[64+64];   //端点1数据收发缓冲区
__attribute__ ((aligned(16))) UINT8 endpTXbuff[64+64];   //端点1数据收发缓冲区

#define pSetupReq    ((PUSB_SETUP_REQ)endpTXbuff)

UINT8 UsbDevEndp0Size=0;
PUSBHS_HOST pOTGHost;
UINT8 FoundNewDev=0;
/*******************************************************************************
* Function Name  : OTG_RCC_Init
* Description    : USB OTG clock configuration.
* Input          : None
* Return         : None
*******************************************************************************/
void OTG_RCC_Init(void)
{
    /* usb20_phy时钟 */
    RCC->CFGR2 |= USB_48M_CLK_SRC_PHY;                                              //usbotg 时钟选择 0 = systick，1 = usb20_phy
    RCC->CFGR2 |= USBHS_PLL_SRC_HSE | USBHS_PLL_SRC_PRE_DIV2 | USBHS_PLL_CKREF_4M;  //PLL REF = HSE/2 = 4MHz
    RCC->CFGR2 |= USB_48M_CLK_SRC_PHY | USBHS_PLL_ALIVE;
    RCC->AHBPCENR |= RCC_AHBPeriph_USBHS;                                           //usbhs时钟使能
    RCC->AHBPCENR |= RCC_AHBPeriph_OTG_FS;                                          //usbotg时钟使能
}
/*******************************************************************************
* Function Name  : USBOTG_HostInit
* Description    : USB OTG host mode  initialized.
* Input          : ENABLE / DISABLE
* Return         : None
*******************************************************************************/
void USBOTG_HostInit(FunctionalState stus)
{
    if(stus==ENABLE)
    {
        OTG_RCC_Init();
        USBOTG_FS->BASE_CTRL=USBHD_UC_HOST_MODE;                  //host mode
        USBHD_UHOST_CTRL=0;
        USBOTG_FS->DEV_ADDR=0;
        USBHD_UH_EP_MOD=USBHD_UH_EP_TX_EN|USBHD_UH_EP_RX_EN;      //host tx&rx enable

        USBHD_UH_RX_DMA= (UINT32)&endpRXbuff[0];                  //host rx DMA address
        USBHD_UH_TX_DMA = (UINT32)&endpTXbuff[0];                 //host tx DMA address

        USBHD_UH_RX_CTRL=0;
        USBHD_UH_TX_CTRL=0;
        USBOTG_FS->BASE_CTRL=USBHD_UC_HOST_MODE|USBHD_UC_INT_BUSY|USBHD_UC_DMA_EN;

        USBOTG_FS->INT_FG=0xff;

        DisableRootHubPort();
        USBOTG_FS->INT_EN=USBHD_UIE_TRANSFER|USBHD_UIE_DETECT;
        FoundNewDev = 0;
    }

}
/*******************************************************************************
* Function Name  : DisableRootHubPort
* Description    : disable root hub.
* Input          : None
* Return         : None
*******************************************************************************/
void DisableRootHubPort(void)
{
#ifdef  FOR_ROOT_UDISK_ONLY
    CH103DiskStatus = DISK_DISCONNECT;

#endif

#ifndef DISK_BASE_BUF_LEN
    pOTGHost->DeviceStatus = ROOT_DEV_DISCONNECT;
    pOTGHost->DeviceAddress = 0x00;

#endif
}

/*******************************************************************************
* Function Name  : AnalyzeRootHub
* Description    : Analyze root hub state.
* Input          : None
* Return         : Error
*******************************************************************************/
UINT8 AnalyzeRootHub(void)
{
    UINT8   s;

    s = ERR_SUCCESS;

    if ( USBOTG_FS->MIS_ST & USBHD_UMS_DEV_ATTACH ) {

#ifdef DISK_BASE_BUF_LEN
        if ( CH103DiskStatus == DISK_DISCONNECT

#else
        if ( pOTGHost->DeviceStatus == ROOT_DEV_DISCONNECT

#endif
                || ( USBHD_UHOST_CTRL & USBHD_UH_PORT_EN ) == 0x00 ) {
                DisableRootHubPort( );

#ifdef DISK_BASE_BUF_LEN
                CH103DiskStatus = DISK_CONNECT;

#else
        pOTGHost->DeviceSpeed = USBOTG_FS->MIS_ST & USBHD_UMS_DM_LEVEL ? 0 : 1;
        pOTGHost->DeviceStatus = ROOT_DEV_CONNECTED;

#endif
        s = ERR_USB_CONNECT;
     }
  }

#ifdef DISK_BASE_BUF_LEN
  else if ( CH103DiskStatus >= DISK_CONNECT ) {

#else
  else if ( pOTGHost->DeviceStatus >= ROOT_DEV_CONNECTED ) {

#endif
        DisableRootHubPort( );
        if ( s == ERR_SUCCESS ) s = ERR_USB_DISCON;
  }

    return( s );
}

/*******************************************************************************
* Function Name  : SetHostUsbAddr
* Description    : Set USB host address.
* Input          : addr; host address.
* Return         : None
*******************************************************************************/
void SetHostUsbAddr( UINT8 addr )
{
    USBOTG_FS->DEV_ADDR = (USBOTG_FS->DEV_ADDR&USBHD_UDA_GP_BIT) | (addr&USBHD_USB_ADDR_MASK);
}

#ifndef FOR_ROOT_UDISK_ONLY
/*******************************************************************************
* Function Name  : SetUsbSpeed
* Description    : Set USB speed.
* Input          : FullSpeed: USB speed.
* Return         : None
*******************************************************************************/
void SetUsbSpeed( UINT8 FullSpeed )
{
    if ( FullSpeed )
    {
        USBOTG_FS->BASE_CTRL &= ~ USBHD_UC_LOW_SPEED;
//      R8_UH_SETUP &= ~ RB_UH_PRE_PID_EN;
        USBHD_UH_SETUP &= ~ USBHD_UH_PRE_PID_EN;

    }
    else
    {
        USBOTG_FS->BASE_CTRL |= USBHD_UC_LOW_SPEED;

    }
}
#endif

/*******************************************************************************
* Function Name  : ResetRootHubPort( )
* Description    : Reset root hub.
* Input          : None
* Return         : None
*******************************************************************************/
void ResetRootHubPort( void )
{
    UsbDevEndp0Size = DEFAULT_ENDP0_SIZE;
    SetHostUsbAddr( 0x00 );
    USBHD_UHOST_CTRL &= ~USBHD_UH_PORT_EN;
    SetUsbSpeed( 1 );
    USBHD_UHOST_CTRL = (USBHD_UHOST_CTRL & ~USBHD_UH_LOW_SPEED) | USBHD_UH_BUS_RESET;
    Delay_Ms( 50 );
    USBHD_UHOST_CTRL = USBHD_UHOST_CTRL & ~ USBHD_UH_BUS_RESET;
    Delay_Us( 250 );
    USBOTG_FS->INT_FG = USBHD_UIF_DETECT;
}

/*******************************************************************************
* Function Name  : EnableRootHubPort( )
* Description    : Enable root hub.
* Input          : None
* Return         : ERROR
*******************************************************************************/
UINT8 EnableRootHubPort(void)
{
#ifdef DISK_BASE_BUF_LEN
  if ( CH103DiskStatus < DISK_CONNECT ) CH103DiskStatus = DISK_CONNECT;

#else
  if ( pOTGHost->DeviceStatus < ROOT_DEV_CONNECTED ) pOTGHost->DeviceStatus = ROOT_DEV_CONNECTED;

#endif
  if ( USBOTG_FS->MIS_ST & USBHD_UMS_DEV_ATTACH ) {
#ifndef DISK_BASE_BUF_LEN
    if ( ( USBHD_UHOST_CTRL & USBHD_UH_PORT_EN ) == 0x00 ) {
           pOTGHost->DeviceSpeed = (USBOTG_FS->MIS_ST & USBHD_UMS_DM_LEVEL) ? 0 : 1;
            if ( pOTGHost->DeviceSpeed == 0 ) USBHD_UHOST_CTRL |= USBHD_UD_LOW_SPEED;
    }
#endif
    USBHD_UHOST_CTRL |= USBHD_UD_PORT_EN;
    USBHD_UH_SETUP|=USBHD_UH_SOF_EN;
    return( ERR_SUCCESS );
  }

  return( ERR_USB_DISCON );
}

/*******************************************************************************
* Function Name  : WaitUSB_Interrupt
* Description    : Wait USB Interrput.
* Input          : None
* Return         : EEROR
*******************************************************************************/
UINT8 WaitUSB_Interrupt( void )
{
    UINT16  i;

    for ( i = 30000; i != 0 && ((USBOTG_FS->INT_FG) & USBHD_UIF_TRANSFER) == 0; i -- ){;}
    return( ((USBOTG_FS->INT_FG)&USBHD_UIF_TRANSFER)  ? ERR_SUCCESS : ERR_USB_UNKNOWN );
}

/*******************************************************************************
* Function Name  : USBHostTransact
* Description    : USB host transport transaction.
* Input          : endp_pid: endpoint and PID.
*                  tog: Synchronization flag.
*                  timeout: timeout times.
* Return         : EEROR:
*                                    ERR_USB_UNKNOWN
*                  ERR_USB_DISCON
*                  ERR_USB_CONNECT
*                  ERR_SUCCESS
*******************************************************************************/
UINT8 USBHostTransact( UINT8 endp_pid, UINT8 tog, UINT32 timeout )
{
    UINT8   TransRetry;
    UINT8   s, r;
    UINT32  i;

    USBHD_UH_TX_CTRL = USBHD_UH_RX_CTRL = tog;
    TransRetry = 0;

    do {
        USBHD_UH_EP_PID = endp_pid;
        USBOTG_FS->INT_FG = USBHD_UIF_TRANSFER;
        for ( i = WAIT_USB_TOUT_200US; i != 0 && ((USBOTG_FS->INT_FG) & USBHD_UIF_TRANSFER) == 0 ; i -- )//
        {
            Delay_Us( 1 );
        }

        USBHD_UH_EP_PID = 0x00;
        if ( (USBOTG_FS->INT_FG & USBHD_UIF_TRANSFER) == 0 )
        {
            return( ERR_USB_UNKNOWN );
        }

        if (  USBOTG_FS->INT_FG & USBHD_UIF_DETECT ) {
            USBOTG_FS->INT_FG = USBHD_UIF_DETECT;
            s = AnalyzeRootHub( );

            if ( s == ERR_USB_CONNECT )             FoundNewDev = 1;

#ifdef DISK_BASE_BUF_LEN

            if ( CH103DiskStatus == DISK_DISCONNECT ) return( ERR_USB_DISCON );
            if ( CH103DiskStatus == DISK_CONNECT ) return( ERR_USB_CONNECT );

#else
            if ( pOTGHost->DeviceStatus == ROOT_DEV_DISCONNECT ) return( ERR_USB_DISCON );
            if ( pOTGHost->DeviceStatus == ROOT_DEV_CONNECTED ) return( ERR_USB_CONNECT );

#endif
            Delay_Us( 200 );
        }

        if ( USBOTG_FS->INT_FG & USBHD_UIF_TRANSFER )
        {
            if ( USBOTG_FS->INT_ST & USBHD_UIS_TOG_OK ) return( ERR_SUCCESS );
            r = USBOTG_FS->INT_ST & MASK_UIS_H_RES;
            if ( r == USB_PID_STALL ) return( r | ERR_USB_TRANSFER );    //2e

            if ( r == USB_PID_NAK )
            {
                if ( timeout == 0 ) return( r | ERR_USB_TRANSFER );     //2a
                if ( timeout < 0xFFFF ) timeout --;
                -- TransRetry;
            }
            else switch ( endp_pid >> 4 ) {
                case USB_PID_SETUP:

                case USB_PID_OUT:
                    if ( r ) return( r | ERR_USB_TRANSFER );          //21
                    break;

                case USB_PID_IN:                           //2b
                    if ( (r == USB_PID_DATA0) && (r == USB_PID_DATA1) ) {
                    }
                    else if ( r ) return( r | ERR_USB_TRANSFER );
                    break;
                default:
                    return( ERR_USB_UNKNOWN );
            }
        }
        else {
            USBOTG_FS->INT_FG = 0xFF;
        }
        Delay_Us( 15 );
    } while ( ++ TransRetry < 3 );

    return( ERR_USB_TRANSFER );
}

/*******************************************************************************
* Function Name  : HostCtrlTransfer
* Description    : Host control transport.
* Input          : DataBuf : Receive or send data buffer.
*                  RetLen  : Data length.
* Return         : ERR_USB_BUF_OVER IN
*                  ERR_SUCCESS
*******************************************************************************/
UINT8 HostCtrlTransfer( PUINT8 DataBuf, PUINT8 RetLen )
{
    UINT16  RemLen  = 0;
    UINT8   s, RxLen;
    PUINT8  pBuf;
    PUINT8   pLen;

    pBuf = DataBuf;
    pLen = RetLen;
    Delay_Us( 200 );
    if ( pLen ) *pLen = 0;

    USBHD_UH_TX_LEN = sizeof( USB_SETUP_REQ );
    s = USBHostTransact( USB_PID_SETUP << 4 | 0x00, 0x00, 200000 );
    if ( s != ERR_SUCCESS )
    {
         return( s );
    }
    USBHD_UH_RX_CTRL = USBHD_UH_TX_CTRL = USBHD_UH_R_TOG | USBHD_UH_R_AUTO_TOG | USBHD_UH_T_TOG | USBHD_UH_T_AUTO_TOG;
    USBHD_UH_TX_LEN = 0x01;
    RemLen = pSetupReq -> wLength;

    if ( RemLen && pBuf )
    {
        if ( pSetupReq -> bRequestType & USB_REQ_TYP_IN )
        {
            while ( RemLen )
            {
                Delay_Us( 200 );
                USBHD_UH_RX_DMA=(UINT32)pBuf+*pLen;
                s = USBHostTransact( USB_PID_IN << 4 | 0x00, USBHD_UH_RX_CTRL, 200000/20 );
                if ( s != ERR_SUCCESS )         return( s );
                RxLen = USBOTG_FS->RX_LEN < RemLen ? USBOTG_FS->RX_LEN : RemLen;
                RemLen -= RxLen;
                if ( pLen ) *pLen += RxLen;

//                for ( RxCnt = 0; RxCnt != RxLen; RxCnt ++ )
//                {
//                    *pBuf = pHOST_RX_RAM_Addr[ RxCnt ];
//                    pBuf ++;
//                }

                if ( USBOTG_FS->RX_LEN == 0 || ( USBOTG_FS->RX_LEN & ( UsbDevEndp0Size - 1 ) ) ) break;
            }
            USBHD_UH_TX_LEN = 0x00;
        }
        else
        {
            while ( RemLen )
            {
                Delay_Us( 200 );
                USBHD_UH_TX_DMA = (UINT32)pBuf + *pLen;
                R8_UH_TX_LEN = RemLen >= UsbDevEndp0Size ? UsbDevEndp0Size : RemLen;
//                for ( TxCnt = 0; TxCnt != R8_UH_TX_LEN; TxCnt ++ )
//                {
//                    pHOST_TX_RAM_Addr[ TxCnt ] = *pBuf;
//                    pBuf ++;
//                }

                s = USBHostTransact( USB_PID_OUT << 4 | 0x00, R8_UH_TX_CTRL, 200000/20 );
                if ( s != ERR_SUCCESS ) return( s );
                RemLen -= USBHD_UH_TX_LEN;
                if ( pLen ) *pLen += USBHD_UH_TX_LEN;
            }
        }
    }

    Delay_Us( 200 );
    s = USBHostTransact( ( USBHD_UH_TX_LEN ? USB_PID_IN << 4 | 0x00: USB_PID_OUT << 4 | 0x00 ), USBHD_UH_T_TOG | USBHD_UH_R_TOG, 200000/20 );
    if ( s != ERR_SUCCESS )         return( s );
    if ( USBHD_UH_TX_LEN == 0 )        return( ERR_SUCCESS );
    if ( USBOTG_FS->RX_LEN == 0 )       return( ERR_SUCCESS );

    return( ERR_USB_BUF_OVER );
}

/*******************************************************************************
* Function Name  : CopySetupReqPkg
* Description    : Copy setup request package.
* Input          : pReqPkt: setup request package address.
* Return         : None
*******************************************************************************/
void CopySetupReqPkg( const UINT8 *pReqPkt )
{
    UINT8 i;
    printf("setup:");
    for ( i = 0; i != sizeof( USB_SETUP_REQ ); i ++ )
    {
        ((PUINT8)pSetupReq)[ i ] = *pReqPkt;
        printf("%02x ",((PUINT8)pSetupReq)[ i ]);
        pReqPkt++;
    }
    printf("\n");
}

/*******************************************************************************
* Function Name  : CtrlGetDeviceDescr
* Description    : Get device descrptor
* Input          : DataBuf: Data buffer.
* Return         : ERR_USB_BUF_OVER
*                  ERR_SUCCESS
*******************************************************************************/
UINT8 CtrlGetDeviceDescr( PUINT8 DataBuf )
{
    UINT8   s;
    UINT8  len;

    UsbDevEndp0Size = DEFAULT_ENDP0_SIZE;
    CopySetupReqPkg( SetupGetDevDescr );
    s = HostCtrlTransfer( DataBuf, &len );

    if ( s != ERR_SUCCESS )
    {
        printf("error:%02x\n",s);
        return( s );
    }
    UsbDevEndp0Size = ( (PUSB_DEV_DESCR)DataBuf ) -> bMaxPacketSize0;
  if ( len < ((PUSB_SETUP_REQ)SetupGetDevDescr)->wLength ) return( ERR_USB_BUF_OVER );
    return( ERR_SUCCESS );
}

/*******************************************************************************
* Function Name  : CtrlGetConfigDescr
* Description    : Get configration descrptor.
* Input          : DataBuf: Data buffer.
* Return         : ERR_USB_BUF_OVER
*                  ERR_SUCCESS
*******************************************************************************/
UINT8 CtrlGetConfigDescr( PUINT8 DataBuf )
{
    UINT8   s;
    UINT8  len;

    CopySetupReqPkg( SetupGetCfgDescr );
    s = HostCtrlTransfer( DataBuf, &len );
    if ( s != ERR_SUCCESS ) return( s );
    if ( len < ( (PUSB_SETUP_REQ)SetupGetCfgDescr ) -> wLength ) return( ERR_USB_BUF_OVER );

    len = ( (PUSB_CFG_DESCR)DataBuf ) -> wTotalLength;
    CopySetupReqPkg( SetupGetCfgDescr );
    pSetupReq ->wLength = len;
    s = HostCtrlTransfer( DataBuf, &len );
    if ( s != ERR_SUCCESS ) return( s );

    return( ERR_SUCCESS );
}

/*******************************************************************************
* Function Name  : CtrlSetUsbAddress
* Description    : Set USB device address.
* Input          : addr: Device address.
* Return         : ERR_SUCCESS
*******************************************************************************/
UINT8 CtrlSetUsbAddress( UINT8 addr )
{
    UINT8   s;

    CopySetupReqPkg( SetupSetUsbAddr );
    pSetupReq -> wValue = addr;
    s = HostCtrlTransfer( NULL, NULL );
    if ( s != ERR_SUCCESS ) return( s );
    SetHostUsbAddr( addr );
    Delay_Ms( 10 );

    return( ERR_SUCCESS );
}

/*******************************************************************************
* Function Name  : CtrlSetUsbConfig
* Description    : Set usb configration.
* Input          : cfg: Configration Value.
* Return         : ERR_SUCCESS
*******************************************************************************/
UINT8 CtrlSetUsbConfig( UINT8 cfg )
{
    CopySetupReqPkg( SetupSetUsbConfig );
    pSetupReq -> wValue = cfg;
    return( HostCtrlTransfer( NULL, NULL ) );
}

/*******************************************************************************
* Function Name  : CtrlClearEndpStall
* Description    : Clear endpoint STALL.
* Input          : endp: Endpoint address.
* Return         : ERR_SUCCESS
*******************************************************************************/
UINT8 CtrlClearEndpStall( UINT8 endp )
{
    CopySetupReqPkg( SetupClrEndpStall );
    pSetupReq -> wIndex = endp;
    return( HostCtrlTransfer( NULL, NULL ) );
}

/*******************************************************************************
* Function Name  : CtrlSetUsbIntercace
* Description    : Set USB Interface configration.
* Input          : cfg: Configration value.
* Return         : ERR_SUCCESS
*******************************************************************************/
UINT8 CtrlSetUsbIntercace( UINT8 cfg )
{
    CopySetupReqPkg( SetupSetUsbInterface );
    pSetupReq -> wValue = cfg;
    return( HostCtrlTransfer( NULL, NULL ) );
}



/*****************************************************************************
* Function Name  : USBOTG_HostEnum
* Description    : host enumerated device.
* Input          : None
* Return         : ERROR
*******************************************************************************/
UINT8 USBOTG_HostEnum(void)
{
    UINT8  i, s;

    ResetRootHubPort( );

    for ( i = 0, s = 0; i < 100; i ++ )
    {
        Delay_Ms( 1 );
        if ( EnableRootHubPort( ) == ERR_SUCCESS ) {
                i = 0;
                s ++;
                if ( s > 100 ) break;
        }
    }

    if ( i )
    {
        DisableRootHubPort( );
        return( ERR_USB_DISCON );
    }
    SetUsbSpeed( pOTGHost->DeviceSpeed );

    s = CtrlGetDeviceDescr( endpRXbuff );
    if(s !=ERR_SUCCESS)
    {
       printf("get error\n");
    }
    s = CtrlSetUsbAddress( ((PUSB_SETUP_REQ)SetupSetUsbAddr)->wValue );
    if(s !=ERR_SUCCESS)
    {
       printf("set error\n");
    }


    return( s );
}


/*******************************************************************************
* Function Name  : HubSetPortFeature
* Description    : 设置HUB端口特性
* Input          : UINT8 HubPortIndex    //HUB端口
                   UINT8 FeatureSelt     //HUB端口特性
* Output         : None
* Return         : ERR_SUCCESS 成功
*                  其他        错误
*******************************************************************************/
UINT8   HubSetPortFeature( UINT8 HubPortIndex, UINT8 FeatureSelt )
{
    pSetupReq -> bRequestType = HUB_SET_PORT_FEATURE;
    pSetupReq -> bRequest = HUB_SET_FEATURE;
    pSetupReq -> wValue = 0x0000|FeatureSelt;
    pSetupReq -> wIndex = 0x0000|HubPortIndex;
    pSetupReq -> wLength = 0x0000;
    return( HostCtrlTransfer( NULL, NULL ) );     // 执行控制传输
}

/*******************************************************************************
* Function Name  : HubClearPortFeature
* Description    : 清除HUB端口特性
* Input          : UINT8 HubPortIndex
                   UINT8 FeatureSelt
* Output         : None
* Return         : ERR_SUCCESS 成功
*                  其他        错误
*******************************************************************************/
UINT8   HubClearPortFeature( UINT8 HubPortIndex, UINT8 FeatureSelt )
{
    pSetupReq -> bRequestType = HUB_CLEAR_PORT_FEATURE;
    pSetupReq -> bRequest = HUB_CLEAR_FEATURE;
    pSetupReq -> wValue = 0x0000|FeatureSelt;
    pSetupReq -> wIndex = 0x0000|HubPortIndex;
    pSetupReq -> wLength = 0x0000;
    return( HostCtrlTransfer( NULL, NULL ) );     // 执行控制传输
}
