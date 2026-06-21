#include "bsp_i2c_light_sensor.h"



uint32_t LIGHT_SENSOR_WriteCmd(uint8_t cmd)
{
	return BSP_I2C_WriteBytes(LIGHT_SENSOR_I2Cx, LIGHT_SENSOR_I2C_ADDR, &cmd, 1);
}

void LIGHT_SENSOR_PowerOn(void)
{
    LIGHT_SENSOR_WriteCmd(LIGHT_SENSOR_CMD_POWER_ON);
}

void LIGHT_SENSOR_PowerDown(void)
{
    LIGHT_SENSOR_WriteCmd(LIGHT_SENSOR_CMD_POWER_DOWN);
}

void LIGHT_SENSOR_Init(void)
{
	LIGHT_SENSOR_PowerOn();
	
	/*
     * RESET is only valid after Power On.
     */
    LIGHT_SENSOR_WriteCmd(LIGHT_SENSOR_CMD_RESET);

    /*
     * For smart lamp, continuous H-resolution mode is enough.
     */
    LIGHT_SENSOR_WriteCmd(LIGHT_SENSOR_CMD_CONT_H_RES_MODE);
	
}

uint32_t LIGHT_SENSOR_ReadRaw(uint16_t *raw)
{
    uint8_t buf[2];


    if (BSP_I2C_ReadBytes(LIGHT_SENSOR_I2Cx, LIGHT_SENSOR_I2C_ADDR, buf, 2) != 1)
    {
		I2C_DEBUG("Read sensor error");
		return 0;
    }

    *raw = ((uint16_t)buf[0] << 8) | buf[1];

    return 1;
}

uint32_t LIGHT_SENSOR_ReadLux(float *lux)
{
	uint16_t raw = 0;
	if(LIGHT_SENSOR_ReadRaw(&raw) != 1)
	{
		I2C_DEBUG("Read lux error");
		return 0;
	}
	*lux =  raw / 1.2f;
	return 1;
}