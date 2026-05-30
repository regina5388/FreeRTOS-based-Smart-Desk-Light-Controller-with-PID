#ifndef BSP_I2C_H
#define BSP_I2C_H

#include "stm32f10x.h"

#define EEPROM_I2Cx                                I2C1
#define EEPROM_I2C_APBxClock_FUN                   RCC_APB1PeriphClockCmd
#define EEPROM_I2C_CLK                             RCC_APB1Periph_I2C1
#define EEPROM_I2C_GPIO_APBxClock_FUN              RCC_APB2PeriphClockCmd
#define EEPROM_I2C_GPIO_CLK                        RCC_APB2Periph_GPIOB     
#define EEPROM_I2C_SCL_PORT                        GPIOB   
#define EEPROM_I2C_SCL_PIN                         GPIO_Pin_6
#define EEPROM_I2C_SDA_PORT                        GPIOB 
#define EEPROM_I2C_SDA_PIN                         GPIO_Pin_7


/* STM32 I2C Fast Mode (400kHz) */
#define I2C_Speed              400000  

/*EEPROM Block to use*/
#define EEPROM_Block0_ADDRESS 0xA0 

/* 
 * This address is the STM32's own I2C address.
 * It must be different from any external I2C device address.
 */
#define I2Cx_OWN_ADDRESS7      0X0A   

/* AT24C01/02: 8 bytes per page */
#define I2C_PageSize           8

/* Turn debug print information on or off */
#define EEPROM_DEBUG_ON         0

/* Time out time */
#define I2CT_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define I2CT_LONG_TIMEOUT         ((uint32_t)(10 * I2CT_FLAG_TIMEOUT))

/* 
 * EEPROM_INFO
 * ------------------------------------------------------------
 * Print informational messages related to EEPROM operations.
 * - Always enabled (no switch control)
 * - Automatically adds a prefix and newline
 * - Supports printf-style formatting
 */
#define EEPROM_INFO(fmt,arg...) \
    printf("<<-EEPROM-INFO->> " fmt "\n", ##arg)


/* 
 * EEPROM_ERROR
 * ------------------------------------------------------------
 * Print error messages for EEPROM-related failures.
 * - Always enabled (used for critical issues)
 * - Adds a clear error prefix for easy identification
 * - Supports printf-style formatting
 */
#define EEPROM_ERROR(fmt,arg...) \
    printf("<<-EEPROM-ERROR->> " fmt "\n", ##arg)


/* 
 * EEPROM_DEBUG
 * ------------------------------------------------------------
 * Print debug messages for EEPROM operations.
 * - Controlled by EEPROM_DEBUG_ON (0 = disable, 1 = enable)
 * - Includes current source code line number (__LINE__)
 * - Uses do{...}while(0) to ensure safe macro usage in all contexts
 * - Supports printf-style formatting
 */
#define EEPROM_DEBUG(fmt,arg...) \
    do{ \
        if(EEPROM_DEBUG_ON) \
            printf("<<-EEPROM-DEBUG->> [%d]" fmt "\n", __LINE__, ##arg); \
    }while(0)
		
		
uint32_t I2C_EE_BufferRead(u8* pBuffer, u8 ReadAddr, u16 NumByteToRead); //u8 -> uint8_t
void I2C_EE_BufferWrite(u8* pBuffer, u8 WriteAddr, u16 NumByteToWrite);
uint32_t I2C_EE_PageWrite(u8* pBuffer, u8 WriteAddr, u8 NumByteToWrite); //u8 -> uint8_t
uint32_t I2C_EE_ByteWrite(u8* pBuffer, u8 WriteAddr); //u8 -> uint8_t
void I2C_GPIO_Config(void);


#endif
