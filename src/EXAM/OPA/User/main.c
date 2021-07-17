/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
*******************************************************************************/

/*
 *@Note
 OPA4作电压跟随器输出，DAC输出给OPA4，ADC采样OPA4
 DAC_CH1——PA4
 OPA4_CHP1——PC4
 OPA4_CHN1——PC3
 OPA4_OUT_IO_ADC——PA0

   本例中PA0和PC3短接，PA4接PC4，若负反馈接电阻，也可构成运算放大器
*/

#include "debug.h"

/* Global define */


/* Global Variable */ 



/*******************************************************************************
* Function Name  : OPA4_Init
* Description    : Initializes OPA4 collection.
* Input          : None
* Return         : None
*******************************************************************************/
void OPA4_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    OPA_InitTypeDef  OPA_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE );
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    OPA_InitStructure.OPA_NUM=OPA4;
    OPA_InitStructure.PSEL=CHP1;
    OPA_InitStructure.NSEL=CHN1;
    OPA_InitStructure.Mode=OUT_IO_ADC;
    OPA_Init(&OPA_InitStructure);
    OPA_Cmd(OPA4,ENABLE);

}

/*******************************************************************************
* Function Name  : Dac_Channel1_Init
* Description    : Initializes DAC Channel1 collection.
* Input          : None
* Return         : None
*******************************************************************************/ 
void Dac_Channel1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	DAC_InitTypeDef DAC_InitType;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE );
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 		     
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
	
	DAC_InitType.DAC_Trigger=DAC_Trigger_None;
	DAC_InitType.DAC_WaveGeneration=DAC_WaveGeneration_None;
	DAC_InitType.DAC_LFSRUnmask_TriangleAmplitude=DAC_TriangleAmplitude_4095;
	DAC_InitType.DAC_OutputBuffer=DAC_OutputBuffer_Enable ;
    DAC_Init(DAC_Channel_1,&DAC_InitType);
    DAC_Cmd(DAC_Channel_1, ENABLE);


    DAC_SetChannel1Data(DAC_Align_12b_R, 0);
}


/*******************************************************************************
* Function Name  : ADC_Channel0_Init
* Description    : Initializes ADC Channel0 collection.
* Input          : None
* Return         : None
*******************************************************************************/
void ADC_Channel0_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE );
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_Cmd(ADC1, ENABLE);
    ADC1->CTLR1 |= (1<<26);      //buffer en
}

/*******************************************************************************
* Function Name  : Get_ADC_Val
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
*                    ADC_Channel_16: ADC Channel16 selected.
*                    ADC_Channel_17: ADC Channel17 selected.
* Return         : val: The Data conversion value.
*******************************************************************************/
u16 Get_ADC_Val(u8 ch)
{
  u16 val;

    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));

    val = ADC_GetConversionValue(ADC1);

    return val;
}

/*******************************************************************************
* Function Name  : Get_ADC_Average
* Description    : Returns ADCx conversion result average data.
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
*                    ADC_Channel_16: ADC Channel16 selected.
*                    ADC_Channel_17: ADC Channel17 selected.
* Return         : val: The Data conversion value.
*******************************************************************************/
u16 Get_ADC_Average(u8 ch,u8 times)
{
    u32 temp_val=0;
    u8 t;
    u16 val;

    for(t=0;t<times;t++)
    {
        temp_val+=Get_ADC_Val(ch);
        Delay_Ms(5);
    }

    val = temp_val/times;

    return val;
}


/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Return         : None
*******************************************************************************/
int main(void)
{
    u16 ADC_val,i;
    u16 DAC_OUT[6]={0,100,500,1000,2000,3000};
    Delay_Init();
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n",SystemCoreClock);
	printf("OPA Test\r\n");
	Dac_Channel1_Init();
	OPA4_Init();
	ADC_Channel0_Init();

    while(1)
   {
        for(i=0;i<6;i++)
        {
            DAC_SetChannel1Data(DAC_Align_12b_R, DAC_OUT[i]);
            ADC_val = Get_ADC_Average( ADC_Channel_0, 10 );
            printf( "OPA_OUT:%04d\r\n", ADC_val );
            Delay_Ms(500);
        }
   }
}

