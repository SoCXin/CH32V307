/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
*******************************************************************************/

/*
 *@Note
 TouchKey������̣�
   ��������ʾ ͨ��2(PA2)���� Touchkey Ӧ�á�
 
*/

#include "debug.h"

/* Global define */


/*******************************************************************************
* Function Name  : Touch_Key_Init
* Description    : Initializes Touch Key collection.
* Input          : None
* Return         : None
*******************************************************************************/
void Touch_Key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE );
    RCC_ADCCLKConfig(RCC_PCLK2_Div4);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

	ADC_Cmd(ADC1, ENABLE);
	TKey1->CTLR1 |= (1<<26)|(1<<24);     // Enable TouchKey and Buffer
}

/*******************************************************************************
* Function Name  : Touch_Key_Adc
* Description    : Returns ADCx conversion result data.
* Input          : ch: ADC channel.
*                    ADC_Channel_0: ADC Channel0 selected.
*                    ADC_Channel_1: ADC Channel1 selected.
*                    ADC_Channel_2: ADC Channel2 selected.
*                    ADC_Channel_3: ADC Channel3 selected.
*                    ADC_Channel_4: ADC Channel4 selected.
*                    ADC_Channel_5: ADC Channel5 selected.
*                    ADC_Channel_6: ADC Channel6 selected.
*                    ADC_Channel_7: ADC Channel7 selected.
*                    ADC_Channel_8: ADC Channel8 selected.
*                    ADC_Channel_9: ADC Channel9 selected.
*                    ADC_Channel_10: ADC Channel10 selected.
*                    ADC_Channel_11: ADC Channel11 selected.
*                    ADC_Channel_12: ADC Channel12 selected.
*                    ADC_Channel_13: ADC Channel13 selected.
*                    ADC_Channel_14: ADC Channel14 selected.
*                    ADC_Channel_15: ADC Channel15 selected.
* Return         : val: The Data conversion value.
*******************************************************************************/
u16 Touch_Key_Adc(u8 ch)
{
  ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_7Cycles5 );
  TKey1->IDATAR1 =0x1c;
  TKey1->RDATAR =0x8;
  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));
  return (uint16_t) TKey1->RDATAR;

}

/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Return         : None
*******************************************************************************/
int main(void)
{
	u16 ADC_val;

	Delay_Init();
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n",SystemCoreClock);

	Touch_Key_Init();
	while(1)
	{
	    ADC_val = Touch_Key_Adc(ADC_Channel_2);
	    printf("TouckKey Value:%d\r\n",ADC_val);
	    Delay_Ms(500);
	}
}
