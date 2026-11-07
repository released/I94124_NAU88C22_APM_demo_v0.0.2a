#include "Platform.h"
#include "init_def.h"
#include "pdma_handler.h"
#include "data_handler.h"
#include "codec.h"
#include "i2c_handler.h"

extern DATA_MANAGER g_sDataManager;


void TMR0_IRQHandler(void)
{
	static uint8_t u8Count = 0;

	if(TIMER_GetIntFlag(TIMER0) == 1)
	{
		/* Clear Timer0 time-out interrupt flag */
		TIMER_ClearIntFlag(TIMER0);
	
		if(++u8Count >= 10)
		{
			SYS_EnableTrimHIRC(SYS_IRCTCTL_FREQSEL_48M);
			u8Count = 0;
		}
	}
}

volatile uint8_t tmr3_flag = 0;
volatile uint32_t tmr3_tick = 0;
/*
 * The 1 kHz timer requests a codec-cache refresh every 250 ms.
 * I2C1 register reads are performed later in the foreground command handler.
 */
void TMR3_IRQHandler(void)
{
	if(TIMER_GetIntFlag(TIMER3) == 1)
	{
		/* Clear Timer3 time-out interrupt flag */
		TIMER_ClearIntFlag(TIMER3);
	
		tmr3_tick++;
		if (tmr3_tick >= 60000)
		{
			tmr3_tick = 0;
		}

		if ((tmr3_tick % 250) == 0)	// raise flag
		{
			tmr3_flag = 1;
		}	

	}
}

/*
 * Dispatch hardware transfer-complete flags to the PCM transport handlers.
 * The APM itself runs in main, outside this interrupt. Ready flags are
 * notifications; they are not a lossless queue or an ownership lock.
 */
void PDMA_IRQHandler(void)
{
	uint32_t u32Status;
	uint32_t u32Transferdone;	
	
	// Get interrupt status.
	u32Status = PDMA_GET_INT_STATUS();
	
	// TDF-PDMA Channel Transfer Done Flag
	u32Transferdone = PDMA_GET_TD_STS();
	// write 1 to clear
	PDMA_CLR_TD_FLAG(u32Transferdone);

	// PDMA Read/Write Target Abort Interrupt Flag
	if(u32Status & PDMA_STATUS_ABTIF) 				
	{
		if (PDMA_GET_ABORT_STS() & (1<<I2S0_TX_PDMA_CH))  
		{
			PDMA_CLR_ABORT_FLAG((1<<I2S0_TX_PDMA_CH));
		}		
		if (PDMA_GET_ABORT_STS() & (1<<I2S0_RX_PDMA_CH))  
		{
			PDMA_CLR_ABORT_FLAG((1<<I2S0_RX_PDMA_CH));
		}
		if (PDMA_GET_ABORT_STS() & (1<<DPWM_PDMA_CH))  
		{
			PDMA_CLR_ABORT_FLAG((1<<DPWM_PDMA_CH));
		}
		if (PDMA_GET_ABORT_STS() & (1<<DMIC_PDMA_CH))  
		{
			PDMA_CLR_ABORT_FLAG((1<<DMIC_PDMA_CH));
		}		
	}
	else if(u32Status & PDMA_STATUS_TDIF) 	// Transfer Done Interrupt Flag
	{	
		if(u32Transferdone & (1<<I2S0_TX_PDMA_CH) )	
		{
			pdma_i2s_tx_handler(&g_sDataManager);
		}
		
		if(u32Transferdone & (1<<SPI1_I2S_TX_PDMA_CH) )	
		{
			pdma_spi_i2s_tx_handler(&g_sDataManager);
		}
		
		if(u32Transferdone & (1<<SPI2_I2S_TX_PDMA_CH) )	
		{
			pdma_spi_i2s_tx_handler(&g_sDataManager);
		}
		
		if(u32Transferdone & (1<<I2S0_RX_PDMA_CH) )		
		{			
			pdma_i2s_rx_handler(&g_sDataManager);
		}

		if(u32Transferdone & (1<<SPI1_I2S_RX_PDMA_CH) )	
		{
			pdma_spi_i2s_rx_handler(&g_sDataManager);
		}
		
		if(u32Transferdone & (1<<SPI2_I2S_RX_PDMA_CH) )	
		{
			pdma_spi_i2s_rx_handler(&g_sDataManager);
		}
		
		if(u32Transferdone & (1<<DMIC_PDMA_CH) )		
		{			
			pdma_dmic_handler(&g_sDataManager);
		}
		
		if(u32Transferdone & (1<<DPWM_PDMA_CH) )	
		{
//			pdma_dpwm_handler(&g_sDataManager);
		}		
	}
}

/*
 * External host control bus: receive commands / return one-byte replies.
 */
void I2C0_IRQHandler(void)
{
	I2C_Cmd_Receiver(I2C0);
}

/*
 * Separate codec-control master bus: advance codec register transactions.
 */
void I2C1_IRQHandler(void)
{
	Codec_I2C_Handler(I2C1);
}

