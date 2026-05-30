#ifndef BSP_TIM
#define BSP_TIM

#include "stm32f10x.h"

#define    TIM_APBxClock_FUN             RCC_APB1PeriphClockCmd
#define    TIMx                          TIM3
#define    TIM_CLK                       RCC_APB1Periph_TIM3
#define    TIM_OCxPRELOAD_FUN            TIM_OC3PreloadConfig
#define    TIM_OCxINIT_FUN               TIM_OC3Init


#define    LED_GPIO_APBxClock_FUN        RCC_APB2PeriphClockCmd

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

void TIM_RCC_Config(void);
void TIM_GPIO_Config(void);
void TIM_Output_Config(void);
void Set_DutyCycle(TIM_TypeDef *TIMx_Periphral, u8 channel, u8 percent);

#endif
