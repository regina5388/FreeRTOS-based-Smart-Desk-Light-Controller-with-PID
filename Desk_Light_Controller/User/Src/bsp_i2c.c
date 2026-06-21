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

static void I2C_Delay(void)
{
    volatile uint32_t i;

    for (i = 0; i < 100; i++)
    {
        __NOP();
    }
}

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
        I2C_Delay();
        GPIO_SetBits(EEPROM_I2C_SCL_PORT, EEPROM_I2C_SCL_PIN);
        I2C_Delay();
    }

    // fake STOP: SDA low -> SCL high -> SDA high
    GPIO_ResetBits(EEPROM_I2C_SDA_PORT, EEPROM_I2C_SDA_PIN);
    I2C_Delay();
    GPIO_SetBits(EEPROM_I2C_SCL_PORT, EEPROM_I2C_SCL_PIN);
    I2C_Delay();
    GPIO_SetBits(EEPROM_I2C_SDA_PORT, EEPROM_I2C_SDA_PIN);
    I2C_Delay();
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
	I2C_InitStruct.I2C_OwnAddress1 = I2Cx_OWN_ADDRESS;
	
	
	I2C_Init(EEPROM_I2Cx , &I2C_InitStruct);
	I2C_Cmd(EEPROM_I2Cx, ENABLE);

	EEPROM_ADDRESS = EEPROM_Block0_ADDRESS;
}


uint32_t BSP_I2C_WriteBytes(I2C_TypeDef *I2Cx, uint8_t dev_addr, const uint8_t *data, uint16_t len)
{
    uint16_t i;
    I2CTimeout = I2CT_FLAG_TIMEOUT;
	
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
    {
        if (I2CTimeout-- == 0)
		{
			I2C_DEBUG("I2C Busy time out");
            return 0;
		}
			
    }

    I2C_GenerateSTART(I2Cx, ENABLE);
	I2CTimeout = I2CT_FLAG_TIMEOUT;
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if (I2CTimeout-- == 0)
        {
            I2C_GenerateSTOP(I2Cx, ENABLE);
			I2C_DEBUG("Start time out");
            return 0;
        }
    }

    I2C_Send7bitAddress(I2Cx, dev_addr, I2C_Direction_Transmitter);
	I2CTimeout = I2CT_FLAG_TIMEOUT;
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if (I2CTimeout-- == 0)
        {
            I2C_GenerateSTOP(I2Cx, ENABLE);
			I2C_DEBUG("Address sending time out");
            return 0;
        }
    }

    for (i = 0; i < len; i++)
    {
        I2C_SendData(I2Cx, data[i]);
		I2CTimeout = I2CT_FLAG_TIMEOUT;
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        {
            if (I2CTimeout-- == 0)
            {
                I2C_GenerateSTOP(I2Cx, ENABLE);
				I2C_DEBUG("Data send time out");
                return 0;
            }
        }
    }

    I2C_GenerateSTOP(I2Cx, ENABLE);

    return 1;
}

uint32_t BSP_I2C_ReadBytes(I2C_TypeDef *I2Cx, uint8_t dev_addr, uint8_t *data, uint16_t len)
{
    if ((data == 0) || (len != 2))
    {
        I2C_DEBUG("Wrong data length");
        return 0;
    }

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
    {
        if (I2CTimeout-- == 0)
        {
            I2C_DEBUG("I2C busy");
            return 0;
        }
    }

    I2C_AcknowledgeConfig(I2Cx, ENABLE);

    /* START */
    I2C_GenerateSTART(I2Cx, ENABLE);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if (I2CTimeout-- == 0)
        {
            I2C_GenerateSTOP(I2Cx, ENABLE);
            I2C_DEBUG("Start Time out");
            return 0;
        }
    }

    /* Send slave address + read */
    I2C_Send7bitAddress(I2Cx, dev_addr, I2C_Direction_Receiver);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        if (I2CTimeout-- == 0)
        {
            I2C_GenerateSTOP(I2Cx, ENABLE);
            I2C_AcknowledgeConfig(I2Cx, ENABLE);
            I2C_DEBUG("Address sending time out");
            return 0;
        }
    }

    /*
     * Read first byte.
     * After first byte is received, disable ACK before receiving last byte.
     */
    I2CTimeout = I2CT_FLAG_TIMEOUT;
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
    {
        if (I2CTimeout-- == 0)
        {
            I2C_GenerateSTOP(I2Cx, ENABLE);
            I2C_AcknowledgeConfig(I2Cx, ENABLE);
            I2C_DEBUG("Byte not received");
            return 0;
        }
    }

    data[0] = I2C_ReceiveData(I2Cx);

    /*
     * Last byte: master sends NACK.
     */
    I2C_AcknowledgeConfig(I2Cx, DISABLE);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
    {
        if (I2CTimeout-- == 0)
        {
            I2C_GenerateSTOP(I2Cx, ENABLE);
            I2C_AcknowledgeConfig(I2Cx, ENABLE);
            I2C_DEBUG("Byte not received");
            return 0;
        }
    }

    data[1] = I2C_ReceiveData(I2Cx);

    /* STOP */
    I2C_GenerateSTOP(I2Cx, ENABLE);

    /* Restore ACK for next communication */
    I2C_AcknowledgeConfig(I2Cx, ENABLE);

    return 1;
}








