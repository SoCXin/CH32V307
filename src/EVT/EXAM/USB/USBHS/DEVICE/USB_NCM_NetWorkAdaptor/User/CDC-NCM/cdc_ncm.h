/********************************** (C) COPYRIGHT *******************************
* File Name          : cdc_ncm.h
* Author             : WCH
* Version            : V1.3.0
* Date               : 2022/07/20
* Description        : cdc-ncm program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

#ifndef CDC_NCM_CDC_NCM_H_
#define CDC_NCM_CDC_NCM_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "string.h"
#include "debug.h"
#include "eth_driver.h"
#include "usb_desc.h"
#include "ch32v30x_usbhs_device.h"

typedef struct __attribute__((packed)) _ECM_SPEED
{
    uint8_t  reqtype;
    uint8_t  bnotifycode;
    uint16_t wvalue;
    uint16_t windex;
    uint16_t wlength;
    uint32_t dlspeed; /* download stream speed on bit-rate */
    uint32_t ulspeed; /* upload stream speed on bit-rate */
}ECM_SPD, *pECM_SPD;

typedef struct __attribute__((packed)) _ECM_LINK
{
    uint8_t  reqtype;
    uint8_t  bnotifycode;
    uint16_t wvalue; /* 0 = link disable; 1 = Link ready */
    uint16_t windex;
    uint16_t wlength;
}ECM_LINK, *pECM_LINK;

typedef struct __attribute__((packed)) _NCM_NTH
{
    uint32_t dwSignature;
    uint16_t wHeaderLength;
    uint16_t wSequence;
    uint16_t wBlockLength;
    uint16_t wNdpIndex;
}NCM_NTH, *pNCM_NTH;

typedef struct __attribute__((packed)) _NCM_NDP
{
    uint32_t dwSignature;
    uint16_t wLength;
    uint16_t wNextNdpIndex;
    uint16_t wDatagramInd_Len[ ];
}NCM_NDP, *pNCM_NDP;

typedef struct __attribute__((packed)) _NCM_BUFF_COMM
{
    uint16_t Flag;
    uint16_t Length;
    uint32_t Address;
}NCM_BUFF_COMM, *pNCM_BUFF_COMM;

#define ECM_SPD_Change                  ((pECM_SPD)ECM_NetWork_Speed_Change)
#define ECM_Link_Status                 ((pECM_LINK)ECM_NetWork_Connection)

#define NCM_USB_UP_BUFFSIZE             1580
#define NCM_USB_DOWN_BUFFSIZE           1580

/******************************************************************************/
/* CDC-ECM Link Status */
#define DEF_ECM_NETWORK_CONNECTION      0x00
#define DEF_ECM_NETWORK_CONNECT         1
#define DEF_ECM_NETWORK_DISCONN         0
#define DEF_ECM_RES_AVALABLE            0x01
#define DEF_ECM_CONN_SPD_CHANGE         0x2A
#define DEF_ECM_CONN_SPD_1000M          1000000000
#define DEF_ECM_CONN_SPD_100M           100000000
#define DEF_ECM_CONN_SPD_10M            10000000

/******************************************************************************/
/* CDC-ECM package filter (0x43)  */
#define DEF_ECM_PACKFILTER_MASK         0x1F
#define DEF_ECM_PACK_PROMISCUOUS        0x01
#define DEF_ECM_PACK_ALL_MULTICAST      0x02
#define DEF_ECM_PACK_DIRECTED           0x04
#define DEF_ECM_PACK_BROADCASRT         0x08
#define DEF_ECM_PACK_MULTICAST          0x10

/******************************************************************************/
/* CDC-ECM模式相关命令定义 */
#define DEF_ECM_SENDENCAPCMD            0x00                                    /* Optional：用于发起一个控制协议所支持的命令 */
#define DEF_ECM_GET_ENCAPRESPONSE       0x01                                    /* Optional：用于请求对控制协议所支持的回应 */
#define DEF_ECM_SET_ETHMULFILTERS       0x40                                    /* Optional：用于设置组播 */
#define DEF_ECM_SET_ETHPOWER            0x41                                    /* Optional：设置网络电源管理模式 */
#define DEF_ECM_GET_ETHPOWER            0x42                                    /* Optional：获取网络电源管理模式 */
#define DEF_ECM_SET_ETHPACKETFILTER     0x43                                    /* Required: 设置主机端接收以太网包的过滤器 */
#define DEF_ECM_GET_ETHSTATISTIC        0x44                                    /* Optional：获取网络设备的统计信息，包括发送包个数、接收包个数、接收错误包个数等 */

/******************************************************************************/
/* CDC-NCM模式相关命令定义 */
/* 0x40---0x44命令同ECM模式 */
#define DEF_NCM_GET_NTB_PARAMETERS      0x80                                    /* Required: 获取NTB包参数结构 */
#define DEF_NCM_GET_NET_ADDRESS         0x81                                    /* Optional：获取MAC网络地址 */
#define DEF_NCM_SET_NET_ADDRESS         0x82                                    /* Optional：设置MAC网络地址 */
#define DEF_NCM_GET_NTB_FORMAT          0x83                                    /* Optional：获取NTB包格式 */
#define DEF_NCM_SET_NTB_FORMAT          0x84                                    /* Optional：设置NTB包格式*/
#define DEF_NCM_GET_NTB_INPUT_SIZE      0x85                                    /* Required: 获取NTB包输入大小 */
#define DEF_NCM_SET_NTB_INPUT_SIZE      0x86                                    /* Required: 设置NTB包输入大小 */
#define DEF_NCM_GET_MAX_DATAGRAM_SIZE   0x87                                    /* Optional：获取现有最大数据包大小 */
#define DEF_NCM_SET_MAX_DATAGRAM_SIZE   0x88                                    /* Optional：设置现有最大数据包大小 */
#define DEF_NCM_GET_CRC_MODE            0x89                                    /* Optional：获取现有CRC模式 */
#define DEF_NCM_SET_CRC_MODE            0x8A

/******************************************************************************/
/* NET Status Definition */
#define DEF_NETSTAT_LINK_RDY            0x40 /* Valid link established */
#define DEF_NETSTAT_LINK_DIS            0x00 /* No valid link established */
#define DEF_NETSTAT_ANC_RDY             0x20 /* Auto negotiation process completed */
#define DEF_NETSTAT_ANC_DIS             0x00 /* Auto negotiation process not completed */
#define DEF_NETSTAT_1000MBITS           0x10 /* Link Speed 1000Mbits, when this bit is 1, ignore DEF_NETSTAT_100MBITS/DEF_NETSTAT_10MBITS bit definition */
#define DEF_NETSTAT_100MBITS            0x80 /* Link Speed 100Mbits */
#define DEF_NETSTAT_10MBITS             0x00 /* Link Speed 10MBits */
#define DEF_NETSTAT_FULLDUPLEX          0x01 /* Link Duplex full-duplex */
#define DEF_NETSTAT_HALFDUPLEX          0x00 /* Link Duplex half-duplex */

#ifndef QUERY_STAT_FLAG
#define QUERY_STAT_FLAG  ((LastQueryPhyTime == (LocalTime / 1000)) ? 0 : 1)
#endif

/* Ringbuffer define  */
#define DEF_U2E_REMINE                 (2)/* usb to eth 停止传输的剩余队列个数 */
#define DEF_U2E_RESTART                (2)/* usb to eth 重新开始传输的队列个数 */
#define DEF_U2E_MAXBLOCKS              (ETH_TXBUFNB)/* usb to eth 最大队列深度 */
#define DEF_U2E_PACKHEADOFFSET         (4)/* usb to eth 头部包偏移，单位uint8_t */
#define DEF_U2E_PACKTAILOFFSET         (1532)/* usb to eth 尾部偏移，单位uint8_t */
#define DEF_E2U_REMINE                 (8)/* eth to usb 停止传输的剩余队列个数 */
#define DEF_E2U_RESTART                (2)/* eth to usb 重新开始传输的队列个数 */
#define DEF_E2U_MAXBLOCKS              (ETH_RXBUFNB)/* eth to usb 最大队列深度 */
#define DEF_E2U_PACKLOADOFFSET         (4)/* usb to eth 头部包偏移，单位uint8_t */
#define DEF_U2E_PACKTAILOFFSET         (1532)/* usb to eth 尾部偏移，单位uint8_t */

/* Ring Buffer typedef */
typedef struct __attribute__((packed)) _RING_BUFF_COMM
{
    volatile uint32_t  LoadPtr;
    volatile uint32_t  DealPtr;
    volatile uint32_t  RemainPack;
    volatile uint32_t  StopFlag;
} RING_BUFF_COMM, pRING_BUFF_COMM;

extern __attribute__((__aligned__(4))) RING_BUFF_COMM U2E_Trance_Manage;
extern __attribute__((__aligned__(4))) uint32_t U2E_PackAdr[ ];
extern __attribute__((__aligned__(4))) uint32_t U2E_PackLen[ ];
extern __attribute__((__aligned__(4))) RING_BUFF_COMM E2U_Trance_Manage;
extern __attribute__((__aligned__(4))) uint32_t E2U_PackAdr[ ];
extern __attribute__((__aligned__(4))) uint32_t E2U_PackLen[ ];
extern __attribute__((__aligned__(4))) ETH_DMADESCTypeDef *DMARxDealTabs[ ];
extern __attribute__((__aligned__(4))) ETH_DMADESCTypeDef DMARxDscrTab[ ];       /* MAC receive descriptor, 4-byte aligned*/
extern __attribute__((__aligned__(4))) ETH_DMADESCTypeDef DMATxDscrTab[ ];       /* MAC send descriptor, 4-byte aligned */
extern __attribute__((__aligned__(4))) uint8_t  MACRxBuf[ ];                     /* MAC receive buffer, 4-byte aligned */
extern __attribute__((__aligned__(4))) uint8_t  MACTxBuf[ ];                     /* MAC send buffer, 4-byte aligned */
extern NCM_BUFF_COMM USB_TxManage;
extern NCM_BUFF_COMM USB_RxManage;

/* Extern Variable */
extern __attribute__ ((aligned(4))) uint8_t ECM_NetWork_Speed_Change[ ];
extern __attribute__ ((aligned(4))) uint8_t ECM_NetWork_Connection[ ];
extern __attribute__ ((aligned(4))) uint8_t NTB_Parameter_Structure[ ];
extern __attribute__ ((aligned(4))) uint8_t USB_TxBuff[ ];
extern __attribute__ ((aligned(4))) uint8_t USB_RxBuff[ ];

extern volatile uint8_t  ECM_Pack_Filter;
extern volatile uint8_t  ETH_LastStatus;
extern volatile uint8_t  ETH_NETWork_Status;
extern volatile uint32_t U2E_PackCnounter;
extern volatile uint32_t E2U_PackCnounter;
extern uint8_t  PhyInit_Flag;

/* Extern Functions */
extern void USB2ETH_Trance( void );
extern void ETH2USB_Trance( void );
extern void ECM_Load_Status( void );
extern void ECM_Change_MAC_Filter( uint8_t pac_filter );
extern void MACAddr_Change_To_SNDesc( uint8_t *pmacbuf );
extern void ETH_GetMacAddr( uint8_t *p );
extern void ETH_DriverInit( uint8_t *addr );
extern void ETH_PhyAbility_Set( void );
/* extern from eth_driver.c */
extern uint32_t LastQueryPhyTime;
extern uint32_t ETH_TxPktChainMode( uint16_t len, uint32_t *pBuff );

#ifdef __cplusplus
}
#endif

#endif /* CDC_NCM_CDC_NCM_H_ */
