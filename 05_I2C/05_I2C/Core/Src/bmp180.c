/*
 * bmp180.c
 *
 *  Created on: Sep 12, 2026
 *      Author: emagi
 */

#include "main.h"
#include "bmp180.h"

uint8_t Read8(bmp180_t *bmp180, uint8_t Register)
{
	uint8_t Value;
	HAL_I2C_Mem_Read(bmp180->bmp180_i2c, ((bmp180->address)<<1), Register, 1, &Value, 1, I2C_TIMEOUT);

	return Value;
}

uint16_t Read16(bmp180_t *bmp180, uint8_t Register)
{
	uint8_t Value[2];
	HAL_I2C_Mem_Read(bmp180->bmp180_i2c, ((bmp180->address)<<1), Register, 1, Value, 2, I2C_TIMEOUT);

	return ((Value[0] << 8) | Value[1]);
}

void Write8(bmp180_t *bmp180, uint8_t Register, uint8_t Value)
{
	HAL_I2C_Mem_Write(bmp180->bmp180_i2c, ((bmp180->address)<<1), Register, 1, &Value, 1, I2C_TIMEOUT);
}


void BMP180_SetOversampling(bmp180_t *bmp180, uint8_t OverS)
{

	uint8_t Tmp;
	if(OverS>3)
	{
		OverS = 3;
	}
	Tmp = Read8(bmp180, BMP180_CTRL_MEAS_REG);

	Tmp = (Tmp & 0x3F) | (OverS << 6);
	Write8(bmp180, BMP180_CTRL_MEAS_REG, Tmp);
}


void BMP180_ReadTemperature(bmp180_t *bmp180)
{
	uint8_t lsb, msb;
	Write8(bmp180, 0xF4, 0x2E);
	HAL_Delay(5);

	lsb = Read8(bmp180, 0xF7);
	msb = Read8(bmp180, 0xF6);

	uint32_t UT = ((uint32_t)msb<<8) | lsb;
	bmp180->UT_result_raw = UT;
}

void BMP180_ReadPressure(bmp180_t *bmp180)
{
	uint8_t xlsb, lsb, msb;
	Write8(bmp180, 0xF4, 0x34|(BMP180_ULTRA_LOW_POWER<<6));
	HAL_Delay(5);

	xlsb = Read8(bmp180, BMP180_OUT_XLSB_REG);
	lsb = Read8(bmp180, BMP180_OUT_LSB_REG);
	msb = Read8(bmp180, BMP180_OUT_MSB_REG);
	uint32_t UP = ((uint32_t)msb<<16 | (uint32_t)lsb <<8 | (uint32_t)xlsb) >> (8 - BMP180_ULTRA_LOW_POWER);
	bmp180->UP_result_raw = UP;
}

void BMP180_CalculateTemperature(bmp180_t *bmp180)
{
	bmp180->X1 = (bmp180->UT_result_raw - bmp180->ac6) * bmp180->ac5 / (1<<15);
	bmp180->X2 = (bmp180->mc * (1<<11)) / (bmp180->X1 + bmp180->md);
	bmp180->B5 = bmp180->X1 + bmp180->X2;
	bmp180->UT_result_real = (bmp180->B5 + 8)/(1<<4);
}

void BMP180_CalculatePressure(bmp180_t *bmp180)
{
	bmp180->B6 = bmp180->B5 - 4000;

	bmp180->X1 = (bmp180->b2 * (bmp180->B6 * bmp180->B6 / (1<<12))) / (1<<11);
	bmp180->X2 = bmp180->ac2 * bmp180->B6 / (1<<11);
	bmp180->X3 = bmp180->X1 + bmp180->X2;

	bmp180->B3 = (((bmp180->ac1 * 4 + bmp180->X3) << BMP180_ULTRA_LOW_POWER) + 2)/4;

	bmp180->X1 = bmp180->ac3 * bmp180->B6 / (1<<13);
	bmp180->X2 = (bmp180->b1 * (bmp180->B6 * bmp180->B6 / (1<<12))) / (1<<15);
	bmp180->X3 = ((bmp180->X1 + bmp180->X2) + 2) / (1<<2);

	bmp180->B4 = bmp180->ac4 * (uint32_t)(bmp180->X3 + 32768) / (1<<15);
	bmp180->B7 = ((uint32_t)bmp180->UP_result_raw - bmp180->B3) * (50000 >> BMP180_ULTRA_LOW_POWER);

	if(bmp180->B7 < 0x80000000)
	{
		bmp180->UP_result_real = (bmp180->B7 * 2)/bmp180->B4;
	}

	else
	{
		bmp180->UP_result_real = (bmp180->B7 / bmp180->B4) * 2;
	}

	bmp180->X1 = (bmp180->UP_result_real /(1<<8)) * (bmp180->UP_result_real/(1<<8));
	bmp180->X1 = (bmp180->X1 * 3038) / (1<<15);
	bmp180->X2 = (-7357 * bmp180->UP_result_real)/(1<<15);

	bmp180->UP_result_real = bmp180->UP_result_real + (bmp180->X1 + bmp180->X2 + 3791) / (1<<4);

}


uint8_t BMP180_Init(bmp180_t *bmp180, I2C_HandleTypeDef *i2c, uint8_t address)
{

	uint8_t ChipID;

	bmp180->bmp180_i2c = i2c;
	bmp180->address = address;

	ChipID = Read8(bmp180, BMP180_ID_REG);

	if(ChipID != 0x55)
	{
		return 1;
	}

	bmp180->ac1 = Read16(bmp180, BMP180_AC1_REG);
	bmp180->ac2 = Read16(bmp180, BMP180_AC2_REG);
	bmp180->ac3 = Read16(bmp180, BMP180_AC3_REG);
	bmp180->ac4 = Read16(bmp180, BMP180_AC4_REG);
	bmp180->ac5 = Read16(bmp180, BMP180_AC5_REG);
	bmp180->ac6 = Read16(bmp180, BMP180_AC6_REG);
	bmp180->b1 = Read16(bmp180, BMP180_B1_REG);
	bmp180->b2 = Read16(bmp180, BMP180_B2_REG);
	bmp180->mb = Read16(bmp180, BMP180_MB_REG);
	bmp180->mc = Read16(bmp180, BMP810_MC_REG);
	bmp180->md = Read16(bmp180, BMP180_MD_REG);

	BMP180_SetOversampling(bmp180, BMP180_STANDARD);

	return 0;
}




