#ifndef _BSP_EXTI
#define _BSP_EXTI

#define KEY2_INT_LINE 		        EXTI_Line13
#define KEY1_INT_LINE 		        EXTI_Line0
#define KEY1_IRQHandler           EXTI0_IRQHandler
#define KEY2_IRQHandler           EXTI15_10_IRQHandler

#include "stm32f10x.h"

void EXTI_Key_Config(void);

#endif
