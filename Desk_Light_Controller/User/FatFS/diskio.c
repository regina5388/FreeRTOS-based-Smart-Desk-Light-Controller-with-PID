/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Basic definitions of FatFs */
#include "diskio.h"		/* Declarations FatFs MAI */
#include "../USER/Inc/bsp_spi_flash.h"

/* Example: Declarations of the platform and disk functions in the project */
//#include "platform.h"
//#include "storage.h"

/* Example: Mapping of physical drive number for each drive */
#define ATA			0	/* For SD Card*/
#define SPI_FLASH	1	/* External SPI chip */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;

	switch (pdrv) {
	case ATA :

		// translate the reslut code here

		return stat;

	case SPI_FLASH :
		if(SPI_Flash_ReadID() == sFLASH_ID){
			stat &= ~STA_NOINIT; //stat = 0;
		}else{
			stat = STA_NOINIT;
		}

		return stat;

	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;
	int i;
	
	switch (pdrv) {
	case ATA :

		// translate the reslut code here

		return stat;

	case SPI_FLASH :

		SPI_FLASH_Init();
		i = 500;
		while(i--);// Short delay
		
		SPI_Flash_WakeUp();
		stat = disk_status(SPI_FLASH);
		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res = RES_PARERR;

	switch (pdrv) {
	case ATA :
		
		// translate the arguments here
		
		return res;

	case SPI_FLASH :
		
		sector += 512; //Avoid writing into the first 6MB room (reserved for other stuff)
		SPI_Flash_BufferRead(buff,sector<<12, count<<12); //Sector Size is 4096
		res = RES_OK;
		
		return res;

	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;

	switch (pdrv) {
	case ATA :
		// translate the arguments here

		return res;

	case SPI_FLASH :
		//important! Erase before writing
		sector += 512;
		SPI_Flash_SectorErase(sector <<12);
		SPI_Flash_BufferWrite((u8 *)buff, sector <<12, count << 12);
		res = RES_OK;
	
		return res;

	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_PARERR;

	switch (pdrv) {
	case ATA :

		// Process of the command for the RAM drive

		return res;

	case SPI_FLASH :
		switch(cmd){
			
			case GET_SECTOR_COUNT:
				* (DWORD * )buff = 1536;
				res = RES_OK;
				return res;
			
			case GET_SECTOR_SIZE:
				* (DWORD * )buff = 4096;
				res = RES_OK;
				return res;
			
			case GET_BLOCK_SIZE:		
				* (DWORD * )buff = 1;
				res = RES_OK;
				return res;
		}

		return res;

	}

	return RES_PARERR;
}

