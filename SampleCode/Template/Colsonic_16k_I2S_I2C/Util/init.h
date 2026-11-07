#ifndef __INIT_H__
#define __INIT_H__

#include "Platform.h"

uint32_t SysClk_Initiate(void);
void I2C0_Slave_Init(void);
void USBD_Init(void);
void PDMA_Init(void);
void DPWM_Init(uint32_t u32SampleRate, uint32_t u32Bit);
void DMIC_Init(uint32_t u32SampleRate);
void TMR0_Init(void);
void TMR3_Init(void);
void UART0_Init(void);
void CODEC_I2C1_Init(void);
void CODEC_I2S_Init_85L40_2CH(uint32_t u32SampleRate, uint32_t u32Bit, uint32_t u32mclk);
void CODEC_I2S_Init_8822(uint32_t u32SampleRate, uint32_t u32Bit, uint32_t u32mclk);
void SPI1_I2S_Init(uint32_t u32SampleRate);
void SPI2_I2S_Init(uint32_t u32SampleRate);
#endif
