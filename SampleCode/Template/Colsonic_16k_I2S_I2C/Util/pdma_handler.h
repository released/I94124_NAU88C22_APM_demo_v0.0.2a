#ifndef __PDMA_HANDLER_H__
#define __PDMA_HANDLER_H__

#include "Platform.h"
#include "data_handler.h"

void pdma_i2s_tx_handler(DATA_MANAGER* psDataManager);
//void pdma_dpwm_handler(DATA_MANAGER* psDataManager);
void pdma_spi_i2s_tx_handler(DATA_MANAGER* psDataManager);
void pdma_i2s_rx_handler(DATA_MANAGER* psDataManager);
void pdma_spi_i2s_rx_handler(DATA_MANAGER* psDataManager);
void pdma_dmic_handler(DATA_MANAGER* psDataManager);

#endif
