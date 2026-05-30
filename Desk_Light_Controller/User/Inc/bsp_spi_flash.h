#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include "stm32f10x.h"
#include <stdio.h>

#define  sFLASH_ID              0XEF4017    //W25Q64

#define SPI_FLASH_PageSize              256
#define SPI_FLASH_PerWritePageSize      256

//Flash Commands
#define W25X_WriteEnable		      0x06 
#define W25X_WriteDisable		      0x04 
#define W25X_ReadStatusReg		    0x05 
#define W25X_WriteStatusReg		    0x01 
#define W25X_ReadData			        0x03 
#define W25X_FastReadData		      0x0B 
#define W25X_FastReadDual		      0x3B 
#define W25X_PageProgram		      0x02 
#define W25X_BlockErase			      0xD8 
#define W25X_SectorErase		      0x20 
#define W25X_ChipErase			      0xC7 
#define W25X_PowerDown			      0xB9 
#define W25X_ReleasePowerDown	    0xAB 
#define W25X_DeviceID			        0xAB 
#define W25X_ManufactDeviceID   	0x90 
#define W25X_JedecDeviceID		    0x9F

/********************SPI********************/
#define      FLASH_SPIx                        SPI1
#define      FLASH_SPI_APBxClock_FUN          RCC_APB2PeriphClockCmd
#define      FLASH_SPI_CLK                     RCC_APB2Periph_SPI1

//CS
#define      FLASH_SPI_CS_APBxClock_FUN       RCC_APB2PeriphClockCmd
#define      FLASH_SPI_CS_CLK                  RCC_APB2Periph_GPIOC    
#define      FLASH_SPI_CS_PORT                 GPIOC
#define      FLASH_SPI_CS_PIN                  GPIO_Pin_0

//SCK
#define      FLASH_SPI_SCK_APBxClock_FUN      RCC_APB2PeriphClockCmd
#define      FLASH_SPI_SCK_CLK                 RCC_APB2Periph_GPIOA   
#define      FLASH_SPI_SCK_PORT                GPIOA   
#define      FLASH_SPI_SCK_PIN                 GPIO_Pin_5
//MISO
#define      FLASH_SPI_MISO_APBxClock_FUN     RCC_APB2PeriphClockCmd
#define      FLASH_SPI_MISO_CLK                RCC_APB2Periph_GPIOA    
#define      FLASH_SPI_MISO_PORT               GPIOA 
#define      FLASH_SPI_MISO_PIN                GPIO_Pin_6
//MOSI
#define      FLASH_SPI_MOSI_APBxClock_FUN     RCC_APB2PeriphClockCmd
#define      FLASH_SPI_MOSI_CLK                RCC_APB2Periph_GPIOA    
#define      FLASH_SPI_MOSI_PORT               GPIOA 
#define      FLASH_SPI_MOSI_PIN                GPIO_Pin_7

#define  		SPI_FLASH_CS_LOW()     						GPIO_ResetBits( FLASH_SPI_CS_PORT, FLASH_SPI_CS_PIN )
#define  		SPI_FLASH_CS_HIGH()    						GPIO_SetBits( FLASH_SPI_CS_PORT, FLASH_SPI_CS_PIN )
/********************SPI********************/

#define SPIT_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define SPIT_LONG_TIMEOUT         ((uint32_t)(10 * SPIT_FLAG_TIMEOUT))

/********************Debug********************/
#define FLASH_DEBUG_ON         1

#define FLASH_INFO(fmt,arg...)           printf("<<-FLASH-INFO->> "fmt"\n",##arg)
#define FLASH_ERROR(fmt,arg...)          printf("<<-FLASH-ERROR->> "fmt"\n",##arg)
#define FLASH_DEBUG(fmt,arg...)          do{\
                                          if(FLASH_DEBUG_ON)\
                                          printf("<<-FLASH-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg);\
                                          }while(0)

/********************Functions********************/
void SPI_FLASH_Init(void);										  
u32 SPI_Flash_ReadID(void);
u32 SPI_Flash_ReadDeviceID(void);
void SPI_Flash_SectorErase(u32 SectorAddress);
void SPI_Flash_BufferWrite(u8 *pBuffer, u32 WriteAddress, u16 NumByteToWrite);
void SPI_Flash_BufferRead(u8 *pBuffer, u32 ReadAddress, u16 NumByteToRead);	
void SPI_Flash_WakeUp(void);										  
#endif

										  