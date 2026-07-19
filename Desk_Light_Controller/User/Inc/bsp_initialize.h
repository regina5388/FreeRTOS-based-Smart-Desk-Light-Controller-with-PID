#ifndef BSP_INITIALIZE
#define BSP_INITIALIZE

#include "stm32f10x.h"      // STM32 standard peripheral definitions (registers, types)
#include "bsp_led.h"        // LED driver (GPIO config + control functions)
#include "bsp_key.h"        // Key (button) driver
#include "bsp_rccclkconfig.h" // System clock configuration (HSE/PLL setup)
#include "bsp_uart.h"       // UART driver (init + send/receive)
#include "bsp_dma_uart.h"
#include "bsp_i2c.h"
#include "bsp_spi_flash.h"
#include "ff.h"
#include "bsp_adc.h"
#include "bsp_timer.h"
#include "bsp_rot_encoder.h" 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "state_fsm.h"
#include "event_groups.h"
#include "bsp_i2c_light_sensor.h"
#include "pid.h"



void BSP_Init(void);

#endif
