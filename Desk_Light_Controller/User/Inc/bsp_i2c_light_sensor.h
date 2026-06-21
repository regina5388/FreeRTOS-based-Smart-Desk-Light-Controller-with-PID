#ifndef BSP_I2C_LIGHT_SENSOR
#define BSP_I2C_LIGHT_SENSOR


#include "stm32f10x.h"
#include "bsp_i2c.h"

#define LIGHT_SENSOR_I2Cx                                I2C1


#include <stdint.h>

/* LIGHT_SENSOR I2C address
 * ADDR = LOW  -> 0x23
 * ADDR = HIGH -> 0x5C
 *
 * STM32 Standard Peripheral Library usually uses left-shifted address.
 */
#define LIGHT_SENSOR_ADDR_LOW_7BIT          0x23
#define LIGHT_SENSOR_ADDR_HIGH_7BIT         0x5C

#define LIGHT_SENSOR_ADDR_LOW               (LIGHT_SENSOR_ADDR_LOW_7BIT << 1)    /* 0x46 */
#define LIGHT_SENSOR_ADDR_HIGH              (LIGHT_SENSOR_ADDR_HIGH_7BIT << 1)   /* 0xB8 */

/* Choose your module address here */
#define LIGHT_SENSOR_I2C_ADDR               LIGHT_SENSOR_ADDR_LOW


/* LIGHT_SENSOR commands */
#define LIGHT_SENSOR_CMD_POWER_DOWN         0x00    /* No active state */
#define LIGHT_SENSOR_CMD_POWER_ON           0x01    /* Waiting for measurement command */
#define LIGHT_SENSOR_CMD_RESET              0x07    /* Reset data register, only valid after Power On */

/* Continuous measurement modes */
#define LIGHT_SENSOR_CMD_CONT_H_RES_MODE    0x10    /* 1 lx resolution, typ. 120 ms */
#define LIGHT_SENSOR_CMD_CONT_H_RES_MODE2   0x11    /* 0.5 lx resolution, typ. 120 ms */
#define LIGHT_SENSOR_CMD_CONT_L_RES_MODE    0x13    /* 4 lx resolution, typ. 16 ms */

/* One-time measurement modes */
#define LIGHT_SENSOR_CMD_ONE_H_RES_MODE     0x20    /* 1 lx resolution, typ. 120 ms, then Power Down */
#define LIGHT_SENSOR_CMD_ONE_H_RES_MODE2    0x21    /* 0.5 lx resolution, typ. 120 ms, then Power Down */
#define LIGHT_SENSOR_CMD_ONE_L_RES_MODE     0x23    /* 4 lx resolution, typ. 16 ms, then Power Down */

/* Measurement timing */
#define LIGHT_SENSOR_H_RES_TYP_TIME_MS     120
#define LIGHT_SENSOR_H_RES_MAX_TIME_MS     180
#define LIGHT_SENSOR_L_RES_TYP_TIME_MS     16
#define LIGHT_SENSOR_L_RES_MAX_TIME_MS     24

/* Lux conversion */
#define LIGHT_SENSOR_LUX_DIVIDER            1.2f


void LIGHT_SENSOR_Init(void);
uint32_t LIGHT_SENSOR_WriteCmd(uint8_t cmd);
uint32_t LIGHT_SENSOR_ReadRaw(uint16_t *raw);
uint32_t LIGHT_SENSOR_ReadLux(float *lux);
void LIGHT_SENSOR_PowerDown(void);
void LIGHT_SENSOR_PowerOn(void);

#endif