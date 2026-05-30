#ifndef _BSP_SYSTICK
#define _BSP_SYSTICK

#include "stm32f10x.h"

void Systick_Init(void);
void Delay_10us(uint32_t nTime);
void IntCounterDecrement(void);

#endif
