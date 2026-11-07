#ifndef __PDMA_INIT_H__
#define __PDMA_INIT_H__

#include <Platform.h>

void PDMA_I2S0_TX_Init(uint8_t u8Ch);
void PDMA_I2S0_RX_Init(uint8_t u8Ch);
void PDMA_SPI1_I2S_TX_Init(uint8_t u8Ch);
void PDMA_SPI1_I2S_RX_Init(uint8_t u8Ch);
void PDMA_SPI2_I2S_TX_Init(uint8_t u8Ch);
void PDMA_SPI2_I2S_RX_Init(uint8_t u8Ch);
void PDMA_DPWM_Init(uint8_t u8Ch);
void PDMA_DMIC_Init(uint8_t u8Ch);

#endif
