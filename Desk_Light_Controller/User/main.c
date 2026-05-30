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

extern __IO uint16_t ADC_ConvertedValue;

float ADC_ConvertedValueLocal;   

void Delay(__IO uint32_t nCount);


FATFS fs;
FIL fnew;
FRESULT res_flash;
UINT fnum;
BYTE buffer[1024] = {0};
BYTE textFileBuffer[] = "Hello World\r\n";
BYTE work[4096];


int main(void)
{
    TIM_RCC_Config();
    TIM_GPIO_Config();
    TIM_Output_Config();

    UART_Config();    /* Initialize USART for printf */

    printf("PWM duty cycle test start\r\n");

    for (;;)
    {
        Delay(0xffffff);
        Set_DutyCycle(TIMx, 3, 20);
        printf("TIM3_CH2 duty cycle = 20%%\r\n");

        Delay(0xffffff);
        Set_DutyCycle(TIMx, 3, 60);
        printf("TIM3_CH2 duty cycle = 60%%\r\n");

        Delay(0xffffff);
        Set_DutyCycle(TIMx, 3, 0);
        printf("TIM3_CH2 duty cycle = 0%%\r\n");

        Delay(0xffffff);
        Set_DutyCycle(TIMx, 3, 80);
        printf("TIM3_CH2 duty cycle = 80%%\r\n");

        Delay(0xffffff);
        Set_DutyCycle(TIMx, 3, 100);
        printf("TIM3_CH2 duty cycle = 100%%\r\n");
    }
}

/* -------------------- Delay Function -------------------- */

// Simple software delay using a busy loop
// NOTE:
// - Not accurate (depends on compiler optimization & clock)
// - CPU is fully occupied during delay (not efficient)
// - Only suitable for simple demos/tests
void Delay(__IO u32 nCount)
{
    for (; nCount != 0; nCount--);
}

