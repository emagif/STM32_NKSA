/*
 * bmp180.h
 *
 *  Created on: Sep 12, 2026
 *      Author: emagi
 */

#ifndef SRC_BMP180_H_
#define SRC_BMP180_H_


#define I2C_TIMEOUT 1000
#define BMP180_ADDRESS 0x77

// Calibration coefficients
#define BMP180_AC1_REG                     0xAA
#define BMP180_AC2_REG                     0xAC
#define BMP180_AC3_REG                     0xAE
#define BMP180_AC4_REG                     0xB0
#define BMP180_AC5_REG                     0xB2
#define BMP180_AC6_REG                     0xB4
#define BMP180_B1_REG                      0xB6
#define BMP180_B2_REG			           0xB8
#define BMP180_MB_REG                      0xBA
#define BMP810_MC_REG                      0xBC
#define BMP180_MD_REG                      0xBE

// Registers
#define BMP180_OUT_XLSB_REG                0xF8
#define BMP180_OUT_LSB_REG		           0xF7
#define BMP180_OUT_MSB_REG		           0xF6
#define BMP180_CTRL_MEAS_REG               0xF4
#define BMP180_SOFT_RESET_REG	           0xE0
#define BMP180_ID_REG			           0xD0

#define BMP180_ULTRA_LOW_POWER             0
#define BMP180_STANDARD 		           1
#define BMP180_HIGH_RESOLUTION             2
#define BMP180_ULTRA_HIGH_RESOLUTION       3


typedef struct {
	I2C_HandleTypeDef   *bmp180_i2c;
	uint8_t 			address;

	int16_t ac1, ac2, ac3;
	uint16_t ac4, ac5, ac6;
	int16_t b1, b2, mb, mc, md;
	int32_t UP_result_raw, UT_result_raw;
	int32_t UP_result_real, UT_result_real;
	int32_t X1, X2, X3, B3, B5, B6;
	uint32_t B4, B7;
}bmp180_t;

uint8_t BMP180_Init(bmp180_t *bmp180, I2C_HandleTypeDef *i2c, uint8_t address);
void BMP180_SetOversampling(bmp180_t *bmp180, uint8_t OverS);
void BMP180_ReadPressure(bmp180_t *bmp180);
void BMP180_ReadTemperature(bmp180_t *bmp180);
void BMP180_CalculateTemperature(bmp180_t *bmp180);
void BMP180_CalculatePressure(bmp180_t *bmp180);


#endif /* SRC_BMP180_H_ */
