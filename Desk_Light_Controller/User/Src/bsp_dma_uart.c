#include "bsp_dma_uart.h"
uint8_t SendBuff[SENDBUFF_SIZE];

void DMA_UART_Config(void)
{
	DMA_InitTypeDef DMA_init;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	
	DMA_init.DMA_BufferSize= SENDBUFF_SIZE;
	DMA_init.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_init.DMA_M2M = DMA_M2M_Disable;
	DMA_init.DMA_MemoryBaseAddr = (u32)SendBuff;
	DMA_init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_init.DMA_Mode=DMA_Mode_Normal ;
	DMA_init.DMA_PeripheralBaseAddr = USART_DR_ADDRESS;
	DMA_init.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
	DMA_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_init.DMA_Priority = DMA_Priority_Medium ;
	
	DMA_Init(USART_TX_DMA_CHANNEL, &DMA_init);
	DMA_Cmd(USART_TX_DMA_CHANNEL, ENABLE);

}
