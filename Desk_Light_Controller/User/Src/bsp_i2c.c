#include "bsp_i2c.h"
#include "bsp_uart.h"	
	
uint16_t EEPROM_ADDRESS;
static __IO uint32_t  I2CTimeout;
	
typedef enum
{
    I2C_TIMEOUT_EV5_START = 0,          // Timeout waiting for EV5 (Master mode select after START)

    I2C_TIMEOUT_EV6_ADDR_WRITE,         // Timeout waiting for EV6 (Address sent, transmitter mode selected)
    I2C_TIMEOUT_EV8_ADDR,               // Timeout waiting for EV8 (Memory address transmitted)
    I2C_TIMEOUT_EV8_DATA,               // Timeout waiting for EV8 (Data byte transmitted)

    I2C_TIMEOUT_BUSY_FLAG,              // Timeout waiting for I2C bus not busy

    I2C_TIMEOUT_PAGE_EV5_START,         // Page write: EV5 timeout
    I2C_TIMEOUT_PAGE_EV6_ADDR,          // Page write: EV6 timeout
    I2C_TIMEOUT_PAGE_EV8_ADDR,          // Page write: EV8 (address phase) timeout
    I2C_TIMEOUT_PAGE_EV8_DATA,          // Page write: EV8 (data phase) timeout

    I2C_TIMEOUT_READ_BUSY,              // Read: bus busy timeout
    I2C_TIMEOUT_READ_EV5_START,         // Read: EV5 timeout
    I2C_TIMEOUT_READ_EV6_ADDR_WRITE,    // Read: EV6 (write phase) timeout
    I2C_TIMEOUT_READ_EV8_ADDR,          // Read: EV8 (address phase) timeout
    I2C_TIMEOUT_READ_EV5_RESTART,       // Read: repeated START EV5 timeout
    I2C_TIMEOUT_READ_EV6_ADDR_READ,     // Read: EV6 (receiver mode) timeout
    I2C_TIMEOUT_READ_EV7_DATA           // Read: EV7 (data received) timeout

} I2C_TIMEOUT_ErrorCode;

static void I2C_BusRecover(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    uint8_t i;

    EEPROM_I2C_GPIO_APBxClock_FUN(EEPROM_I2C_GPIO_CLK, ENABLE);

    GPIO_InitStruct.GPIO_Pin = EEPROM_I2C_SCL_PIN | EEPROM_I2C_SDA_PIN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(EEPROM_I2C_SCL_PORT, &GPIO_InitStruct);

    GPIO_SetBits(EEPROM_I2C_SCL_PORT, EEPROM_I2C_SCL_PIN);
    GPIO_SetBits(EEPROM_I2C_SDA_PORT, EEPROM_I2C_SDA_PIN);

    for (i = 0; i < 9; i++)
    {
        GPIO_ResetBits(EEPROM_I2C_SCL_PORT, EEPROM_I2C_SCL_PIN);
        Delay(1000);
        GPIO_SetBits(EEPROM_I2C_SCL_PORT, EEPROM_I2C_SCL_PIN);
        Delay(1000);
    }

    // fake STOP: SDA low -> SCL high -> SDA high
    GPIO_ResetBits(EEPROM_I2C_SDA_PORT, EEPROM_I2C_SDA_PIN);
    Delay(1000);
    GPIO_SetBits(EEPROM_I2C_SCL_PORT, EEPROM_I2C_SCL_PIN);
    Delay(1000);
    GPIO_SetBits(EEPROM_I2C_SDA_PORT, EEPROM_I2C_SDA_PIN);
    Delay(1000);
}

void I2C_GPIO_Config()
{
	//Structures declaration
	GPIO_InitTypeDef GPIO_InitStruct;
	I2C_InitTypeDef I2C_InitStruct;
	I2C_BusRecover();
	//Clock enable
	EEPROM_I2C_APBxClock_FUN(EEPROM_I2C_CLK, ENABLE);
	EEPROM_I2C_GPIO_APBxClock_FUN (EEPROM_I2C_GPIO_CLK , ENABLE);
	
	//GPIO Init
	GPIO_InitStruct.GPIO_Pin = EEPROM_I2C_SCL_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(EEPROM_I2C_SCL_PORT , &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = EEPROM_I2C_SDA_PIN;
	GPIO_Init(EEPROM_I2C_SDA_PORT, &GPIO_InitStruct);
	
	//I2C Init
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStruct.I2C_ClockSpeed = I2C_Speed;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_OwnAddress1 = I2Cx_OWN_ADDRESS7;
	
	
	I2C_Init(EEPROM_I2Cx , &I2C_InitStruct);
	I2C_Cmd(EEPROM_I2Cx, ENABLE);

	EEPROM_ADDRESS = EEPROM_Block0_ADDRESS;
}

static uint32_t I2C_TIMEOUT_UserCallback(uint8_t errorCode)
{
	EEPROM_ERROR("I2C time out! errorCode = %d",errorCode);
	return 0;
}

/**************ByteWrite Function**************/
uint32_t I2C_EE_ByteWrite(u8* pBuffer, u8 WriteAddr) //u8 -> uint8_t
{
	//Start Signal
	I2C_GenerateSTART(EEPROM_I2Cx, ENABLE);
	I2CTimeout = I2CT_FLAG_TIMEOUT;
	
	while (!I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_MODE_SELECT ))
	{
		if((I2CTimeout--) == 0)
		{
			I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
			I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
			return I2C_TIMEOUT_UserCallback(I2C_TIMEOUT_EV5_START);
		}
			
	}
	
	/*Send EEPROM device addressAddress*/
	I2C_Send7bitAddress(EEPROM_I2Cx, EEPROM_ADDRESS, I2C_Direction_Transmitter);
	I2CTimeout = I2CT_FLAG_TIMEOUT;
	
	while (!I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if((I2CTimeout--) == 0)
		{
			I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
			I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
			return I2C_TIMEOUT_UserCallback(I2C_TIMEOUT_EV6_ADDR_WRITE);
		}
			
	}
	
	//Send adress of EEPROM to be written to EEPROM
	I2C_SendData(EEPROM_I2Cx, WriteAddr);
	I2CTimeout = I2CT_FLAG_TIMEOUT;

		while (!I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED ))
	{
		if((I2CTimeout--) == 0)
		{
			I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
			I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
			return I2C_TIMEOUT_UserCallback(I2C_TIMEOUT_EV8_ADDR);
		}
			
	}
	
	//Send Data
	I2C_SendData(EEPROM_I2Cx, *pBuffer);
	I2CTimeout = I2CT_FLAG_TIMEOUT;

		while (!I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED ))
	{
		if((I2CTimeout--) == 0)
		{
			I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
			I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
			return I2C_TIMEOUT_UserCallback(I2C_TIMEOUT_EV8_DATA);
		}
			
	}
	
	//Send Stop Signal
	I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
	
	return 1;
	
}


/**************PageWrite Function**************/
/*
 * EEPROM page write limitation explanation:
 *
 * The AT24C02 EEPROM organizes its memory into fixed-size pages (8 bytes per page).
 * During a page write operation, only the starting address is sent once, and the
 * internal address pointer automatically increments for each byte written.
 *
 * However, this address increment is limited within the current page. If the number
 * of bytes written exceeds the page boundary, the address will wrap around to the
 * beginning of the same page, causing previously written data in that page to be
 * overwritten.
 *
 * Therefore, the number of bytes written in a single page write operation must not
 * exceed the page size (8 bytes for AT24C02).
 *
 * To write more data, the write operation must be split into multiple page writes,
 * each followed by a wait for the EEPROM internal write cycle to complete.
 */
uint32_t I2C_EE_PageWrite(u8* pBuffer, u8 WriteAddr, uint8_t NumByteToWrite) //u8 -> uint8_t
{
	//Wait until not busy
	I2CTimeout = I2CT_LONG_TIMEOUT;
	while (I2C_GetFlagStatus(EEPROM_I2Cx, I2C_FLAG_BUSY))
	{
		if((I2CTimeout--) == 0)
		{
			I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
			I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
			return I2C_TIMEOUT_UserCallback(I2C_TIMEOUT_BUSY_FLAG);
		}
			
	}
	
	
	//Start Signal
	I2C_GenerateSTART(EEPROM_I2Cx, ENABLE);
	I2CTimeout = I2CT_FLAG_TIMEOUT;
	
	while (!I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_MODE_SELECT ))
	{
		if((I2CTimeout--) == 0)
		{
			I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
			I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
			return I2C_TIMEOUT_UserCallback( I2C_TIMEOUT_PAGE_EV5_START);
		}
			
	}
	
	//Send EEPROM device addressAddress
	I2C_Send7bitAddress(EEPROM_I2Cx, EEPROM_ADDRESS, I2C_Direction_Transmitter);
	I2CTimeout = I2CT_FLAG_TIMEOUT;
	
	while (!I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ))
	{
		if((I2CTimeout--) == 0)
		{
			I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
			I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
			return I2C_TIMEOUT_UserCallback(I2C_TIMEOUT_PAGE_EV6_ADDR);
		}
			
	}
	
	//Send adress of EEPROM to be written to EEPROM
	I2C_SendData(EEPROM_I2Cx, WriteAddr);
	I2CTimeout = I2CT_FLAG_TIMEOUT;

	while (!I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED  ))
	{
		if((I2CTimeout--) == 0)
		{
			I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
			I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
			return I2C_TIMEOUT_UserCallback(I2C_TIMEOUT_PAGE_EV8_ADDR);
		}
			
	}
	
	//Send Data
	while (NumByteToWrite--)
	{
		I2C_SendData(EEPROM_I2Cx, *pBuffer);
		pBuffer++;
		I2CTimeout = I2CT_FLAG_TIMEOUT;

			while (!I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED ))
		{
			if((I2CTimeout--) == 0)
			{
				I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
				I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
				return I2C_TIMEOUT_UserCallback(I2C_TIMEOUT_PAGE_EV8_DATA);
			}
				
		}
		
	}
	

	//Send Stop Signal
	I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
	
	return 1;
	
}

/**************BufferRead Function**************/
/*
 * I2C EEPROM Read Sequence Explanation:
 *
 * 1. First START:
 *    Used to send the EEPROM internal memory address (Write mode).
 *
 * 2. Second START (Repeated START):
 *    Switches the bus direction from write to read without releasing the bus.
 *
 * 3. Before receiving the last byte:
 *    Disable ACK and generate STOP condition to indicate that
 *    this is the final byte to be read.
 */
uint32_t I2C_EE_BufferRead(u8* pBuffer, u8 ReadAddr, u16 NumByteToRead) 
{
	//Wait until not busy
	I2CTimeout = I2CT_LONG_TIMEOUT;
	while (I2C_GetFlagStatus(EEPROM_I2Cx, I2C_FLAG_BUSY))
	{
		if((I2CTimeout--) == 0)
		{
			I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
			I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
			return I2C_TIMEOUT_UserCallback(I2C_TIMEOUT_READ_BUSY);
		}
			
	}
	
	
	//Start Signal
	I2C_GenerateSTART(EEPROM_I2Cx, ENABLE);
	I2CTimeout = I2CT_FLAG_TIMEOUT;
	
	while (!I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_MODE_SELECT ))
	{
		if((I2CTimeout--) == 0)
		{
			I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
			I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
			return I2C_TIMEOUT_UserCallback( I2C_TIMEOUT_READ_EV5_START);
		}
			
	}
	
	//Send EEPROM device addressAddress
	I2C_Send7bitAddress(EEPROM_I2Cx, EEPROM_ADDRESS, I2C_Direction_Transmitter);
	I2CTimeout = I2CT_FLAG_TIMEOUT;
	
	while (!I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ))
	{
		if((I2CTimeout--) == 0)
		{
			I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
			I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
			return I2C_TIMEOUT_UserCallback(I2C_TIMEOUT_READ_EV6_ADDR_WRITE);
		}
			
	}
	
	//Enable one more time to clear EVENT6 Flag
	I2C_Cmd(EEPROM_I2Cx, ENABLE);
	
	//Send adress of EEPROM to be written to EEPROM
	I2C_SendData(EEPROM_I2Cx, ReadAddr);
	I2CTimeout = I2CT_FLAG_TIMEOUT;

	while (!I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED  ))
	{
		if((I2CTimeout--) == 0)
		{
			I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
			I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
			return I2C_TIMEOUT_UserCallback(I2C_TIMEOUT_READ_EV8_ADDR);
		}
			
	}
	
	//2nd Start Signal
	I2C_GenerateSTART(EEPROM_I2Cx, ENABLE);
	I2CTimeout = I2CT_FLAG_TIMEOUT;
	
	while (!I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_MODE_SELECT ))
	{
		if((I2CTimeout--) == 0)
		{
			I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
			I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
			return I2C_TIMEOUT_UserCallback( I2C_TIMEOUT_READ_EV5_RESTART);
		}
			
	}
	
	//Send EEPROM device addressAddress
	I2C_Send7bitAddress(EEPROM_I2Cx, EEPROM_ADDRESS, I2C_Direction_Receiver);
	I2CTimeout = I2CT_FLAG_TIMEOUT;
	
	while (!I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED ))
	{
		if((I2CTimeout--) == 0)
		{
			I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
			I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
			return I2C_TIMEOUT_UserCallback(I2C_TIMEOUT_READ_EV6_ADDR_READ);
		}
			
	}
	
	
	//Read Data
	while (NumByteToRead)
	{
		//Last Data to be Received
		if(NumByteToRead == 1)
		{

			I2C_AcknowledgeConfig(EEPROM_I2Cx, DISABLE);
			I2C_GenerateSTOP(EEPROM_I2Cx,ENABLE);
		}
		
		I2CTimeout = I2CT_FLAG_TIMEOUT;

			while (!I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_BYTE_RECEIVED ))
		{
			if((I2CTimeout--) == 0)
			{
				I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE);
				I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
				return I2C_TIMEOUT_UserCallback(I2C_TIMEOUT_READ_EV7_DATA);
			}
				
		}
		
		*pBuffer = I2C_ReceiveData(EEPROM_I2Cx);
		pBuffer++;
		NumByteToRead--;
	}
	

	//Send Stop Signal
	I2C_AcknowledgeConfig(EEPROM_I2Cx, ENABLE);
	
	return 1;
	
}


/*
 * Wait until EEPROM finishes its internal write cycle.
 *
 * After a write operation, the EEPROM becomes busy for a short time
 * and does not respond (no ACK) to I2C commands.
 *
 * This function repeatedly sends a START condition and the device
 * address in write mode, and checks whether the EEPROM responds
 * with an ACK.
 *
 * Once the EEPROM acknowledges the address, it means the internal
 * write cycle is complete and the device is ready for the next operation.
 *
 * Finally, a STOP condition is generated to release the I2C bus.
 */

void I2C_EE_WaitEepromStandbyState(void)      
{
  vu16 SR1_Tmp = 0;

  do
  {
    /* Send START condition */
    I2C_GenerateSTART(EEPROM_I2Cx, ENABLE);
    /* Read I2C1 SR1 register */
    SR1_Tmp = I2C_ReadRegister(EEPROM_I2Cx, I2C_Register_SR1);
    /* Send EEPROM address for write */
    I2C_Send7bitAddress(EEPROM_I2Cx, EEPROM_ADDRESS, I2C_Direction_Transmitter);
  }while(!(I2C_ReadRegister(EEPROM_I2Cx, I2C_Register_SR1) & 0x0002));
  
  /* Clear AF flag */
  I2C_ClearFlag(EEPROM_I2Cx, I2C_FLAG_AF);
    /* STOP condition */    
    I2C_GenerateSTOP(EEPROM_I2Cx, ENABLE); 
}


/**************BufferWrite Function**************/
void I2C_EE_BufferWrite(u8* pBuffer, u8 WriteAddr, u16 NumByteToWrite)
{
  u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0;

  Addr = WriteAddr % I2C_PageSize;
  count = I2C_PageSize - Addr;
  NumOfPage =  NumByteToWrite / I2C_PageSize;
  NumOfSingle = NumByteToWrite % I2C_PageSize;
 
  /* If WriteAddr is I2C_PageSize aligned  */
  if(Addr == 0) 
  {
    /* If NumByteToWrite < I2C_PageSize */
    if(NumOfPage == 0) 
    {
      I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle);
      I2C_EE_WaitEepromStandbyState();
    }
    /* If NumByteToWrite > I2C_PageSize */
    else  
    {
      while(NumOfPage--)
      {
        I2C_EE_PageWrite(pBuffer, WriteAddr, I2C_PageSize); 
    	I2C_EE_WaitEepromStandbyState();
        WriteAddr +=  I2C_PageSize;
        pBuffer += I2C_PageSize;
      }

      if(NumOfSingle!=0)
      {
        I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle);
        I2C_EE_WaitEepromStandbyState();
      }
    }
  }
  /* If WriteAddr is not I2C_PageSize aligned  */
  else 
  {
    /* If NumByteToWrite < I2C_PageSize */
    if(NumOfPage== 0) 
    {
      I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle);
      I2C_EE_WaitEepromStandbyState();
    }
    /* If NumByteToWrite > I2C_PageSize */
    else
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / I2C_PageSize;
      NumOfSingle = NumByteToWrite % I2C_PageSize;	
      
      if(count != 0)
      {  
        I2C_EE_PageWrite(pBuffer, WriteAddr, count);
        I2C_EE_WaitEepromStandbyState();
        WriteAddr += count;
        pBuffer += count;
      } 
      
      while(NumOfPage--)
      {
        I2C_EE_PageWrite(pBuffer, WriteAddr, I2C_PageSize);
        I2C_EE_WaitEepromStandbyState();
        WriteAddr +=  I2C_PageSize;
        pBuffer += I2C_PageSize;  
      }
      if(NumOfSingle != 0)
      {
        I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle); 
        I2C_EE_WaitEepromStandbyState();
      }
    }
  }  
}



