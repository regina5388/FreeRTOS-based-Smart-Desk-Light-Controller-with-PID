#include "bsp_exti.h"

#define KEY1_INT_GPIO_PORT        GPIOA
#define KEY1_INT_GPIO_CLK         RCC_APB2Periph_GPIOA
#define KEY1_INT_GPIO_PIN         GPIO_Pin_0
#define KEY1_INT_IRQ 		          EXTI0_IRQn
#define KEY1_INT_LINE 		        EXTI_Line0
#define KEY1_INT_GPIO_PORTSOURCE  GPIO_PortSourceGPIOA
#define KEY1_INT_GPIO_PINSOURCE   GPIO_PinSource0
#define KEY1_IRQHandler           EXTI0_IRQHandler

#define KEY2_INT_GPIO_PORT        GPIOC
#define KEY2_INT_GPIO_CLK         RCC_APB2Periph_GPIOC
#define KEY2_INT_GPIO_PIN         GPIO_Pin_13
#define KEY2_INT_IRQ 		          EXTI15_10_IRQn
#define KEY2_INT_LINE 		        EXTI_Line13
#define KEY2_INT_GPIO_PORTSOURCE  GPIO_PortSourceGPIOC
#define KEY2_INT_GPIO_PINSOURCE   GPIO_PinSource13
#define KEY2_IRQHandler           EXTI15_10_IRQHandler

static void NVIC_Config(void)	
{
	NVIC_InitTypeDef NVIC_Struct;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	NVIC_Struct.NVIC_IRQChannel =  KEY1_INT_IRQ;
	NVIC_Struct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Struct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_Struct.NVIC_IRQChannelSubPriority = 0;
	
	NVIC_Init(&NVIC_Struct);

	NVIC_Struct.NVIC_IRQChannel = KEY2_INT_IRQ;
	NVIC_Struct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Struct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_Struct.NVIC_IRQChannelSubPriority = 0;
	
	NVIC_Init(&NVIC_Struct);
	
}

void EXTI_Key_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_Struct;
	
	//Clock enable
	RCC_APB2PeriphClockCmd(KEY1_INT_GPIO_CLK|
												 KEY2_INT_GPIO_CLK|
												RCC_APB2Periph_AFIO, ENABLE);

	NVIC_Config();
	
	GPIO_InitStruct.GPIO_Pin = KEY1_INT_GPIO_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	
	GPIO_Init(KEY1_INT_GPIO_PORT, &GPIO_InitStruct);
	
	GPIO_EXTILineConfig(KEY1_INT_GPIO_PORTSOURCE, KEY1_INT_GPIO_PINSOURCE);

	GPIO_InitStruct.GPIO_Pin = KEY2_INT_GPIO_PIN;

	GPIO_Init(KEY2_INT_GPIO_PORT, &GPIO_InitStruct);
	GPIO_EXTILineConfig(KEY2_INT_GPIO_PORTSOURCE, KEY2_INT_GPIO_PINSOURCE);
	
	EXTI_Struct.EXTI_Line = KEY1_INT_LINE;
	EXTI_Struct.EXTI_LineCmd = ENABLE;
	EXTI_Struct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_Struct.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_Init(&EXTI_Struct);

	EXTI_Struct.EXTI_Line = KEY2_INT_LINE;
	EXTI_Struct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_Struct);
	
}

