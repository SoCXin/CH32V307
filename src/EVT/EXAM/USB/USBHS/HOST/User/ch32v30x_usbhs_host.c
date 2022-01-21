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
UINT8  Endp0MaxSize = 0;
UINT16V EndpnMaxSize = 512;
PUSBHS_HOST Host_Status;

__attribute__ ((aligned(4))) const UINT8  GetDevDescrptor[]={USB_REQ_TYP_IN, USB_GET_DESCRIPTOR, 0x00, USB_DESCR_TYP_DEVICE, 0x00, 0x00, 8, 0x00};
__attribute__ ((aligned(4))) const UINT8  GetConfigDescrptor[]= {USB_REQ_TYP_IN, USB_GET_DESCRIPTOR, 0x00, USB_DESCR_TYP_CONFIG, 0x00, 0x00, 0x04, 0x00};
__attribute__ ((aligned(4))) const UINT8  SetAddress[]={USB_REQ_TYP_OUT, USB_SET_ADDRESS, USB_DEVICE_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00};
__attribute__ ((aligned(4))) const UINT8  SetConfig[]={USB_REQ_TYP_OUT, USB_SET_CONFIGURATION, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
__attribute__ ((aligned(4))) const UINT8  Clear_EndpStall[]={USB_REQ_RECIP_INTERF, USB_SET_INTERFACE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

__attribute__ ((aligned(4))) UINT8 endpRXbuff[USBHS_MAX_PACK_SIZE]; //端点1数据收发缓冲区
__attribute__ ((aligned(4))) UINT8 endpTXbuff[USBHS_MAX_PACK_SIZE]; //端点3数据收发缓冲区

#define pSetupReq         ((PUSB_SETUP_REQ)endpTXbuff)

#define FULL_SPEED_TYPE   0
#define HIGH_SPEED_TYPE   1
#define LOW_SPEED_TYPE    2

/*********************************************************************
 * @fn      user2mem_copy
 *
 * @brief   copy the contents of the buffer to another address.
 *
 * @param   usrbuf - buffer address
 *          addr - target address
 *          bytes - length
 *
 * @return  none
 */
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

/*********************************************************************
 * @fn      USB20_RCC_Init
 *
 * @brief   USB RCC initialized
 *
 * @return  none
 */
void USB20_RCC_Init( void )
{
    RCC->CFGR2 = USBHS_PLL_SRC_HSE | USBHS_PLL_SRC_PRE_DIV2 | USBHS_PLL_CKREF_4M;//PLL REF = HSE/2 = 4MHz
    RCC->CFGR2 |= USBHS_CLK_SRC_PHY|USBHS_PLLALIVE;
    RCC->AHBPCENR |= RCC_AHBPeriph_USBHS;                  //USB clock enable
    Delay_Us(200);
    USBHSH->HOST_CTRL |= PHY_SUSPENDM;
    Delay_Us(5);
}

/*********************************************************************
 * @fn      SetBusReset
 *
 * @brief   Reset USB bus
 *
 * @return  none
 */
void  SetBusReset(void)
{
    USBHSH->HOST_CTRL |= SEND_BUS_RESET;                              //bus reset
    Delay_Ms(15);
    USBHSH->HOST_CTRL &= ~SEND_BUS_RESET;
    while( !(USBHSH->HOST_CTRL & UH_SOFT_FREE) );                     //wait bus idle;
    USBHSH->HOST_CTRL |= SEND_SOF_EN;                                 //sof enable
}

/*********************************************************************
 * @fn      USBHS_HostInit
 *
 * @brief   USB host mode initialized.
 *
 * @param   sta - ENABLE or DISABLE
 *
 * @return  none
 */
void USBHS_HostInit (FunctionalState sta)  // USBHS host initial
{
    if(sta==ENABLE)
    {
        USB20_RCC_Init();

#if   HIGH_SPEED_TYPE

        USBHSH->CONTROL =  INT_BUSY_EN | DMA_EN | HIGH_SPEED | HOST_MODE;

#elif FULL_SPEED_TYPE

        USBHSH->CONTROL =  INT_BUSY_EN | DMA_EN | FULL_SPEED | HOST_MODE;

#else

        USBHSH->CONTROL =  INT_BUSY_EN | DMA_EN | LOW_SPEED | HOST_MODE;

#endif

        USBHSH->HOST_CTRL = PHY_SUSPENDM | SEND_SOF_EN;
        USBHSH->HOST_EP_CONFIG = HOST_TX_EN | HOST_RX_EN ;                // send enable, receive enable
        USBHSH->HOST_RX_MAX_LEN = EndpnMaxSize;                           // receive max length
        USBHSH->HOST_TX_DMA = (UINT32)&endpTXbuff[0];                     // set host TX DMA address
        USBHSH->HOST_RX_DMA = (UINT32)&endpRXbuff[0];                     // set host RX DMA address
    }
    else
    {
        USBHSH->CONTROL = USB_FORCE_RST | USB_ALL_CLR;
    }
}

/*********************************************************************
 * @fn      CtrlGetDevDescr
 *
 * @brief   Get device descrptor
 *
 * @param   dev - host status variable
 *          buf - request command buffer
 *
 * @return  Error state
 */
UINT8 CtrlGetDevDescr(PUSBHS_HOST pdev,UINT8 *buf)
{
    UINT8 rxlen;
    UINT16 len;
    Endp0MaxSize = 8;
    user2mem_copy(buf,(UINT32)&endpTXbuff[0],8);
    rxlen = USBHS_HostCtrlTransfer(endpRXbuff,&len);
    if( rxlen != ERR_SUCCESS )         return  ERR_SUCCESS;
    if( len < Endp0MaxSize )           return  ERR_USB_BUF_OVER;

    pdev->DeviceEndp0Size = ((PUSB_DEV_DESCR)endpRXbuff)->bMaxPacketSize0;
    Endp0MaxSize = ((PUSB_DEV_DESCR)endpRXbuff)->bMaxPacketSize0;
    return  rxlen;
}

/*********************************************************************
 * @fn      CtrlGetConfigDescr
 *
 * @brief   Get configration descrptor.
 *
 * @param   pdev - host status variable
 *          buf - request command buffer
 *
 * @return  Error state
 */
UINT8 CtrlGetConfigDescr(PUSBHS_HOST pdev,UINT8 *buf)
{
    UINT32 rxlen;
    UINT16 reallen,len;

    user2mem_copy(buf,(UINT32)&endpTXbuff[0],8);
    rxlen = USBHS_HostCtrlTransfer((UINT8 *)endpRXbuff,&len);
    if(rxlen != ERR_SUCCESS)      return   ERR_SUCCESS;
    if(len < pSetupReq->wLength )  return  ERR_USB_BUF_OVER;

    reallen = ((PUSB_CFG_DESCR)endpRXbuff)->wTotalLength;             //解析全部配置描述符的长度

    user2mem_copy(buf,(UINT32)&endpTXbuff[0],8);
    pSetupReq->wLength = reallen;
    rxlen = USBHS_HostCtrlTransfer((UINT8 *)endpRXbuff,&len);         //获取全部配置描述符
    if(rxlen != ERR_SUCCESS)      return  ERR_SUCCESS;
    if(len < pSetupReq->wLength)  return ERR_USB_BUF_OVER;

    pdev->DeviceCongValue = ((PUSB_CFG_DESCR)endpRXbuff)->bConfigurationValue;
    Analysis_Descr(pdev,(UINT8 *)endpRXbuff,pSetupReq->wLength);
    return  rxlen;
}

/*********************************************************************
 * @fn      CtrlSetUsbAddress
 *
 * @brief   Set USB device address.
 *
 * @param   pdev - host status variable
 *          buf - request command buffer
 *          addr - Device address.
 *
 * @return  Error state
 */
UINT8 CtrlSetAddress(PUSBHS_HOST pdev,UINT8 *buf,UINT8 addr)
{
    UINT8 rxlen;

    user2mem_copy(buf,(UINT32)&endpTXbuff[0],8);
    pSetupReq->wValue = addr;
    rxlen = USBHS_HostCtrlTransfer(NULL,NULL);
    if(rxlen != ERR_SUCCESS)  return  ERR_SUCCESS;

    USBHS_CurrentAddr(addr);
    pdev->DeviceAddress = addr ;
    return  ERR_SUCCESS;
}

/*********************************************************************
 * @fn      CtrlSetUsbConfig
 *
 * @brief   Set usb configration.
 *
 * @param   pdev - host status variable.
 *          buf - request command buffer.
 *
 * @return  Error state
 */
UINT8 CtrlSetConfig(PUSBHS_HOST pdev,UINT8 *buf )
{
    UINT32 rxlen;

    user2mem_copy(buf,(UINT32)&endpTXbuff[0],8);
    pSetupReq->wValue = pdev->DeviceCongValue;
    rxlen=USBHS_HostCtrlTransfer(NULL,NULL);
    if(rxlen != ERR_SUCCESS)  return  ERR_SUCCESS;

    return  ERR_SUCCESS;
}

/*********************************************************************
 * @fn      USBHS_Transact
 *
 * @brief   Host transaction
 *
 * @param   endp_pid - PID of the transaction  and the number of endpoint
 *          toggle - sync  trigger bit
 *          timeout - value of timeout
 *
 * @return  Error state
 */
UINT8 USBHS_Transact(UINT8 endp_pid,UINT8 toggle,UINT32 timeout)
{
    UINT8   TransRetry,r;
    UINT32  i;

    USBHSH->HOST_TX_CTRL = USBHSH->HOST_RX_CTRL = toggle;
    TransRetry = 0;
    do
    {
        USBHSH->HOST_EP_PID = endp_pid ;                                      //设置主机发送包的令牌
        USBHSH->INT_FG = USBHS_ACT_FLAG;                                      //清发送完成中断
        for ( i = WAIT_USB_TOUT_200US; i != 0 && ((USBHSH->INT_FG) & USBHS_ACT_FLAG) == 0 ; i -- )//
        {
            Delay_Us( 1 );                                                      //等待发送完成
        }
        USBHSH->HOST_EP_PID = 0x00;
        if ( (USBHSH->INT_FG & USBHS_ACT_FLAG) == 0 )  return( ERR_USB_UNKNOWN );

        if( USBHSH->INT_FG & USBHS_DETECT_FLAG )                                //当前设备被拔除
        {
            USBHSH->INT_FG = USBHS_DETECT_FLAG;
            if( USBHSH->MIS_ST & USBHS_ATTCH )
            {
                if(USBHSH->HOST_CTRL & SEND_SOF_EN)    return ( ERR_USB_CONNECT );
            }
            else    return ( ERR_USB_DISCON );
        }

        if ( USBHSH->INT_FG & USBHS_ACT_FLAG )                                  //数据包传输成功
        {
            r = USBHSH->INT_ST & USBHS_HOST_RES;
            if((endp_pid >> 4) == USB_PID_IN )
            {
                if ( USBHSH->INT_ST & USBHS_TOGGLE_OK )  return( ERR_SUCCESS );  //数据包令牌匹配
            }
            else
            {
                if ( r == USB_PID_ACK )                  return( ERR_SUCCESS );  //setup/out包等待设备响应ACK
            }
            if ( r == USB_PID_STALL )                    return( r | ERR_USB_TRANSFER );              //设备响应STALL

            if ( r == USB_PID_NAK )
            {
                if ( timeout == 0 )                      return( r | ERR_USB_TRANSFER );                 //设备响应NAK 超时
                if ( timeout < 0xFFFF ) timeout --;
                -- TransRetry;
            }
            else switch ( endp_pid >> 4  )
            {
                case USB_PID_SETUP:
                    break;
                case USB_PID_OUT:
                    if ( r ) return( r | ERR_USB_TRANSFER );          //
                    break;
                case USB_PID_IN:                           //2b
                    if ( (r == USB_PID_DATA0) || (r == USB_PID_DATA1) )
                    {

                    }
                    else if ( r ) return( r | ERR_USB_TRANSFER );
                    break;
                default:
                    return( ERR_USB_UNKNOWN );
            }
        }
        else {
            USBHSH->INT_FG = 0xFF;
        }
        Delay_Us( 15 );
    } while ( ++ TransRetry < 3 );    //尝试超过三次未成功

    return( ERR_USB_TRANSFER );
}

/*********************************************************************
 * @fn      USBHS_HostCtrlTransfer
 *
 * @brief   Host control transfer.
 *
 * @param   DataBuf - Receive or send data buffer.
 *          RetLen - Data length.
 *
 * @return  Error state
 */
UINT8 USBHS_HostCtrlTransfer(UINT8 *databuf,PUINT16 len)
{
    UINT32  rxlen;
    UINT16  ReLen;
    PUINT16 pLen;
    UINT8   ret;
    PUINT8  pBuf;
    UINT8   Ttog=1;
    pBuf = databuf;
    pLen = len;
    ReLen = pSetupReq->wLength;

    if( pLen )  *pLen = 0;
    USBHSH->HOST_TX_LEN = 8;
    ret = USBHS_Transact((USB_PID_SETUP<<4)|DEF_ENDP_0, 0, 200000);
    if(ret != ERR_SUCCESS)      return ret;                     //error

    if(ReLen && pBuf)                                           //data stage
    {
       if(pSetupReq->bRequestType == USB_REQ_TYP_IN)            //device to host
       {
           while(ReLen)
           {
               Delay_Us( 100 );
               USBHSH->HOST_RX_DMA = (UINT32)pBuf + *pLen;
               ret = USBHS_Transact((USB_PID_IN<<4)| DEF_ENDP_0, Ttog<<3, 20000);
               if(ret != ERR_SUCCESS)                return ret;

               rxlen =(USBHSH->RX_LEN >= ReLen) ? ReLen : USBHSH->RX_LEN;
               ReLen -= rxlen;
               Ttog ^=1;
               if(pLen)  *pLen += rxlen;
               if( ( USBHSH->RX_LEN == 0 ) || (USBHSH->RX_LEN & ( Endp0MaxSize - 1 )))  break;
            }
            USBHSH->HOST_TX_LEN = 0 ;
         }
       else
       {                                             // host to device
          while(ReLen)
          {
               Delay_Us( 100 );
               USBHSH->HOST_TX_DMA = (UINT32)pBuf + *pLen;
               USBHSH->HOST_TX_LEN = (ReLen > Endp0MaxSize)? Endp0MaxSize : ReLen;

               ret = USBHS_Transact((USB_PID_OUT<<4)|DEF_ENDP_0, Ttog<<3,  20000);
               if(ret != ERR_SUCCESS)               return  ret;
               ReLen -= USBHSH->HOST_TX_LEN;
               Ttog ^=1;
               if( pLen )  *pLen += USBHSH->HOST_TX_LEN;
          }
        }
    }
    Delay_Us( 100 );
    ret = USBHS_Transact(((USBHSH->HOST_TX_LEN) ? USB_PID_IN<<4|DEF_ENDP_0 : USB_PID_OUT<<4|DEF_ENDP_0), 1<<3, 20000);

    if(ret != ERR_SUCCESS)            return ret;

    if ( USBHSH->HOST_TX_LEN == 0 )   return( ERR_SUCCESS );    //status stage is out, send a zero-length packet.

    if ( USBHSH->RX_LEN == 0 )        return( ERR_SUCCESS );    //status stage is in, a zero-length packet is returned indicating success.

    return ERR_USB_BUF_OVER;
}

/*********************************************************************
 * @fn      USBOTG_HostEnum
 *
 * @brief   Host enumerated device.
 *
 * @return  Error state
 */
UINT8 USBHS_HostEnum( void )
{
  UINT8 ret;

  ret = CtrlGetDevDescr(Host_Status,(UINT8 *)GetDevDescrptor);
  if(ret != ERR_SUCCESS)
  {
      printf("error1\n");
      return  ret;
  }

  ret = CtrlSetAddress(Host_Status,(UINT8 *)SetAddress,8);
  if(ret != ERR_SUCCESS)
  {
      printf("error2\n");
      return  ret;
  }

  ret = CtrlGetConfigDescr(Host_Status,(UINT8 *)GetConfigDescrptor);
  if(ret != ERR_SUCCESS)
  {
      printf("error3\n");
      return  ret;
  }

  ret = CtrlSetConfig(Host_Status,(UINT8 *)SetConfig);
  if(ret != ERR_SUCCESS)
  {
      printf("error4\n");
      return  ret;
  }

  return ERR_SUCCESS;
}

/*********************************************************************
 * @fn      USBHS_CurrentAddr
 *
 * @brief   Current device address.
 *
 * @param   address - Endpoint address.
 *
 * @return  none
 */
void USBHS_CurrentAddr( UINT8 address )
{
    USBHSH->DEV_AD &= 0x00;
    USBHSH->DEV_AD |= address; // SET ADDRESS
}

/*********************************************************************
 * @fn      Anaylisys_Descr
 *
 * @brief   Descriptor analysis.
 *
 * @param   pusbdev - host status variable.
 *          pdesc - descriptor buffer to analyze
 *          l - length
 *
 * @return  none
 */
void Analysis_Descr(PUSBHS_HOST pusbdev,PUINT8 pdesc, UINT16 l)
{
    UINT16 i;
    for(i=0;i<l;i++)                                                //分析描述符
    {
     if((pdesc[i]==0x09)&&(pdesc[i+1]==0x02))
        {
            printf("bNumInterfaces:%02x \n",pdesc[i+4]);            //配置描述符里的接口数-第5个字节
        }

     if((pdesc[i]==0x07)&&(pdesc[i+1]==0x05))
        {
            if((pdesc[i+2])&0x80)
            {
                 printf("endpIN:%02x \n",pdesc[i+2]&0x0f);              //取in端点号
                 pusbdev->DevEndp.InEndpNum = pdesc[i+2]&0x0f;
                 pusbdev->DevEndp.InEndpCount++;
                 EndpnMaxSize = ((UINT16)pdesc[i+5]<<8)|pdesc[i+4];     //取端点大小
                 pusbdev->DevEndp.InEndpMaxSize = EndpnMaxSize;
                 printf("In_endpmaxsize:%02x \n",EndpnMaxSize);
            }
            else
            {
                printf("endpOUT:%02x \n",pdesc[i+2]&0x0f);              //取out端点号
                pusbdev->DevEndp.OutEndpNum = pdesc[i+2]&0x0f;
                pusbdev->DevEndp.OutEndpCount++;
                EndpnMaxSize =((UINT16)pdesc[i+5]<<8)|pdesc[i+4];        //取端点大小
                pusbdev->DevEndp.OutEndpMaxSize = EndpnMaxSize;
                printf("Out_endpmaxsize:%02x \n",EndpnMaxSize);
            }
        }
    }
}


