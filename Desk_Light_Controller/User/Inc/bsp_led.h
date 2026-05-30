#ifndef _LED_H
#define _LED_H

#include "stm32f10x.h"
//Red
#define LED1_GPIO_PORT GPIOB
#define LED1_GPIO_CLK RCC_APB2Periph_GPIOB
#define LED1_GPIO_PIN GPIO_Pin_5
//Green
#define LED2_GPIO_PORT GPIOB
#define LED2_GPIO_CLK RCC_APB2Periph_GPIOB
#define LED2_GPIO_PIN GPIO_Pin_0
//Blue
#define LED3_GPIO_PORT GPIOB
#define LED3_GPIO_CLK RCC_APB2Periph_GPIOB
#define LED3_GPIO_PIN GPIO_Pin_1


//Functions
void LED_GPIO_Config(void);
void LED1_GPIO_Set(void);
void LED2_GPIO_Set(void);
void LED3_GPIO_Set(void);
void GPIO_Toggle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);


#endif
