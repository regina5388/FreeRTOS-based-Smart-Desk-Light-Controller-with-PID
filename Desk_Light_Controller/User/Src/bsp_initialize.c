#include "bsp_initialize.h"

void BSP_Init(void)
{
	//Use 4 bits for Priority
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    
	TIM_RCC_Config();
    TIM_GPIO_Config();
    TIM_Output_Config();

    UART_Config();    /* Initialize USART for printf */
	RotEncoder_Config();
	FSM_init();
}
