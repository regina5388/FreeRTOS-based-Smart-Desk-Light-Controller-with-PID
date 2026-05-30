#include "bsp_uart.h"


//Loopback debug
static void NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_Struct;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	NVIC_Struct.NVIC_IRQChannel =  DEBUG_USART_IRQ;
	NVIC_Struct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Struct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_Struct.NVIC_IRQChannelSubPriority = 1;
	
	NVIC_Init(&NVIC_Struct);
}

void UART_Config(void)
{
	USART_InitTypeDef USART_struct;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//Open Clock
	DEBUG_USART_APBxClkCmd(DEBUG_USART_CLK, ENABLE); //defined in header file
	DEBUG_USART_GPIO_APBxClkCmd(DEBUG_USART_GPIO_CLK, ENABLE);
	
	//Uart Init
	USART_struct.USART_BaudRate = DEBUG_USART_BAUDRATE;
	USART_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_struct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_struct.USART_WordLength = USART_WordLength_8b;
	USART_struct.USART_Parity = USART_Parity_No ;
	USART_struct.USART_StopBits = USART_StopBits_1 ;
	
	USART_Init(DEBUG_USARTx, &USART_struct);
	
	//GPIO Init
	GPIO_InitStruct.GPIO_Pin = DEBUG_USART_TX_GPIO_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	
	GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStruct);
	
	
	GPIO_InitStruct.GPIO_Pin = DEBUG_USART_RX_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;

	GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStruct);
	
	//NVIC_Config();
	
	//Enable USART Interrupt
	//USART_ITConfig(DEBUG_USARTx, USART_IT_RXNE, ENABLE);
	
	//Enable Uart
	USART_Cmd(DEBUG_USARTx, ENABLE);
}


void Uart_SendByte(USART_TypeDef* pUSARTx, uint8_t Data)
{
	USART_SendData( pUSARTx, Data);
	
	//Wait for Data sending to be finished
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);

}
void Uart_SendString(USART_TypeDef* pUSARTx, char array[])
{
	unsigned int i = 0;
	while (array[i] != '\0')
	{
		Uart_SendByte( pUSARTx, array[i]);
		i++;
	}
		while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
}

int fputc(int ch, FILE *f)
{
	USART_SendData(DEBUG_USARTx, ch);
	while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);
	return (ch);
}

int fgetc(FILE *f)
{
	while(USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE)== RESET);	
	return (int)USART_ReceiveData(DEBUG_USARTx);
	
}
