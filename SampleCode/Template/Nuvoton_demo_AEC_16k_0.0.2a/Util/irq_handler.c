#include "Platform.h"
#include "init_def.h"
#include "pdma_handler.h"
#include "data_handler.h"
#include "codec.h"

extern DATA_MANAGER g_sDataManager;


void TMR0_IRQHandler(void)
{
	static uint8_t u8Count = 0;

	if(TIMER_GetIntFlag(TIMER0) == 1)
	{
		/* Clear Timer0 time-out interrupt flag */
		TIMER_ClearIntFlag(TIMER0);	
		
//		if(++u8Count >= 10)
//		{
//			SYS_EnableTrimHIRC(SYS_IRCTCTL_FREQSEL_48M);
//			u8Count = 0;
//		}
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
		
		if(u32Transferdone & (1<<I2S0_RX_PDMA_CH) )		
		{			
			pdma_i2s_rx_handler(&g_sDataManager);
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

void I2C0_IRQHandler(void)
{
	Codec_I2C_Handler(I2C0);
}

void I2C1_IRQHandler(void)
{
	Codec_I2C_Handler(I2C1);
}

