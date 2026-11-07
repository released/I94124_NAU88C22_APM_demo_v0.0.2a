/**************************************************************************//**
 * @file     pdma_config.h
 * @version  V1.00
 * @brief    
 *
 * @copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __PDMA_CONFIG_H__
#define __PDMA_CONFIG_H__

#include "stdint.h" 


typedef struct dma_desc_t 
{
    uint32_t ctl;
    uint32_t endsrc;
    uint32_t enddest;
    uint32_t offset;
} DMA_DESC_T;

#endif //__PDMA_CONFIG_H__

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/

