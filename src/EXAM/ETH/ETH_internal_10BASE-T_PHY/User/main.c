/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
*******************************************************************************/

/*
 *@Note 
 ETH及内部10BASE-T物理层演示例程：
 以太网打印出接收到的帧的前22个字节，并发送固定的帧到对端。
 注意：本例程在主频为144MHz下演示，不使用MII。
*/

#include "my_eth_driver.h"
#include "debug.h"
#include "main.h"

/* Global Variable */
ETH_DMADESCTypeDef /* 发送描述符表 */DMARxDscrTab[16];
ETH_DMADESCTypeDef /* 接收描述符表 */DMATxDscrTab[2];
uint8_t /* 接收队列 */Rx_Buff[ETH_RXBUFNB*2][ETH_MAX_PACKET_SIZE];
uint8_t /* 发送队列 */Tx_Buff[ETH_TXBUFNB*2][ETH_MAX_PACKET_SIZE];
volatile FrameTypeDef frame;

/* Transport sample */
uint8_t ARP_package[]=
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,/* 目标MAC */
    0x84, 0xc2, 0xe4, 0x01, 0x02, 0x03,/* 源MAC */
    0x08,0x06,/* ARP包 */
    0x00,0x01,/* 硬件类型 */
    0x08,0x00,/* 协议类型 */
    0x06,0x04,/* 硬件地址长度和协议地址长度 */
    0x00,0x01,/* 情求包 */
    0x84,0xc2,0xe4,0x01,0x02,0x03,/* 源MAC地址 */
    0xc0,0xa8,0x1,0x0f,/* 源协议地址 */
    0x00,0x00,0x00,0x00,0x00,0x00,/* 目标MAC地址 */
    0xc0,0xa8,0x01,0x65,/* 源协议地址 */
#if 1/*padding 16bytes*行数   */
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x66,0x55,0x55,0xee,0x96,0x85,0x25,0xaa
#endif
};

/*******************************************************************************
* Function Name  : SET_MCO
* Description    : Set MCO wave output.
* Input          : None
* Return         : None
*******************************************************************************/
void SET_MCO(void)
{
    GPIO_InitTypeDef GPIO;

    RCC_AHBPeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

    GPIO.GPIO_Pin = GPIO_Pin_8;
    GPIO.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOA,&GPIO);

    RCC_PLL3Cmd(DISABLE);
    RCC_PREDIV2Config(RCC_PREDIV2_Div2);
    RCC_PLL3Config(RCC_PLL3Mul_15);
    RCC_MCOConfig(RCC_MCO_PLL3CLK);
    RCC_PLL3Cmd(ENABLE);
    Delay_Ms(100);
    while(RESET == RCC_GetFlagStatus(RCC_FLAG_PLL3RDY))
    {
        printf("Wait for PLL3 ready.\n");
        Delay_Ms(500);
    }
    RCC_AHBPeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
}

/*******************************************************************************
* Function Name  : TIM1_Init
* Description    : Ethernet send counter.
* Input          : None
* Return         : None
*******************************************************************************/
void TIM1_Init( void )
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); /* 打开TIM1时钟  */

    TIM_TimeBaseStructure.TIM_Period = 500; /* 计数值 */
    TIM_TimeBaseStructure.TIM_Prescaler =9600; /* 预分频值 */
    TIM_TimeBaseStructure.TIM_ClockDivision = 0; /* 时钟分割  */
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  /* 计数方向  */
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //

    TIM_ITConfig(TIM1, TIM_IT_Update ,ENABLE);/* 使能更新中断 */

    TIM_Cmd(TIM1, ENABLE);
}

/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Return         : None
*******************************************************************************/
int main(void)
{
    RCC_ClocksTypeDef RCC_ClocksStatus;
    uint16_t recv_length,i;
    uint8_t * recv_ptr;

	Delay_Init();
	USART_Printf_Init(115200);
    RCC_GetClocksFreq(&RCC_ClocksStatus);
    printf("WCH Ethernet internal 10BASE-T PHY test program.Complair @ %s,%s.\n",__DATE__,__TIME__);
    printf("SystemClk:%d Hz.\n",RCC_ClocksStatus.SYSCLK_Frequency);
    /* Set PLL3 output as 60Hz */
    SET_MCO();
    /* TIM1 initialization */
    TIM1_Init();
    /* ETH initialization */
    Ethernet_Configuration();
    ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
    ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);
    ETH_Start();

    printf("Enter main loop.\n");
	while(1)
	{
	    /* 是否接收到数据 */
	    if(frame.length)
	    {
	        recv_length = frame.length;
	        frame.length=0;
	        recv_ptr = (uint8_t *)frame.buffer;
            printf("receive counter:%d.\n",ETH->MMCRGUFCR);
            printf("read_length:%d\n",recv_length);
            printf("Received@0x%08x\n",recv_ptr);
            for(i=0;i<6;i++)
                printf("%02x ",*(uint8_t*)(recv_ptr+i));
                printf("\n");
            for(i=6;i<12;i++)
                printf("%02x ",*(uint8_t*)(recv_ptr+i));
                printf("\n");
            for(i=12;i<22;i++)
                printf("%02x ",*(uint8_t*)(recv_ptr+i));
            printf("END!\n\n");
	    }

	    /* 是否启动发送数据 */
	    if( TIM_GetITStatus(TIM1, TIM_IT_Update)  )
	    {
	        TIM_ClearITPendingBit(TIM1, TIM_IT_Update  );

            mac_send(ARP_package,sizeof(ARP_package));
            printf("Send length:%d.\n",sizeof(ARP_package));
            printf("MMCTGFCR:%d.\n",ETH->MMCTGFCR);
            printf("\n");
	    }
	}
}







