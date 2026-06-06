#include "bsp_rot_encoder.h"

void RotEncoder_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_Initstruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_ICInitTypeDef TIM_ICInitStruct;

    /* Enable GPIO, AFIO and TIM clocks */
    ENCODER_KEY_GPIO_APBxClock_FUN(ENCODER_KEY_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    TIM_ENCODER_APBxClock_FUN(TIM_ENCODER_CLK, ENABLE);
    TIM_ENCODER_GPIO_APBxClock_FUN(TIM_ENCODER_C1_GPIO_CLK, ENABLE);
    TIM_ENCODER_GPIO_APBxClock_FUN(TIM_ENCODER_C2_GPIO_CLK, ENABLE);

    /* Configure encoder push button as input with pull-up */
    GPIO_InitStruct.GPIO_Pin = ENCODER_KEY_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(ENCODER_KEY_GPIO_PORT, &GPIO_InitStruct);

    /* Connect button GPIO pin to EXTI line */
    GPIO_EXTILineConfig(ENCODER_KEY_GPIO_PORTSOURCE,
                        ENCODER_KEY_GPIO_PINSOURCE);

    /* Configure button EXTI interrupt on falling edge */
    EXTI_InitStruct.EXTI_Line = ENCODER_KEY_LINE;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStruct);

    /* Enable EXTI interrupt in NVIC */
    NVIC_Initstruct.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_Initstruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Initstruct.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_Initstruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_Initstruct);

    /* Configure encoder A/B pins as input with pull-up */
    GPIO_InitStruct.GPIO_Pin = TIM_ENCODER_C1_GPIO_PIN;
    GPIO_Init(TIM_ENCODER_C1_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = TIM_ENCODER_C2_GPIO_PIN;
    GPIO_Init(TIM_ENCODER_C2_GPIO_PORT, &GPIO_InitStruct);

    /* Configure timer counter range for encoder counting */
    TIM_TimeBaseInitStruct.TIM_Period = 65535;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 0;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIMx_ENCODER, &TIM_TimeBaseInitStruct);

    /* Configure input capture filter for encoder channel 1 */
    TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStruct.TIM_ICFilter = 0x06;
    TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;

    /* Enable encoder interface mode: count on both TI1 and TI2 edges */
    TIM_EncoderInterfaceConfig(TIMx_ENCODER,
                               TIM_EncoderMode_TI12,
                               TIM_ICPolarity_Rising,
                               TIM_ICPolarity_Rising);

    TIM_ICInit(TIMx_ENCODER, &TIM_ICInitStruct);

    /* Configure input capture filter for encoder channel 2 */
    TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIMx_ENCODER, &TIM_ICInitStruct);

    /* Clear counter before starting encoder timer */
    TIM_SetCounter(TIMx_ENCODER, 0);

    /* Start encoder timer */
    TIM_Cmd(TIMx_ENCODER, ENABLE);
}

uint16_t RotEncoder_getCounter(void)
{
	return TIM_GetCounter(TIMx_ENCODER);
}


