#include "pdma_init.h"
#include "pdma_config.h"
#include "buf_def.h"
#include "data_handler.h"

extern DATA_MANAGER g_sDataManager;

/*
 * Scatter-gather descriptors link row 0 -> row 1 -> row 0 continuously.
 * Each active 16-bit stereo transfer uses 32-bit PDMA words (32 per row),
 * not 32 bytes. TX: RAM -> FIFO; RX: FIFO -> RAM. The ISR services the
 * completed row while hardware advances to the other row. Keep the fixed
 * descriptor addresses consistent with the linker/memory configuration.
 */
DMA_DESC_T DMA_I2S0TXDESC[2] __attribute__((section(".ARM.__at_0x20000100")));
DMA_DESC_T DMA_I2S0RXDESC[2] __attribute__((section(".ARM.__at_0x20000120")));
DMA_DESC_T DMA_SPI1I2STXDESC[2] __attribute__((section(".ARM.__at_0x20000140")));
DMA_DESC_T DMA_SPI1I2SRXDESC[2] __attribute__((section(".ARM.__at_0x20000160")));
DMA_DESC_T DMA_SPI2I2STXDESC[2] __attribute__((section(".ARM.__at_0x20000180")));
DMA_DESC_T DMA_SPI2I2SRXDESC[2] __attribute__((section(".ARM.__at_0x200001A0")));
DMA_DESC_T DMA_DMICDESC[2] __attribute__((section(".ARM.__at_0x200001C0")));
DMA_DESC_T DMA_DPWMDESC[2] __attribute__((section(".ARM.__at_0x200001E0")));

void PDMA_I2S0_TX_Init(uint8_t u8Ch)
{
	PDMA_Open(1 << u8Ch);
	
	// I2S Tx
	DMA_I2S0TXDESC[0].ctl = (((I2S_TX_BUF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32				|		
												PDMA_SAR_INC				|		// Source Address Increment
												PDMA_DAR_FIX				|		// Destination Address fixed
												PDMA_REQ_SINGLE				|		// Transfer Type = single
												PDMA_OP_SCATTER;			        // Basic mode
	DMA_I2S0TXDESC[0].endsrc = (uint32_t)&g_sDataManager.au32I2S_Tx_Buff[0][0];    
	DMA_I2S0TXDESC[0].enddest = (uint32_t)&I2S0->TXFIFO;
	DMA_I2S0TXDESC[0].offset = ((uint32_t)&DMA_I2S0TXDESC[1] - (PDMA->SCATBA));

	DMA_I2S0TXDESC[1].ctl = (((I2S_TX_BUF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32				|		
												PDMA_SAR_INC				|		// Source Address Increment
												PDMA_DAR_FIX				|		// Destination Address fixed
												PDMA_REQ_SINGLE				|		// Transfer Type = single
												PDMA_OP_SCATTER;			        // Basic mode											
	DMA_I2S0TXDESC[1].endsrc = (uint32_t)&g_sDataManager.au32I2S_Tx_Buff[1][0];    
	DMA_I2S0TXDESC[1].enddest = (uint32_t)&I2S0->TXFIFO;
	DMA_I2S0TXDESC[1].offset = ((uint32_t)&DMA_I2S0TXDESC[0] - (PDMA->SCATBA));

	PDMA_SetTransferMode(u8Ch, PDMA_I2S0_TX, TRUE, DMA_I2S0TXDESC[1].offset);
	
	// Enable PDMA channel 0 interrupt
	PDMA_EnableInt(u8Ch, PDMA_INT_TRANS_DONE);
}

void PDMA_I2S0_RX_Init(uint8_t u8Ch)
{
	PDMA_Open((1 << u8Ch));
	
	// I2S RX
	DMA_I2S0RXDESC[0].ctl = (((I2S_RX_BUF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32				|		
												PDMA_SAR_FIX				|		// Source Address Increment
												PDMA_DAR_INC				|		// Destination Address fixed
												PDMA_REQ_SINGLE				|		// Transfer Type = single
												PDMA_OP_SCATTER;			        // Basic mode
	DMA_I2S0RXDESC[0].endsrc = (uint32_t)&I2S0->RXFIFO;
	DMA_I2S0RXDESC[0].enddest = (uint32_t)&g_sDataManager.au32I2S_Rx_Buff[0][0];
	DMA_I2S0RXDESC[0].offset = ((uint32_t)&DMA_I2S0RXDESC[1] - (PDMA->SCATBA));

	DMA_I2S0RXDESC[1].ctl = (((I2S_RX_BUF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32				|		
												PDMA_SAR_FIX				|		// Source Address Increment
												PDMA_DAR_INC				|		// Destination Address fixed
												PDMA_REQ_SINGLE				|		// Transfer Type = single
												PDMA_OP_SCATTER;			        // Basic mode											
	DMA_I2S0RXDESC[1].endsrc = (uint32_t)&I2S0->RXFIFO;    
	DMA_I2S0RXDESC[1].enddest = (uint32_t)&g_sDataManager.au32I2S_Rx_Buff[1][0];
	DMA_I2S0RXDESC[1].offset = ((uint32_t)&DMA_I2S0RXDESC[0] - (PDMA->SCATBA));

	PDMA_SetTransferMode(u8Ch, PDMA_I2S0_RX, TRUE, DMA_I2S0RXDESC[1].offset);
	
	// Enable PDMA channel 1 interrupt
	PDMA_EnableInt(u8Ch, PDMA_INT_TRANS_DONE);
}

void PDMA_SPI1_I2S_TX_Init(uint8_t u8Ch)
{
	PDMA_Open(1 << u8Ch);
	
	// SPI I2S Tx
	DMA_SPI1I2STXDESC[0].ctl = (((I2S_TX_BUF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32				|		
												PDMA_SAR_INC				|		// Source Address Increment
												PDMA_DAR_FIX				|		// Destination Address fixed
												PDMA_REQ_SINGLE				|		// Transfer Type = single
												PDMA_OP_SCATTER;			        // Basic mode
	DMA_SPI1I2STXDESC[0].endsrc = (uint32_t)&g_sDataManager.au32SPI_I2S_Tx_Buff[0][0];    
	DMA_SPI1I2STXDESC[0].enddest = (uint32_t)&SPI1->TX;
	DMA_SPI1I2STXDESC[0].offset = ((uint32_t)&DMA_SPI1I2STXDESC[1] - (PDMA->SCATBA));

	DMA_SPI1I2STXDESC[1].ctl = (((I2S_TX_BUF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32				|		
												PDMA_SAR_INC				|		// Source Address Increment
												PDMA_DAR_FIX				|		// Destination Address fixed
												PDMA_REQ_SINGLE				|		// Transfer Type = single
												PDMA_OP_SCATTER;			        // Basic mode											
	DMA_SPI1I2STXDESC[1].endsrc = (uint32_t)&g_sDataManager.au32SPI_I2S_Tx_Buff[1][0];    
	DMA_SPI1I2STXDESC[1].enddest = (uint32_t)&SPI1->TX;
	DMA_SPI1I2STXDESC[1].offset = ((uint32_t)&DMA_SPI1I2STXDESC[0] - (PDMA->SCATBA));

	PDMA_SetTransferMode(u8Ch, PDMA_SPI1_TX, TRUE, DMA_SPI1I2STXDESC[1].offset);
	
	// Enable PDMA channel 0 interrupt
	PDMA_EnableInt(u8Ch, PDMA_INT_TRANS_DONE);
}

void PDMA_SPI1_I2S_RX_Init(uint8_t u8Ch)
{
	PDMA_Open((1 << u8Ch));
	
	// I2S RX
	DMA_SPI1I2SRXDESC[0].ctl = (((I2S_RX_BUF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32				|		
												PDMA_SAR_FIX				|		// Source Address Increment
												PDMA_DAR_INC				|		// Destination Address fixed
												PDMA_REQ_SINGLE				|		// Transfer Type = single
												PDMA_OP_SCATTER;			        // Basic mode
	DMA_SPI1I2SRXDESC[0].endsrc = (uint32_t)&SPI1->RX;
	DMA_SPI1I2SRXDESC[0].enddest = (uint32_t)&g_sDataManager.au32SPI_I2S_Rx_Buff[0][0];
	DMA_SPI1I2SRXDESC[0].offset = ((uint32_t)&DMA_SPI1I2SRXDESC[1] - (PDMA->SCATBA));

	DMA_SPI1I2SRXDESC[1].ctl = (((I2S_RX_BUF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32				|		
												PDMA_SAR_FIX				|		// Source Address Increment
												PDMA_DAR_INC				|		// Destination Address fixed
												PDMA_REQ_SINGLE				|		// Transfer Type = single
												PDMA_OP_SCATTER;			        // Basic mode											
	DMA_SPI1I2SRXDESC[1].endsrc = (uint32_t)&SPI1->RX;    
	DMA_SPI1I2SRXDESC[1].enddest = (uint32_t)&g_sDataManager.au32SPI_I2S_Rx_Buff[1][0];
	DMA_SPI1I2SRXDESC[1].offset = ((uint32_t)&DMA_SPI1I2SRXDESC[0] - (PDMA->SCATBA));

	PDMA_SetTransferMode(u8Ch, PDMA_SPI1_RX, TRUE, DMA_SPI1I2SRXDESC[1].offset);
	
	// Enable PDMA channel 1 interrupt
	PDMA_EnableInt(u8Ch, PDMA_INT_TRANS_DONE);
}

void PDMA_SPI2_I2S_TX_Init(uint8_t u8Ch)
{
	PDMA_Open(1 << u8Ch);
	
	// SPI I2S Tx
	DMA_SPI2I2STXDESC[0].ctl = (((I2S_TX_BUF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32				|		
												PDMA_SAR_INC				|		// Source Address Increment
												PDMA_DAR_FIX				|		// Destination Address fixed
												PDMA_REQ_SINGLE				|		// Transfer Type = single
												PDMA_OP_SCATTER;			        // Basic mode
	DMA_SPI2I2STXDESC[0].endsrc = (uint32_t)&g_sDataManager.au32SPI_I2S_Tx_Buff[0][0];    
	DMA_SPI2I2STXDESC[0].enddest = (uint32_t)&SPI2->TX;
	DMA_SPI2I2STXDESC[0].offset = ((uint32_t)&DMA_SPI2I2STXDESC[1] - (PDMA->SCATBA));

	DMA_SPI2I2STXDESC[1].ctl = (((I2S_TX_BUF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32				|		
												PDMA_SAR_INC				|		// Source Address Increment
												PDMA_DAR_FIX				|		// Destination Address fixed
												PDMA_REQ_SINGLE				|		// Transfer Type = single
												PDMA_OP_SCATTER;			        // Basic mode											
	DMA_SPI2I2STXDESC[1].endsrc = (uint32_t)&g_sDataManager.au32SPI_I2S_Tx_Buff[1][0];    
	DMA_SPI2I2STXDESC[1].enddest = (uint32_t)&SPI2->TX;
	DMA_SPI2I2STXDESC[1].offset = ((uint32_t)&DMA_SPI2I2STXDESC[0] - (PDMA->SCATBA));

	PDMA_SetTransferMode(u8Ch, PDMA_SPI2_TX, TRUE, DMA_SPI2I2STXDESC[1].offset);
	
	// Enable PDMA channel 0 interrupt
	PDMA_EnableInt(u8Ch, PDMA_INT_TRANS_DONE);
}

void PDMA_SPI2_I2S_RX_Init(uint8_t u8Ch)
{
	PDMA_Open((1 << u8Ch));
	
	// I2S RX
	DMA_SPI2I2SRXDESC[0].ctl = (((I2S_RX_BUF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32				|		
												PDMA_SAR_FIX				|		// Source Address Increment
												PDMA_DAR_INC				|		// Destination Address fixed
												PDMA_REQ_SINGLE				|		// Transfer Type = single
												PDMA_OP_SCATTER;			        // Basic mode
	DMA_SPI2I2SRXDESC[0].endsrc = (uint32_t)&SPI2->RX;
	DMA_SPI2I2SRXDESC[0].enddest = (uint32_t)&g_sDataManager.au32SPI_I2S_Rx_Buff[0][0];
	DMA_SPI2I2SRXDESC[0].offset = ((uint32_t)&DMA_SPI2I2SRXDESC[1] - (PDMA->SCATBA));

	DMA_SPI2I2SRXDESC[1].ctl = (((I2S_RX_BUF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32				|		
												PDMA_SAR_FIX				|		// Source Address Increment
												PDMA_DAR_INC				|		// Destination Address fixed
												PDMA_REQ_SINGLE				|		// Transfer Type = single
												PDMA_OP_SCATTER;			        // Basic mode											
	DMA_SPI2I2SRXDESC[1].endsrc = (uint32_t)&SPI2->RX;    
	DMA_SPI2I2SRXDESC[1].enddest = (uint32_t)&g_sDataManager.au32SPI_I2S_Rx_Buff[1][0];
	DMA_SPI2I2SRXDESC[1].offset = ((uint32_t)&DMA_SPI2I2SRXDESC[0] - (PDMA->SCATBA));

	PDMA_SetTransferMode(u8Ch, PDMA_SPI2_RX, TRUE, DMA_SPI2I2SRXDESC[1].offset);
	
	// Enable PDMA channel 1 interrupt
	PDMA_EnableInt(u8Ch, PDMA_INT_TRANS_DONE);
}

void PDMA_DMIC_Init(uint8_t u8Ch)
{
	PDMA_Open((1 << u8Ch));

	// Rx description
	DMA_DMICDESC[0].ctl = ((DMIC_BUF_LEN-1)<<PDMA_DSCT_CTL_TXCNT_Pos)| 
						PDMA_WIDTH_32|PDMA_SAR_FIX|PDMA_DAR_INC|PDMA_REQ_SINGLE|PDMA_OP_SCATTER;
	DMA_DMICDESC[0].endsrc = (uint32_t)&DMIC->FIFO;
	DMA_DMICDESC[0].enddest = (uint32_t)&g_sDataManager.au32DMIC_Buff[0][0];
	DMA_DMICDESC[0].offset = (uint32_t)&DMA_DMICDESC[1] - (PDMA->SCATBA);

	DMA_DMICDESC[1].ctl = ((DMIC_BUF_LEN-1)<<PDMA_DSCT_CTL_TXCNT_Pos)| 
						PDMA_WIDTH_32|PDMA_SAR_FIX|PDMA_DAR_INC|PDMA_REQ_SINGLE|PDMA_OP_SCATTER;
	DMA_DMICDESC[1].endsrc = (uint32_t)&DMIC->FIFO;
	DMA_DMICDESC[1].enddest = (uint32_t)&g_sDataManager.au32DMIC_Buff[1][0];
	DMA_DMICDESC[1].offset = (uint32_t)&DMA_DMICDESC[0] - (PDMA->SCATBA);   //link to first description

	PDMA->DSCT[2].CTL = PDMA_OP_SCATTER;
	PDMA->DSCT[2].NEXT = (uint32_t)&DMA_DMICDESC[0] - (PDMA->SCATBA);
	
	// Request source is memory to memory
	PDMA_SetTransferMode(u8Ch, PDMA_DMIC_RX, TRUE, DMA_DMICDESC[1].offset);
			
	// Enable PDMA channel 2 interrupt
	PDMA_EnableInt(u8Ch, PDMA_INT_TRANS_DONE);
}
/*
void PDMA_DPWM_Init(uint8_t u8Ch)
{
	PDMA_Open(1 << u8Ch);
	
	DMA_DPWMDESC[0].ctl = (((DPWM_BUF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32				|		
												PDMA_SAR_INC				|		// Source Address Increment
												PDMA_DAR_FIX				|		// Destination Address fixed
												PDMA_REQ_SINGLE				|		// Transfer Type = single
												PDMA_OP_SCATTER;			        // Basic mode
	DMA_DPWMDESC[0].endsrc = (uint32_t)&g_sDataManager.au32DPWM_Buff[0][0];    
	DMA_DPWMDESC[0].enddest = (uint32_t)&DPWM->FIFO;
	DMA_DPWMDESC[0].offset = ((uint32_t)&DMA_DPWMDESC[1] - (PDMA->SCATBA));

	DMA_DPWMDESC[1].ctl = (((DPWM_BUF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32				|		
												PDMA_SAR_INC				|		// Source Address Increment
												PDMA_DAR_FIX				|		// Destination Address fixed
												PDMA_REQ_SINGLE				|		// Transfer Type = single
												PDMA_OP_SCATTER;			        // Basic mode											
	DMA_DPWMDESC[1].endsrc = (uint32_t)&g_sDataManager.au32DPWM_Buff[1][0];    
	DMA_DPWMDESC[1].enddest = (uint32_t)&DPWM->FIFO;
	DMA_DPWMDESC[1].offset = ((uint32_t)&DMA_DPWMDESC[0] - (PDMA->SCATBA));

	PDMA_SetTransferMode(u8Ch, PDMA_DPWM_TX, TRUE, DMA_DPWMDESC[1].offset);
	
	// Enable PDMA channel 0 interrupt
	PDMA_EnableInt(u8Ch, PDMA_INT_TRANS_DONE);
}*/
