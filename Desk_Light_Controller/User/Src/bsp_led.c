#include "bsp_led.h"

void LED_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//Clock enable
	RCC_APB2PeriphClockCmd(LED1_GPIO_CLK|
												 LED2_GPIO_CLK|
												 LED3_GPIO_CLK , ENABLE);

	
	GPIO_InitStruct.GPIO_Pin = LED1_GPIO_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	
	GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = LED2_GPIO_PIN;

	GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = LED3_GPIO_PIN;

	GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStruct);
}

void LED1_GPIO_Set(void)
{
  GPIO_WriteBit(LED1_GPIO_PORT, LED1_GPIO_PIN, Bit_RESET);
  GPIO_WriteBit(LED1_GPIO_PORT, LED2_GPIO_PIN, Bit_SET);
  GPIO_WriteBit(LED1_GPIO_PORT, LED3_GPIO_PIN, Bit_SET);
}

void LED2_GPIO_Set(void)
{
  GPIO_WriteBit(LED1_GPIO_PORT, LED1_GPIO_PIN, Bit_SET);
  GPIO_WriteBit(LED1_GPIO_PORT, LED2_GPIO_PIN, Bit_RESET);
  GPIO_WriteBit(LED1_GPIO_PORT, LED3_GPIO_PIN, Bit_SET);
}

void LED3_GPIO_Set(void)
{
  GPIO_WriteBit(LED1_GPIO_PORT, LED1_GPIO_PIN, Bit_SET);
  GPIO_WriteBit(LED1_GPIO_PORT, LED2_GPIO_PIN, Bit_SET);
  GPIO_WriteBit(LED1_GPIO_PORT, LED3_GPIO_PIN, Bit_RESET);
}

void GPIO_Toggle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  GPIOx->ODR ^= GPIO_Pin; //XOR bit operation
}
