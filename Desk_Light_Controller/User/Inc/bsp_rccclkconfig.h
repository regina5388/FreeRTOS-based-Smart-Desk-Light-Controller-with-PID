#ifndef _BSP_RCCCONFIG_H
#define _BSP_RCCCONFIG_H

#include "stm32f10x.h"

void HSE_SetSysClock(uint32_t pllmull);
void MCO_GPIO_Config(void);

#endif
