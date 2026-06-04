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

void RotEncoder_Config (void);

#endif