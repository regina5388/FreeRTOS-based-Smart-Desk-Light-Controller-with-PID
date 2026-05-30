#include "bsp_timer.h"

/**
  * @brief  Configure RCC Clock.
  * @param  None
  * @retval None
  */

void TIM_RCC_Config(void)
{
    /* Enable TIM3 peripheral clock */
    TIM_APBxClock_FUN(TIM_CLK, ENABLE);

    /* Enable LED GPIO peripheral clock */
    LED_GPIO_APBxClock_FUN(LED2_GPIO_CLK, ENABLE);
}

/**
  * @brief  Initialize GPIO pins.
  * @param  None
  * @retval None
  */
void TIM_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 2. Configure GPIO pin */
    GPIO_InitStructure.GPIO_Pin = LED2_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStructure);
}

/**
  * @brief  Configure TIM PWM output.
  * @param  None
  * @retval None
  */
void TIM_Output_Config(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    /* Configure timer base */
    TIM_TimeBaseStructure.TIM_Period = 999;                // ARR -> 1kHz
    TIM_TimeBaseStructure.TIM_Prescaler = 71;              // PSC 72MHz/ (Prescaler + 1) -> 1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);

    /* Configure PWM output channel */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 500;                   // CCR, 50% duty if ARR = 999
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //LED on for low level

    TIM_OCxINIT_FUN(TIMx, &TIM_OCInitStructure);

    /* Enable CCR preload */
    TIM_OCxPRELOAD_FUN(TIMx, TIM_OCPreload_Enable);

    /* Enable ARR preload */
    TIM_ARRPreloadConfig(TIMx, ENABLE);

    /* Enable timer counter */
    TIM_Cmd(TIMx, ENABLE);
}


/**
  * @brief  Set PWM duty cycle for TIMx channel.
  * @param  TIMx: TIM peripheral, e.g. TIM2, TIM3, TIM4
  * @param  channel: 1, 2, 3, or 4
  * @param  percent: duty cycle in percent, range 0~100
  * @retval None
  */
void Set_DutyCycle(TIM_TypeDef *TIMx_Periphral, u8 channel, u8 percent)
{
    uint16_t pulse;

    /* Limit duty cycle range */
	if (percent > 100)
    {
        percent = 100;
    }

    /* Calculate CCR value */
    pulse = (uint16_t)(((TIMx_Periphral->ARR + 1) * percent) / 100);

    /* Update selected channel */
    switch (channel)
    {
        case 1:
            TIM_SetCompare1(TIMx_Periphral, pulse);
            break;

        case 2:
            TIM_SetCompare2(TIMx_Periphral, pulse);
            break;

        case 3:
            TIM_SetCompare3(TIMx_Periphral, pulse);
            break;

        case 4:
            TIM_SetCompare4(TIMx_Periphral, pulse);
            break;

        default:
            break;
    }
}

void Set_DutyCycle_LED (u8 percent)
{
	Set_DutyCycle(TIMx, 3, percent);
}