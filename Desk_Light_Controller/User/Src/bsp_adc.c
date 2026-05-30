#include "bsp_adc.h"

__IO uint16_t ADC_ConvertedValue;

void ADCx_Init(void){

	GPIO_InitTypeDef GPIO_InitStruct;
	ADC_InitTypeDef ADC_InitStruct;
	
	ADC_GPIO_APBxClock_FUN(RCC_APB2Periph_GPIOC,ENABLE);
	ADC_APBxClock_FUN(ADC_CLK, ENABLE);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Pin = ADC_PIN;
	
	GPIO_Init(ADC_PORT,&GPIO_InitStruct);
	
	ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStruct.ADC_NbrOfChannel = 1;
	ADC_InitStruct.ADC_ScanConvMode = DISABLE;
	
	ADC_Init(ADCx, &ADC_InitStruct);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
	ADC_RegularChannelConfig(ADCx, ADC_CHANNEL, 1, ADC_SampleTime_1Cycles5);
	
	ADC_ITConfig(ADCx,ADC_IT_EOC, ENABLE);
	
	ADC_Cmd(ADCx, ENABLE);
	
	ADC_ResetCalibration(ADCx);
	while(ADC_GetResetCalibrationStatus(ADCx));
	

	ADC_StartCalibration(ADCx);
	while(ADC_GetCalibrationStatus(ADCx));
	 
	ADC_SoftwareStartConvCmd(ADCx, ENABLE);
}

void ADC_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	NVIC_InitStruct.NVIC_IRQChannel = ADC_IRQ;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	
	NVIC_Init(&NVIC_InitStruct);

}