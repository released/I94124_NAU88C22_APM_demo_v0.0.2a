#ifndef __CODEC_H__
#define __CODEC_H__

#include "Platform.h"




void Codec_I2C_Handler(I2C_T* i2c);

void NAU88C22_Init(I2C_T* i2c);
void Set_NAU88C22_Reg(I2C_T* i2c, uint8_t reg, uint16_t val);
void NAU88C22_Read(I2C_T* i2c, uint8_t* tmp, uint16_t reg);

void NAU88L21_MicIn_Init(I2C_T* i2c);

#endif
