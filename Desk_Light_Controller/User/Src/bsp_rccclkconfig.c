#include "bsp_rccclkconfig.h"

//Set SysClk to be 8M*pllmull
void HSE_SetSysClock(uint32_t pllmull)
{
  __IO uint32_t StartUpCounter = 0, HSEStartStatus = 0;
  
	
  /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration ---------------------------*/    
  /* Enable HSE */   
	 RCC_DeInit();
   RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready and if Time out is reached exit */
	 HSEStartStatus = RCC_WaitForHSEStartUp();

  if (HSEStartStatus == SUCCESS)
  {
    /* Enable Prefetch Buffer */
	  FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* Flash 2 wait state */ 
		FLASH_SetLatency(FLASH_Latency_2);

    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
		
    /* PCLK2 = HCLK */
    RCC_PCLK1Config(RCC_HCLK_Div1);
		
    /* PCLK1 = HCLK */
		RCC_PCLK2Config(RCC_HCLK_Div2);
		//-----------------------------------------------
		//Set PLL source and division factor
		RCC_PLLConfig(RCC_HCLK_Div1, pllmull);
		//-----------------------------------------------
		
		/* Enable PLL */
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
   
		/* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);


    /* Wait till PLL is used as system clock source */
    while (RCC_GetSYSCLKSource() != (uint32_t)0x08)
    {
    }
	}else{
		//HSE failed to start
		while(1);
	}
}

void MCO_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//Clock enable
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}