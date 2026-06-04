#include "bsp_rot_encoder.h"

void RotEncoder_Config (void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_Initstruct;
	
	ENCODER_KEY_GPIO_APBxClock_FUN(ENCODER_KEY_GPIO_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitStruct.GPIO_Pin = ENCODER_KEY_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;

	GPIO_Init(ENCODER_KEY_GPIO_PORT ,&GPIO_InitStruct);
	GPIO_EXTILineConfig(ENCODER_KEY_GPIO_PORTSOURCE, ENCODER_KEY_GPIO_PINSOURCE);
	
	EXTI_InitStruct.EXTI_Line = ENCODER_KEY_LINE;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;

	EXTI_Init(&EXTI_InitStruct);
	
	NVIC_Initstruct.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_Initstruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Initstruct.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_Initstruct.NVIC_IRQChannelSubPriority = 0;
	
	NVIC_Init(&NVIC_Initstruct);
	
}

