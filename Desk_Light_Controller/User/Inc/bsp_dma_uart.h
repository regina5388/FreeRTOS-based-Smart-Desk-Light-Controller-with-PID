#ifndef _BSP_DMA
#define _BSP_DMA

#include "stm32f10x.h"
#include <stdio.h>


// UART Definition
#define  DEBUG_USARTx                   USART1
#define  DEBUG_USART_CLK                RCC_APB2Periph_USART1
#define  DEBUG_USART_APBxClkCmd         RCC_APB2PeriphClockCmd
#define  DEBUG_USART_BAUDRATE           115200

// UART GPIO Definition
#define  DEBUG_USART_GPIO_CLK           (RCC_APB2Periph_GPIOA)
#define  DEBUG_USART_GPIO_APBxClkCmd    RCC_APB2PeriphClockCmd
    
#define  DEBUG_USART_TX_GPIO_PORT       GPIOA   
#define  DEBUG_USART_TX_GPIO_PIN        GPIO_Pin_9
#define  DEBUG_USART_RX_GPIO_PORT       GPIOA
#define  DEBUG_USART_RX_GPIO_PIN        GPIO_Pin_10

// UART DMA Channel
#define  USART_TX_DMA_CHANNEL     DMA1_Channel4

// UART Data register Address
#define  USART_DR_ADDRESS        (USART1_BASE+0x04)

//Data amount to be send at a time
#define  SENDBUFF_SIZE            5000



void DMA_UART_Config(void);

#endif
