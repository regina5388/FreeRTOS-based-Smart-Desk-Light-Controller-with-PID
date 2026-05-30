#include "bsp_spi_flash.h"

#define Dummy_Byte				0xFF
#define WIP_Flag				0x01 // Write in progress Flag in FLASH

static uint32_t SPITimeout;

typedef enum 
{
	SPI_TIMEOUT_SENDBYTE = 0,          // Timeout waiting for EV5 (Master mode select after START)
	
    SPI_TIMEOUT_RECEIVEBYTE         // Timeout waiting for EV6 (Address sent, transmitter mode selected)

}SPI_TIMEOUT_ErrorCode;

static uint32_t SPI_TIMEOUT_UserCallback(uint8_t errorCode)
{
	FLASH_ERROR("SPI time out! errorCode = %d",errorCode);
	return 0;
}

void SPI_FLASH_Init(void)
{
		//Structures declaration
	GPIO_InitTypeDef GPIO_InitStruct;
	SPI_InitTypeDef SPI_InitStruct;

	//Clock enable
	FLASH_SPI_APBxClock_FUN(FLASH_SPI_CLK, ENABLE);
	FLASH_SPI_CS_APBxClock_FUN (FLASH_SPI_CS_CLK , ENABLE);
	FLASH_SPI_SCK_APBxClock_FUN (FLASH_SPI_SCK_CLK , ENABLE);
	FLASH_SPI_MISO_APBxClock_FUN (FLASH_SPI_MISO_CLK , ENABLE);
	FLASH_SPI_MOSI_APBxClock_FUN (FLASH_SPI_MOSI_CLK , ENABLE);
	
	//GPIO Init
	GPIO_InitStruct.GPIO_Pin = FLASH_SPI_CS_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(FLASH_SPI_CS_PORT , &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = FLASH_SPI_SCK_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(FLASH_SPI_SCK_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = FLASH_SPI_MISO_PIN;
	GPIO_Init(FLASH_SPI_SCK_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = FLASH_SPI_MOSI_PIN;
	GPIO_Init(FLASH_SPI_SCK_PORT, &GPIO_InitStruct);
	
	SPI_FLASH_CS_HIGH();
	
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;//Sampling at even numbers of clock edge
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_High; // Signal high at idle state
	SPI_InitStruct.SPI_CRCPolynomial = 7; //Invalid, since slave Flash does not need CRC
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	
	SPI_Init(FLASH_SPIx, &SPI_InitStruct);
	SPI_Cmd(FLASH_SPIx, ENABLE);
}




/**
  * @brief  Sends one byte through SPI and returns the byte received at the same time.
  * @param  byte: The byte to be sent through SPI.
  * @retval The byte received from the SPI bus, or a timeout callback return value if an error occurs.
  */
static u8 SPI_Flash_SendByte(u8 byte)
{
	SPITimeout = SPIT_FLAG_TIMEOUT;
	
	//Wait until send buffer is empty
	while (SPI_I2S_GetFlagStatus(FLASH_SPIx, SPI_I2S_FLAG_TXE) == RESET)
	{
		if (SPITimeout-- == 0)
		{
			return SPI_TIMEOUT_UserCallback(SPI_TIMEOUT_SENDBYTE);
		}
	}
	
	SPI_I2S_SendData(FLASH_SPIx,byte);
	
	//Wait until receiving buffer is not empty
	
		while (SPI_I2S_GetFlagStatus(FLASH_SPIx, SPI_I2S_FLAG_RXNE) == RESET)
	{
		if (SPITimeout-- == 0)
		{
			return SPI_TIMEOUT_UserCallback(SPI_TIMEOUT_SENDBYTE);
		}
	}
	
	return SPI_I2S_ReceiveData(FLASH_SPIx);

}



/**
  * @brief  Receives one byte from SPI by sending a dummy byte to generate clock pulses.
  * @param  None
  * @retval The byte received from the SPI bus.
  */
static u8 SPI_Flash_ReceiveByte(void)
{

	return SPI_Flash_SendByte(Dummy_Byte);
}


/**
  * @brief  Reads the SPI Flash device ID.
  * @param  None
  * @retval 24-bit device ID read from the SPI Flash.
  */

u32 SPI_Flash_ReadID(void)
	
{
	u32 Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
	
	SPI_FLASH_CS_LOW(); // Start SPI
	
	SPI_Flash_SendByte(W25X_JedecDeviceID);
	
	Temp0 = SPI_Flash_ReceiveByte(); // Manufacture ID
	
	Temp1 = SPI_Flash_ReceiveByte(); // Device type
	
	Temp2 = SPI_Flash_ReceiveByte(); // Storage
	
	SPI_FLASH_CS_HIGH(); // Finish SPI
	
	Temp = (Temp0 << 16)|(Temp1 << 8)| Temp2;
	
	return Temp;
	
}

u32 SPI_Flash_ReadDeviceID(void)
	
{
	u32 ID = 0;
	
	SPI_FLASH_CS_LOW(); // Start SPI
	
	SPI_Flash_SendByte(W25X_DeviceID);
	
	
	SPI_Flash_SendByte(Dummy_Byte);
	SPI_Flash_SendByte(Dummy_Byte);
	SPI_Flash_SendByte(Dummy_Byte);
	
	ID = SPI_Flash_ReceiveByte(); // Storage
	
	SPI_FLASH_CS_HIGH(); // Finish SPI
	
	
	return ID;
	
}

/**
  * @brief  Enables write operations on the SPI Flash.
  * @param  None
  * @retval None
  */

void SPI_Flash_WriteEnable(void)
	
{
	SPI_FLASH_CS_LOW();
	
	SPI_Flash_SendByte(W25X_WriteEnable);
	
	SPI_FLASH_CS_HIGH();
	
}

/**
  * @brief  Waits until the SPI Flash write operation is completed.
  * @param  None
  * @retval None
  */
void SPI_Flash_WaitForWriteEnd(void)
	
{
	u8 Status;
	
	SPI_FLASH_CS_LOW();
	
	SPI_Flash_SendByte(W25X_ReadStatusReg);
	
	do
	{
		Status = SPI_Flash_ReceiveByte();
	
	}while((Status & WIP_Flag)== SET); //Mask Status with 0x01, get only FIG Flag
	
	SPI_FLASH_CS_HIGH();
	
}

/**
  * @brief  Erases one sector of the SPI Flash at the specified address.
  * @param  SectorAddress: Address inside the sector to be erased.
  * @retval None
  */
void SPI_Flash_SectorErase(u32 SectorAddress)
{
	SPI_Flash_WriteEnable();
	SPI_Flash_WaitForWriteEnd(); //Wait for previous process to finish writing
	
	SPI_FLASH_CS_LOW();
	
	SPI_Flash_SendByte(W25X_SectorErase);
	
	SPI_Flash_SendByte((SectorAddress & 0xFF0000) >> 16);
	SPI_Flash_SendByte((SectorAddress & 0xFF00) >> 8);
	SPI_Flash_SendByte(SectorAddress & 0xFF);
	
	SPI_FLASH_CS_HIGH();
	
	SPI_Flash_WaitForWriteEnd();
}

/**
  * @brief  Writes data to one page of the SPI Flash.
  * @param  pBuffer: Pointer to the data buffer to be written.
  * @param  WriteAddress: Start address in SPI Flash.
  * @param  NumByteToWrite: Number of bytes to write, limited to 256.
  * @retval None
  */

static void SPI_Flash_PageWrite(u8 *pBuffer, u32 WriteAddress, u16 NumByteToWrite)
{
	

	SPI_Flash_WriteEnable();
	SPI_Flash_WaitForWriteEnd();
	
	
	SPI_FLASH_CS_LOW();
	
	SPI_Flash_SendByte(W25X_PageProgram);
	
	SPI_Flash_SendByte((WriteAddress & 0xFF0000) >> 16);
	SPI_Flash_SendByte((WriteAddress & 0xFF00) >> 8);
	SPI_Flash_SendByte(WriteAddress & 0xFF);
	
	if (NumByteToWrite > SPI_FLASH_PerWritePageSize)

	{
		NumByteToWrite = SPI_FLASH_PerWritePageSize;
		FLASH_ERROR("SPI Flash write size too large!");
	}

	while (NumByteToWrite --)
	{
		SPI_Flash_SendByte(*pBuffer);
		pBuffer ++;
	}
	
	SPI_FLASH_CS_HIGH();
	
	SPI_Flash_WaitForWriteEnd();
	
}

/**
  * @brief  Writes a buffer of data to SPI Flash across one or more pages.
  *
  *         This function handles page boundary alignment automatically.
  *
  *         Branch description:
  *         - If WriteAddress is page-aligned:
  *           1. If data length is less than one page, write it directly.
  *           2. If data length is one page or more, write full pages first,
  *              then write the remaining bytes.
  *
  *         - If WriteAddress is not page-aligned:
  *           1. If all data fits in the remaining space of the current page,
  *              write it directly.
  *           2. If data crosses the current page boundary, write enough bytes
  *              to fill the current page first, then continue writing the rest
  *              from the next page.
  *           3. If more than one page of data remains, write full pages first,
  *              then write the remaining bytes.
  *
  * @param  pBuffer: Pointer to the data buffer to be written.
  * @param  WriteAddress: Start address in SPI Flash.
  * @param  NumByteToWrite: Number of bytes to write.
  * @retval None
  */
  
void SPI_Flash_BufferWrite(u8 *pBuffer, u32 WriteAddress, u16 NumByteToWrite)
{
	u8 NumOfPage = 0, NumofSingle = 0, AddrMod = 0, count = 0, PageOverFlow = 0;
	
	AddrMod = WriteAddress % SPI_FLASH_PageSize;	
	count = SPI_FLASH_PageSize - AddrMod;// Number of Bytes needed for a Page
	
	NumOfPage = NumByteToWrite / SPI_FLASH_PageSize;	
	NumofSingle =  NumByteToWrite % SPI_FLASH_PageSize;
	
	//Page Aligned
	if(AddrMod == 0)
	{
		 //Only one page
		 if(NumOfPage == 0) //integer division, decimal parts discarded
		 {
			SPI_Flash_PageWrite(pBuffer,WriteAddress, NumByteToWrite);
		 }
		 //More than one page
		 else
		 {  //Write whole pages
			while(NumOfPage--)
			{
				SPI_Flash_PageWrite(pBuffer,WriteAddress, SPI_FLASH_PageSize);
				pBuffer += SPI_FLASH_PageSize;
				WriteAddress += SPI_FLASH_PageSize;
				
			}
			//Write leftovers less than 1 page
			if(NumofSingle != 0)
			{
				SPI_Flash_PageWrite(pBuffer, WriteAddress, NumofSingle);
			}	
		 }
	}
	else
	{
		//Only one page
		 if(NumOfPage == 0)
		 {
			if( NumofSingle > count)// current page starting from WriteAddress is not enough
			{
				PageOverFlow = NumofSingle - count;
				SPI_Flash_PageWrite(pBuffer, WriteAddress, count);
				
				pBuffer += count;
				WriteAddress += count;
				
				SPI_Flash_PageWrite(pBuffer, WriteAddress, PageOverFlow);
				
			}
			else // current page starting from WriteAddress is enough
			{
				SPI_Flash_PageWrite(pBuffer, WriteAddress, NumofSingle);
			}
		 }
		 else //First write not aligned data to align address, then the rest
		 {
			NumByteToWrite -= count;
			NumOfPage = NumByteToWrite / SPI_FLASH_PageSize;	
			NumofSingle =  NumByteToWrite % SPI_FLASH_PageSize;
			
			SPI_Flash_PageWrite(pBuffer, WriteAddress, count);
			pBuffer += count;
			WriteAddress += count; 
			 
			while(NumOfPage--)
			{
				SPI_Flash_PageWrite(pBuffer,WriteAddress, SPI_FLASH_PageSize);
				pBuffer += SPI_FLASH_PageSize;
				WriteAddress += SPI_FLASH_PageSize;
				
			}
			//Write leftovers less than 1 page
			if(NumofSingle != 0)
			{
				SPI_Flash_PageWrite(pBuffer, WriteAddress, NumofSingle);
			}	
			 
		 }
	}

}


/**
  * @brief  Reads a buffer of data from SPI Flash.
  * @param  pBuffer: Pointer to the buffer used to store the received data.
  * @param  ReadAddress: Start address in SPI Flash.
  * @param  NumByteToRead: Number of bytes to read.
  * @retval None
  */
void SPI_Flash_BufferRead(u8 *pBuffer, u32 ReadAddress, u16 NumByteToRead)
{
	SPI_FLASH_CS_LOW();
	
	SPI_Flash_SendByte(W25X_ReadData);
	
	SPI_Flash_SendByte((ReadAddress & 0xFF0000) >> 16);
	SPI_Flash_SendByte((ReadAddress & 0xFF00) >> 8);
	SPI_Flash_SendByte(ReadAddress & 0xFF);
	
	while(NumByteToRead--)
	{
		*pBuffer = SPI_Flash_ReceiveByte();
		pBuffer ++; 
	}
	
	SPI_FLASH_CS_HIGH();
}

/**
  * @brief  Wake up device from power down mode
  * @retval None
  */

void SPI_Flash_WakeUp(void) 
{

  SPI_FLASH_CS_LOW();


  SPI_Flash_SendByte(W25X_ReleasePowerDown);


  SPI_FLASH_CS_HIGH();
}   

