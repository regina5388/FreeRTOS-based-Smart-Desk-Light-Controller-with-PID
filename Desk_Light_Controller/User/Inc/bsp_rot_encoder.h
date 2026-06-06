#ifndef BSP_ROT_ENCODER
#define BSP_ROT_ENCODER

#include "stm32f10x.h"

#define ENCODER_KEY_GPIO_PORT       		 GPIOC
#define ENCODER_KEY_GPIO_CLK        		 RCC_APB2Periph_GPIOC
#define ENCODER_KEY_GPIO_PIN        		 GPIO_Pin_10
#define ENCODER_KEY_IRQ 		    		 EXTI15_10_IRQn
#define ENCODER_KEY_LINE 		    		 EXTI_Line10
#define ENCODER_KEY_GPIO_PORTSOURCE  		 GPIO_PortSourceGPIOC
#define ENCODER_KEY_GPIO_PINSOURCE  		 GPIO_PinSource10
#define ENCODER_KEY_GPIO_APBxClock_FUN       RCC_APB2PeriphClockCmd

#define BUTTON_ON  0
#define BUTTON_OFF 1

#define    TIM_ENCODER_APBxClock_FUN            RCC_APB2PeriphClockCmd
#define    TIMx_ENCODER                  		TIM8
#define    TIM_ENCODER_CLK                      RCC_APB2Periph_TIM8

#define    TIM_ENCODER_GPIO_APBxClock_FUN       RCC_APB2PeriphClockCmd

#define TIM_ENCODER_C1_GPIO_PORT 	GPIOC
#define TIM_ENCODER_C1_GPIO_CLK 	RCC_APB2Periph_GPIOC
#define TIM_ENCODER_C1_GPIO_PIN 	GPIO_Pin_6

#define TIM_ENCODER_C2_GPIO_PORT 	GPIOC
#define TIM_ENCODER_C2_GPIO_CLK 	RCC_APB2Periph_GPIOC
#define TIM_ENCODER_C2_GPIO_PIN 	GPIO_Pin_7

#define STEP 5

void RotEncoder_Config (void);
uint16_t RotEncoder_getCounter(void);


#endif