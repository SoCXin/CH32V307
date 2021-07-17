/********************************** (C) COPYRIGHT *******************************
* File Name          : ch32v30x_usbhs_host.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : This file provides all the USB firmware functions.
*******************************************************************************/ 
#include "ch32v30x_usbhs_host.h"


/******************************** HOST DEVICE **********************************/
UINT8  FoundNewDev=0;;
UINT8  DevEnum=0;
BOOL   Ttog=1;
UINT8  Endp0MaxSize=64;
PUSBHS_HOST Host_Status;

__attribute__ ((aligned(4))) const UINT8  GetDevDescrptor[]={USB_REQ_TYP_IN, USB_GET_DESCRIPTOR, 0x00, USB_DESCR_TYP_DEVICE, 0x00, 0x00, sizeof( USB_DEV_DESCR ), 0x00};
__attribute__ ((aligned(4))) const UINT8  GetConfigDescrptor[]= {USB_REQ_TYP_IN, USB_GET_DESCRIPTOR, 0x00, USB_DESCR_TYP_CONFIG, 0x00, 0x00, 0x04, 0x00};
__attribute__ ((aligned(4))) const UINT8  SetAddress[]={USB_REQ_TYP_OUT, USB_SET_ADDRESS, USB_DEVICE_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00};
__attribute__ ((aligned(4))) const UINT8  SetConfig[]={USB_REQ_TYP_OUT, USB_SET_CONFIGURATION, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
__attribute__ ((aligned(4))) const UINT8  Clear_EndpStall[]={USB_REQ_RECIP_INTERF, USB_SET_INTERFACE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

__attribute__ ((aligned(16))) UINT8 endpRXbuff[USBHS_MAX_PACK_SIZE]; //端点1数据收发缓冲区
__attribute__ ((aligned(16))) UINT8 endpTXbuff[USBHS_MAX_PACK_SIZE]; //端点3数据收发缓冲区

#define pSetupReq         ((PUSB_SETUP_REQ)endpTXbuff)
#define EndpnMaxSize      512

/*******************************************************************************
* Function Name  : user2mem_copy( )
* Description    : copy the contents of the buffer to another address.
* Input          : usrbuf:buffer address
*                  addr: target address
*                  bytes: length
* Return         : None
*******************************************************************************/
void user2mem_copy( UINT8 *usrbuf, UINT32 addr, UINT16 bytes )
{
    UINT16  i;
    UINT8 *p8 = usrbuf;
    UINT8 *p8_ = (UINT8 *)addr;
    for( i=0;i<bytes;i++)
    {
        *p8_++ = *p8++;
    }
}

/*******************************************************************************
* Function Name  : USB20_RCC_Init
* Description    : USB RCC initialized
* Input          : None
* Return         : None
*******************************************************************************/
void USB20_RCC_Init( void )
{
    RCC->CFGR2 = USBHS_PLL_SRC_HSE | USBHS_PLL_SRC_PRE_DIV2 | USBHS_PLL_CKREF_4M;//PLL REF = HSE/2 = 4MHz
    RCC->CFGR2 |= USBHS_CLK_SRC_PHY|USBHS_PLLALIVE;
    RCC->AHBPCENR |= RCC_AHBPeriph_USBHS;                  //USB时钟使能
    Delay_Us(200);
    USBHS->CONTROL |= SUSPENDM;
    Delay_Us(5);
}

/*******************************************************************************
* Function Name  : USBHS_HostInit
* Description    : USB host mode initialized.
* Input          : enable or disable
* Return         : None
*******************************************************************************/
void USBHS_HostInit (FunctionalState sta)  // USBHS host initial
{
    if(sta==ENABLE)
    {
        USB20_RCC_Init();
        USBHS->CONTROL = SUSPENDM | INT_BUSY_EN | DMA_EN | HIGH_SPEED | HOST_MODE|SEND_SOF_EN;
        USBHS->ENDP_CONFIG = HOST_TX_EN | HOST_RX_EN ;
        USBHS->UEP2_MAX_LEN = EndpnMaxSize;                                        // MAX_LEN
        USBHS->UEP3_TX_DMA = (UINT32)&endpTXbuff[0];                     // set host TX DMA address
        USBHS->UEP2_RX_DMA = (UINT32)&endpRXbuff[0];                     // set host RX DMA address

        while( !(USBHS->STATUS & USBHS_DETECT_FLAG ));                //wait device attached
        FoundNewDev=1;
        USBHS->CONTROL |= SEND_BUS_RESET;                              //bus reset
        Delay_Ms(15);
        USBHS->CONTROL &= ~SEND_BUS_RESET;
        while( !(USBHS->CONTROL & LINK_READY) );                      // wait link ready;;
        Host_Status->DeviceSpeed=(UINT8)((USBHS->STATUS) & USBSPEED_MASK);
        if(Host_Status->DeviceSpeed==0x02)
        {
            Endp0MaxSize=8;
        }
    }
    else
    {
        USBHS->CONTROL=USB_FORCE_RST|USB_ALL_CLR;
    }
}

/*******************************************************************************
* Function Name  : CtrlGetDevDescr( )
* Description    : get the device descriptor.
* Input          : pdev: host status variable
*                  buf:request command buffer
* Return         : None
*******************************************************************************/
UINT8 CtrlGetDevDescr(PUSBHS_HOST pdev,UINT8 *buf)
{
    UINT32 rxlen;
    UINT16 len;
    user2mem_copy(buf,(UINT32)&endpTXbuff[0],8);
    rxlen=USBHS_HostCtrlTransfer(endpRXbuff,&len);
    if(rxlen!=TRANS_SUCCESS)
    {
      return  (rxlen=TRANS_ERR);
    }
    if(len < pSetupReq->wLength )  return ERR_USB_BUF_OVER;
    pdev->DeviceEndp0Size=((PUSB_DEV_DESCR)endpRXbuff)->bMaxPacketSize0;
    return  rxlen;
}

/*******************************************************************************
* Function Name  : CtrlGetConfigDescr( )
* Description    : Enable root hub.
* Input          : pdev: host status variable
*                  buf:request command buffer
* Return         : TRANS_SUCCESS
*                  ERR_USB_BUF_OVER
*******************************************************************************/
UINT8 CtrlGetConfigDescr(PUSBHS_HOST pdev,UINT8 *buf)
{
    UINT32 rxlen;
    UINT16 reallen,len;
    user2mem_copy(buf,(UINT32)&endpTXbuff[0],8);
    rxlen=USBHS_HostCtrlTransfer((UINT8 *)endpRXbuff,&len);
    if(rxlen!=TRANS_SUCCESS)
    {
      return  (rxlen=TRANS_ERR);
    }

    if(len <pSetupReq->wLength )  return ERR_USB_BUF_OVER;

    reallen=((PUSB_CFG_DESCR)endpRXbuff)->wTotalLength;
    pdev->DeviceCongValue=((PUSB_CFG_DESCR)endpRXbuff)->bConfigurationValue;

    user2mem_copy(buf,(UINT32)&endpTXbuff[0],8);
    pSetupReq->wLength=reallen;
    rxlen=USBHS_HostCtrlTransfer((UINT8 *)endpRXbuff,&len);
    if(rxlen!=TRANS_SUCCESS)
    {
      return  (rxlen=TRANS_ERR);
    }
    if(len < pSetupReq->wLength)  return ERR_USB_BUF_OVER;

    Anaylysis_Descr(pdev,(UINT8 *)endpRXbuff,pSetupReq->wLength);
    return  rxlen;
}

/*******************************************************************************
* Function Name  : CtrlSetAddress
* Description    : set device address.
* Input          : pdev:host status variable
*                  buf:request command buffer
*                  addr:the address of device
* Return         : TRANS_ERR
*                  TRANS_SUCCESS
*******************************************************************************/
UINT8 CtrlSetAddress(PUSBHS_HOST pdev,UINT8 *buf,UINT8 addr)
{
    UINT32 rxlen;
    user2mem_copy(buf,(UINT32)&endpTXbuff[0],8);
    pSetupReq->wValue=addr;
    rxlen=USBHS_HostCtrlTransfer(NULL,NULL);
    if(rxlen!=TRANS_SUCCESS)
    {
      return  (rxlen=TRANS_ERR);
    }
    USBHS_CurrentAddr(addr);
    return  TRANS_SUCCESS;
}

/*******************************************************************************
* Function Name  : CtrlSetConfig
* Description    : set device configuration.
* Input          : pdev: host status variable.
*                  buf: request command buffer.
* Return         : TRANS_ERR
*				   TRANS_SUCCESS
*******************************************************************************/
UINT8 CtrlSetConfig(PUSBHS_HOST pdev,UINT8 *buf )
{
    UINT32 rxlen;
    user2mem_copy(buf,(UINT32)&endpTXbuff[0],8);
    pSetupReq->wIndex=(pdev->DeviceCongValue)<<8;
    rxlen=USBHS_HostCtrlTransfer(NULL,NULL);
    if(rxlen!=TRANS_SUCCESS)
    {
     return  (rxlen=TRANS_ERR);
    }
    return  TRANS_SUCCESS;
}

/*******************************************************************************
* Function Name  : USBHS_HostCtrlTransfer
* Description    : Host control transport.
* Input          : DataBuf : Receive or send data buffer.
*                  RetLen  : Data length.
* Return         : ERR_USB_BUF_OVER
*                  TRANS_SUCCESS
*                  USBHS_TIMEOUT
*                  DEV_RES_STALL
*******************************************************************************/
UINT32 USBHS_HostCtrlTransfer(UINT8 *databuf,PUINT16 len)
{
    UINT32 rxlen;
    UINT16 txlen;
    UINT16 ReLen;
    PUINT8 pBuf;
    PUINT16 pLen;
    UINT8 StageFlg=0;
    pBuf=databuf;
    pLen=len;
    if(pLen)
    {
       *pLen=0;
    }
    rxlen=USBHS_SetupTransact();
    if(rxlen!=TRANS_SUCCESS)              //error
    {
       return rxlen;
    }
    ReLen=pSetupReq->wLength;
    if(pLen && pBuf)                     //有数据阶段
    {
       if(pSetupReq->bRequestType==USB_REQ_TYP_IN)
       {
           while(ReLen)
           {
               Delay_Us( 100 );
               USBHS->UEP2_RX_DMA = (UINT32)pBuf + *pLen;
               rxlen= USBHS_INTransact(DEF_ENDP_0,Ttog);
               if((rxlen ==USBHS_TOGGLE_ERR)||(rxlen==USBHS_HARDTIMEOUT))               //hardware timeout
               {
                  return rxlen;
               }
               rxlen=(rxlen<ReLen)?rxlen:ReLen;
               ReLen-=rxlen;
               if(pLen)
               {
                 *pLen +=rxlen;
               }
               if((USBHS->RX_LEN==0)||(USBHS->RX_LEN==(Endp0MaxSize-1)))
               {
                  StageFlg=1;
                  len=TRANS_SUCCESS;                //success
                  break;
               }
            }
         }
       else
       {
          while(ReLen)
          {
               Delay_Us( 100 );
               USBHS->UEP3_TX_DMA=(UINT32)pBuf + *pLen;
               txlen=(ReLen>Endp0MaxSize)?Endp0MaxSize:ReLen;
               rxlen=USBHS_OUTTransact(DEF_ENDP_0,txlen,Ttog);
               if(rxlen !=TRANS_SUCCESS)
               {
                  return  rxlen;
               }
               ReLen-=txlen;
               if(pLen)
               {
                   *pLen +=txlen;
               }
          }
        }
    }
    Delay_Us( 100 );
    if(StageFlg)
    {
        rxlen=USBHS_OUTTransact(DEF_ENDP_0,0,Ttog);
        if(rxlen !=TRANS_SUCCESS) return rxlen;
    }
    else
    {
        rxlen=USBHS_INTransact(DEF_ENDP_0,Ttog);
        if(rxlen!=TRANS_SUCCESS)              //error
        {
           return rxlen;
        }
    }
    if ( USBHS->UEP3_CTRL == 0 )        return( TRANS_SUCCESS );
    if (USBHS->RX_LEN == 0 )       return( TRANS_SUCCESS );
    return ERR_USB_BUF_OVER;
}

/*******************************************************************************
* Function Name  : USBHS_HostEnum
* Description    : Host enumerate device.
* Input          : None
* Return         : None
*******************************************************************************/
void USBHS_HostEnum( void )
{
  UINT8 ss;
  ss=CtrlGetDevDescr(Host_Status,(UINT8 *)GetDevDescrptor);
  if(ss !=TRANS_SUCCESS)    printf("error1\n");
  ss=CtrlSetAddress(Host_Status,(UINT8 *)SetAddress,8);
  if(ss !=TRANS_SUCCESS)    printf("error2\n");

  ss=CtrlGetConfigDescr(Host_Status,(UINT8 *)GetConfigDescrptor);
  if(ss !=TRANS_SUCCESS)    printf("error3\n");

  ss=CtrlSetConfig(Host_Status,(UINT8 *)SetConfig);
  if(ss !=TRANS_SUCCESS)    printf("error4\n");

  DevEnum=1;
}

/*******************************************************************************
* Function Name  : USBHS_SetupTransact
* Description    : host Setup transaction.
* Input          : None.
* Return         : None
*******************************************************************************/
UINT32 USBHS_SetupTransact(void)
{
    UINT32 len;
    UINT32 res;
    UINT32 timecnt=0;
    USBHS->UEP3_CTRL = 8;
    USBHS->UEP2_CTRL = (USB_PID_SETUP<<4); // setup
    while( 1 )
    {
        if( USBHS->STATUS & USBHS_ACT_FLAG )
        {
            res = (USBHS->STATUS >>24) & 0xf;
            if( res == USB_PID_ACK )
            {
                len=TRANS_SUCCESS;
                USBHS->UEP2_CTRL = 0;
                USBHS->STATUS = USBHS_ACT_FLAG;
                break;
            }
            else if(res==USB_PID_STALL)
            {
                len=DEV_RES_STALL;
                USBHS->UEP2_CTRL = 0;
                printf("stall\n");
                USBHS->STATUS = USBHS_ACT_FLAG;
                break;
            }
            USBHS->STATUS = USBHS_ACT_FLAG;
        }
        timecnt++;
        if(timecnt>TIMEOUT_COUNT)
        {
            len=USBHS_TIMEOUT;
        }
    }
    return len;
}

/*******************************************************************************
* Function Name  : USBHS_INTransact
* Description    : Host IN transaction
* Input          : endp_num: the number of endpoint.
*                  toggle:the PID of the IN transaction
* Return         : USBHS_TOGGLE_ERR
*                  USBHS_HARDTIMEOUT
*                  ERR_SUCCESS
*                  packet length  of IN transaction
*******************************************************************************/
UINT32  USBHS_INTransact( UINT32 endp_num ,UINT32 toggle)
{
    UINT32 len=0;
    UINT32 res;
    UINT8 tx_success=0;

    USBHS->UEP2_CTRL = (toggle<<27)|(USB_PID_IN<<4) | endp_num; // in
    while( !tx_success )
    {
        if( USBHS->STATUS & USBHS_ACT_FLAG )
        {
            res= (USBHS->STATUS >>24) & 0xf;
            switch(res)
            {
                case USB_NORES_TOUT:    //hard timeout or no response
                    len=USBHS_HARDTIMEOUT;
                    USBHS->UEP2_CTRL = 0;
                    tx_success=1;
                    break;
                case USB_PID_NAK:
                    break ;
                default:                                               //DATA1 or DATA0
                    if(USBHS->STATUS & USBHS_TOGGLE_OK)
                    {
                        len=(USBHS->RX_LEN);
                    }
                    else
                    {
                        len=USBHS_TOGGLE_ERR;
                    }
                    tx_success=1;
                    USBHS->UEP2_CTRL = 0;
                    break;
            }
            USBHS->STATUS = USBHS_ACT_FLAG;
        }
    }
    return len;
}

/*******************************************************************************
* Function Name  : USBHS_OUTTransact
* Description    : Host OUT transaction.
* Input          : endp_num: the number of endpoint.
*                  toggle:the PID of the OUT transaction
*                  tx_len:the packet length of OUT transaction
* Return         : TRANS_SUCCESS
*                  USBHS_TIMEOUT
*******************************************************************************/
UINT32 USBHS_OUTTransact( UINT32 endp_num, UINT32 tx_len,UINT32 toggle )
{
    UINT32  len;
    UINT32 res;
    UINT32 timeout=0;
    USBHS->UEP3_CTRL = (toggle<<19)|tx_len;
    USBHS->UEP2_CTRL = (USB_PID_OUT<<4) | endp_num;
    while( 1 )
    {
        if( USBHS->STATUS & USBHS_ACT_FLAG )
        {
            res=(USBHS->STATUS >>24) & 0xf;
            if(res == USB_PID_ACK )                 //
            {
                USBHS->UEP2_CTRL = 0;
                len=TRANS_SUCCESS;
                USBHS->STATUS = USBHS_ACT_FLAG;
                break;
            }
            else {
                USBHS->STATUS = USBHS_ACT_FLAG;
            }
        }
        timeout++;
        if(timeout>TIMEOUT_COUNT)
        {
            USBHS->UEP2_CTRL = 0;
            len=USBHS_TIMEOUT;
            break;
        }
    }
    return len;
}

/*******************************************************************************
* Function Name  : USBHS_INTransact_ISO
* Description    : IN transaction of synchronous transmission.
* Input          : endp_num: the number of endpoint.
*                  toggle:the PID of the IN transaction
* Return         : USBHS_TIMEOUT
*                  packet length  of IN transaction and return PID of device
*******************************************************************************/
UINT32 USBHS_INTransact_ISO( UINT32 endp_num , UINT8 toggle )
{
    UINT32 timeout=0;
    UINT32 len=0;
    UINT8 res;
    USBHS->ENDP_TYPE = USBHS_ENDP2_RX_ISO;
    USBHS->UEP2_CTRL = (USB_PID_IN<<4) | endp_num | EP_R_RES_NO | (toggle<<27); // in(0x69)
    while( 1 )
    {
        if( USBHS->STATUS &  USBHS_ACT_FLAG )
        {
            res=(UINT8)((USBHS->STATUS >>24) & 0xf);
            if(res==USB_PID_ACK)
            {
                len=USBHS->RX_LEN;
                USBHS->UEP2_CTRL = 0;
                len=len|(res<<24);                        //高8位为当前设备回复TOGGLE
                USBHS->STATUS =  USBHS_ACT_FLAG;
                break;
            }
            else
            {
                USBHS->STATUS =  USBHS_ACT_FLAG;
            }
        }
        timeout++;
        if(timeout>TIMEOUT_COUNT)
        {
            USBHS->UEP2_CTRL = 0;
            len=USBHS_TIMEOUT;
            break;
        }
    }
    USBHS->ENDP_TYPE = 0;
    return len;
}

/*******************************************************************************
* Function Name  : USBHS_OUTTransact_ISO
* Description    : OUT transaction of synchronous transmission.
* Input          : endp_num: Configration Value.
*                  tx_len:the packet length of OUT transaction
*                  toggle:the PID of the OUT transaction
* Return         : ERR_SUCCESS
*                  USBHS_TIMEOUT
*******************************************************************************/
UINT32 USBHS_OUTTransact_ISO( UINT32 endp_num, UINT32 tx_len, UINT8 toggle)
{
    UINT32 timeout=0;
    UINT32 len=0;

    USBHS->ENDP_TYPE =  USBHS_ENDP3_TX_ISO;
    USBHS->UEP3_CTRL =(toggle<<19)|EP_T_RES_NO | tx_len;
    USBHS->UEP2_CTRL = (0xE1<<4) | endp_num; // out(0xE1)
    while( 1 )
    {
        if( USBHS->STATUS &  USBHS_ACT_FLAG )
        {
            if( ((USBHS->STATUS >>24) & 0xf) != 0xa )// !NAK(0x5A)
            {
                USBHS->UEP2_CTRL = 0;
                len=TRANS_SUCCESS;
                USBHS->STATUS =  USBHS_ACT_FLAG;
                break;
            }
            USBHS->STATUS =  USBHS_ACT_FLAG;
        }
        timeout++;
        if(timeout>TIMEOUT_COUNT)
        {
            USBHS->UEP2_CTRL = 0;
            len=USBHS_TIMEOUT;
            break;
        }
    }
    USBHS->ENDP_TYPE = 0;
    return len;
}

/*******************************************************************************
* Function Name  : USBHS_CurrentAddr
* Description    : Current device address.
* Input          : address: Endpoint address.
* Return         : None
*******************************************************************************/
void USBHS_CurrentAddr( UINT32 address )
{
    USBHS->CONTROL &= 0x00ffffff;
    USBHS->CONTROL |= address<<24; // SET ADDRESS
}

/*******************************************************************************
* Function Name  : Anaylisys_Descr
* Description    : descriptor analysis.
* Input          : pusbdev: host status variable.
*                  pdesc:descriptor buffer to analyze
*                  l:length
* Return         : None
*******************************************************************************/
void Anaylysis_Descr(PUSBHS_HOST pusbdev,PUINT8 pdesc, UINT16 l)
{
    UINT16 i,EndPMaxSize;                                         //这里的形参传的是receivebuff里的数据
    for(i=0;i<l;i++)                                               //分析描述符
    {
     if((pdesc[i]==0x09)&&(pdesc[i+1]==0x02))
        {
            printf("bNumInterfaces:%02x \n",pdesc[i+4]);          //配置描述符里的接口数-第5个字节
        }

     if((pdesc[i]==0x07)&&(pdesc[i+1]==0x05))
        {
            if((pdesc[i+2])&0x80) //&&一个一个的比较 到了false就停止比较，&是所有的对象比较完了再停止
            {
                 printf("endpIN:%02x \n",pdesc[i+2]&0x0f);              //取端点号
                 pusbdev->DEVENDP.InEndpNum=pdesc[i+2]&0x0f; //将IN端点号保存到结构体中
                 pusbdev->DEVENDP.InEndpCount++;
                 EndPMaxSize=((UINT16)pdesc[i+5]<<8)|pdesc[i+4];        //描述符里是低字节在前
                 pusbdev->DEVENDP.InEndpMaxSize=EndPMaxSize;
                 printf("In_endpmaxsize:%02x \n",EndPMaxSize);
            }
            else
            {
                printf("endpOUT:%02x \n",pdesc[i+2]&0x0f);              //取端点号
                pusbdev->DEVENDP.OutEndpNum=pdesc[i+2]&0x0f;
                pusbdev->DEVENDP.OutEndpCount++;
                EndPMaxSize=((UINT16)pdesc[i+5]<<8)|pdesc[i+4];
                pusbdev->DEVENDP.OutEndpMaxSize=EndPMaxSize;
                printf("Out_endpmaxsize:%02x \n",EndPMaxSize);
            }
        }
    }
}


