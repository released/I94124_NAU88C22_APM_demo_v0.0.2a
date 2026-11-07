#include "pdma_handler.h"
#include "usb_audio_def.h"
#include "usb_audio_const.h"
#include "string.h"


/*
 * I2S0 TX completion: refill one codec playback row (32 stereo words).
 * Source is the USB playback ring; stopped/short input produces zeros.
 * When recording enables APM, copy this prepared TX row low16 into Far.
 * Far is a digital reference before codec DAC/speaker gain and acoustics.
 * It is assembled at refill time, not an analog speaker measurement.
 */
void pdma_i2s_tx_handler(DATA_MANAGER* psDataManager)
{
	uint32_t i, j;
	uint32_t* pu32Tmp;

	if(psDataManager->psUacPlayback->u8AudioSpeakerState == UAC_STOP_AUDIO_SPEAK)
	{
		memset(&psDataManager->au32I2S_Tx_Buff[psDataManager->u8I2S_Tx_Buff_Index][0],
		0,
		(I2S_TX_BUF_LEN<<2));
	}
	else
	{
		if(psDataManager->psUacPlayback->u16AvalCnt < I2S_TX_BUF_LEN) // i2s 2ch 16bits
		{
			// Lack data from USB
			memset(&psDataManager->au32I2S_Tx_Buff[psDataManager->u8I2S_Tx_Buff_Index][0],
					0,
					(I2S_TX_BUF_LEN<<2));	
		}	
		else
		{
			for (i = 0 ; i < I2S_TX_BUF_LEN ; i++)
			{
				psDataManager->au32I2S_Tx_Buff[psDataManager->u8I2S_Tx_Buff_Index][i] = psDataManager->psUacPlayback->au32USB_RingBuff[psDataManager->psUacPlayback->u16UAC_Buff_ReadIndex++];
				
				if(psDataManager->psUacPlayback->u16UAC_Buff_ReadIndex >= SPK_RING_BUFFER_LEN)
					psDataManager->psUacPlayback->u16UAC_Buff_ReadIndex = 0;			
				
				psDataManager->psUacPlayback->u16AvalCnt -= 1;		
			}	
		}
	}
	
	// For far end, get one ch data
	if(psDataManager->bAlgEnable)
	{	
		for (i = 0 ; i < I2S_TX_BUF_LEN ; i++)
		{
			psDataManager->u16FarI2STmp[psDataManager->u8FarI2STmp_Buf_Idx][psDataManager->u16FarI2STmpWIdx] = 
				(psDataManager->au32I2S_Tx_Buff[psDataManager->u8I2S_Tx_Buff_Index][i]&0xffff);
			psDataManager->u16FarI2STmpWIdx++;
		}
		
		if(psDataManager->u16FarI2STmpWIdx >= COLSONIC_DAT_LEN)
		{
			psDataManager->u16FarI2STmpWIdx = 0;
			psDataManager->bFarReady = TRUE;
			psDataManager->pu16Far = &psDataManager->u16FarI2STmp[psDataManager->u8FarI2STmp_Buf_Idx][0];
			psDataManager->u8FarI2STmp_Buf_Idx ^= 0x1;
		}
	}
	
	psDataManager->u8I2S_Tx_Buff_Index ^= 0x1;
}

void pdma_dpwm_handler(DATA_MANAGER* psDataManager)
{
}

/*
 * I2S0 RX completion first drains 32 output words into the USB record
 * byte ring (or inserts zeros), then gathers low16 codec RX samples as Near.
 * Every 320 samples publish Near; when both Near/Far are ready, this RX
 * handler raises bDoAECReady for foreground processing.
 */
/* Ready flags are notifications only. There are no frame sequence IDs
 * or timestamps to prove Near/Far alignment, and no ownership lock
 * prevents row reuse while the foreground APM is still reading. */
void pdma_i2s_rx_handler(DATA_MANAGER* psDataManager)
{
	uint32_t i, j;
	uint8_t* pu8Tmp;
	uint32_t* pu32Tmp;
	uint32_t* pu32D = &psDataManager->u32Decode[0][0];

	
	if (psDataManager->psUacRecord->u8AudioMicState == UAC_START_AUDIO_RECORD)
	{
		if(psDataManager->u16DecodeAvalCnt < I2S_RX_BUF_LEN)
		{
			for(i = 0; i < I2S_RX_BUF_LEN; i++)
			{
				pu32Tmp = (uint32_t*)&psDataManager->psUacRecord->au8USB_RingBuff[psDataManager->psUacRecord->u16UAC_Buff_WriteIndex];
				*pu32Tmp = 0;
				
				psDataManager->psUacRecord->u16UAC_Buff_WriteIndex += 4;
				if (psDataManager->psUacRecord->u16UAC_Buff_WriteIndex >= MIC_RING_BUFFER_LEN)
					psDataManager->psUacRecord->u16UAC_Buff_WriteIndex = 0;				
			}			
		}
		else
		{
			for(i = 0; i < I2S_RX_BUF_LEN; i++)
			{
				pu32Tmp = (uint32_t*)&psDataManager->psUacRecord->au8USB_RingBuff[psDataManager->psUacRecord->u16UAC_Buff_WriteIndex];
				*pu32Tmp = pu32D[psDataManager->u16DecodeBufRIdx];
				
				if(++psDataManager->u16DecodeBufRIdx >= (COLSONIC_DAT_LEN<<1))
					psDataManager->u16DecodeBufRIdx = 0;
								
				psDataManager->psUacRecord->u16UAC_Buff_WriteIndex += 4;
				if (psDataManager->psUacRecord->u16UAC_Buff_WriteIndex >= MIC_RING_BUFFER_LEN)
					psDataManager->psUacRecord->u16UAC_Buff_WriteIndex = 0;				
			}
			psDataManager->u16DecodeAvalCnt -= I2S_RX_BUF_LEN;
		}
		
		psDataManager->psUacRecord->u16AvalCnt += (I2S_RX_BUF_LEN<<2);
		
		if(psDataManager->psUacRecord->u16AvalCnt >= (MIC_RING_BUFFER_LEN>>1))
			psDataManager->psUacRecord->u8AudioMicState = UAC_BUSY_AUDIO_RECORD;

	}
	else if(psDataManager->psUacRecord->u8AudioMicState == UAC_BUSY_AUDIO_RECORD)
	{
		if(psDataManager->u16DecodeAvalCnt < I2S_RX_BUF_LEN)
		{
			for(i = 0; i < I2S_RX_BUF_LEN; i++)
			{
				pu32Tmp = (uint32_t*)&psDataManager->psUacRecord->au8USB_RingBuff[psDataManager->psUacRecord->u16UAC_Buff_WriteIndex];
				*pu32Tmp = 0;
				
				psDataManager->psUacRecord->u16UAC_Buff_WriteIndex += 4;
				if (psDataManager->psUacRecord->u16UAC_Buff_WriteIndex >= MIC_RING_BUFFER_LEN)
					psDataManager->psUacRecord->u16UAC_Buff_WriteIndex = 0;				
			}			
		}
		else
		{
			for(i = 0; i < I2S_RX_BUF_LEN; i++)
			{
				pu32Tmp = (uint32_t*)&psDataManager->psUacRecord->au8USB_RingBuff[psDataManager->psUacRecord->u16UAC_Buff_WriteIndex];
				*pu32Tmp = pu32D[psDataManager->u16DecodeBufRIdx];
				
				if(++psDataManager->u16DecodeBufRIdx >= (COLSONIC_DAT_LEN<<1))
					psDataManager->u16DecodeBufRIdx = 0;
				
				psDataManager->psUacRecord->u16UAC_Buff_WriteIndex += 4;				
				if (psDataManager->psUacRecord->u16UAC_Buff_WriteIndex >= MIC_RING_BUFFER_LEN)
					psDataManager->psUacRecord->u16UAC_Buff_WriteIndex = 0;				
			}
			
			psDataManager->u16DecodeAvalCnt -= I2S_RX_BUF_LEN;
		}
		
		psDataManager->psUacRecord->u16AvalCnt += (I2S_RX_BUF_LEN<<2);
	}
	
	// For near end, get one ch data
	if(psDataManager->bAlgEnable)
	{
		for (i = 0 ; i < I2S_RX_BUF_LEN ; i++)
		{
			psDataManager->u16NearI2STmp[psDataManager->u8NearI2STmp_Buf_Idx][psDataManager->u16NearI2STmpWIdx] = 
				(psDataManager->au32I2S_Rx_Buff[psDataManager->u8I2S_Rx_Buff_Index][i]&0xffff);
			psDataManager->u16NearI2STmpWIdx += 1;
		}
		
		if(psDataManager->u16NearI2STmpWIdx >= COLSONIC_DAT_LEN)
		{
			psDataManager->u16NearI2STmpWIdx = 0;
			psDataManager->bNearReady = TRUE;
			psDataManager->pu16Near = &psDataManager->u16NearI2STmp[psDataManager->u8NearI2STmp_Buf_Idx][0];
			psDataManager->u8NearI2STmp_Buf_Idx ^= 0x1;
		}
		
		if(psDataManager->bNearReady && psDataManager->bFarReady)
		{
			psDataManager->bDoAECReady = TRUE;
			psDataManager->bNearReady= FALSE;
			psDataManager->bFarReady = FALSE;
		}
	}
	
	psDataManager->u8I2S_Rx_Buff_Index ^= 0x1; 	
}

void pdma_dmic_handler(DATA_MANAGER* psDataManager)
{
	uint32_t i, j, tmp;
	uint32_t* pu32Tmp;
	uint32_t* pu32D = &psDataManager->u32Decode[0][0];
	
	if (psDataManager->psUacRecord->u8AudioMicState == UAC_START_AUDIO_RECORD)
	{
		if(psDataManager->u16DecodeAvalCnt < DMIC_BUF_LEN)
		{
			for(i = 0; i < DMIC_BUF_LEN; i++)
			{
				pu32Tmp = (uint32_t*)&psDataManager->psUacRecord->au8USB_RingBuff[psDataManager->psUacRecord->u16UAC_Buff_WriteIndex];
				*pu32Tmp = 0;
				
				psDataManager->psUacRecord->u16UAC_Buff_WriteIndex += 4;
				if (psDataManager->psUacRecord->u16UAC_Buff_WriteIndex >= MIC_RING_BUFFER_LEN)
					psDataManager->psUacRecord->u16UAC_Buff_WriteIndex = 0;				
			}			
		}
		else
		{
			for (i = 0 ; i < DMIC_BUF_LEN ; i++)
			{	
				pu32Tmp = (uint32_t*)&psDataManager->psUacRecord->au8USB_RingBuff[psDataManager->psUacRecord->u16UAC_Buff_WriteIndex];
				*pu32Tmp = pu32D[psDataManager->u16DecodeBufRIdx];
				
				if(++psDataManager->u16DecodeBufRIdx >= (COLSONIC_DAT_LEN<<1))
					psDataManager->u16DecodeBufRIdx = 0;
								
				psDataManager->psUacRecord->u16UAC_Buff_WriteIndex += 4;
				if (psDataManager->psUacRecord->u16UAC_Buff_WriteIndex >= MIC_RING_BUFFER_LEN)
					psDataManager->psUacRecord->u16UAC_Buff_WriteIndex = 0;
			}
			
			psDataManager->u16DecodeAvalCnt -= DMIC_BUF_LEN;
		}
		
		psDataManager->psUacRecord->u16AvalCnt += (DMIC_BUF_LEN<<2);
		
		if(psDataManager->psUacRecord->u16AvalCnt >= (MIC_RING_BUFFER_LEN>>1))
			psDataManager->psUacRecord->u8AudioMicState = UAC_BUSY_AUDIO_RECORD;
	}
	// Renew the data from record buffer to PDMA buffer
	else if (psDataManager->psUacRecord->u8AudioMicState == UAC_BUSY_AUDIO_RECORD)
	{				
		if(psDataManager->u16DecodeAvalCnt < DMIC_BUF_LEN)
		{
			for(i = 0; i < DMIC_BUF_LEN; i++)
			{
				pu32Tmp = (uint32_t*)&psDataManager->psUacRecord->au8USB_RingBuff[psDataManager->psUacRecord->u16UAC_Buff_WriteIndex];
				*pu32Tmp = 0;
				
				psDataManager->psUacRecord->u16UAC_Buff_WriteIndex += 4;
				if (psDataManager->psUacRecord->u16UAC_Buff_WriteIndex >= MIC_RING_BUFFER_LEN)
					psDataManager->psUacRecord->u16UAC_Buff_WriteIndex = 0;				
			}			
		}
		else
		{
			for (i = 0 ; i < DMIC_BUF_LEN ; i++)
			{	
				pu32Tmp = (uint32_t*)&psDataManager->psUacRecord->au8USB_RingBuff[psDataManager->psUacRecord->u16UAC_Buff_WriteIndex];
				*pu32Tmp = pu32D[psDataManager->u16DecodeBufRIdx];
				
				if(++psDataManager->u16DecodeBufRIdx >= (COLSONIC_DAT_LEN<<1))
					psDataManager->u16DecodeBufRIdx = 0;
								
				psDataManager->psUacRecord->u16UAC_Buff_WriteIndex += 4;
				if (psDataManager->psUacRecord->u16UAC_Buff_WriteIndex >= MIC_RING_BUFFER_LEN)
					psDataManager->psUacRecord->u16UAC_Buff_WriteIndex = 0;
			}
			
			psDataManager->u16DecodeAvalCnt -= DMIC_BUF_LEN;
		}
		
		psDataManager->psUacRecord->u16AvalCnt += (DMIC_BUF_LEN<<2);			
	}

	// For near end, get one ch data
	if(psDataManager->bAlgEnable)
	{
		for (i = 0 ; i < DMIC_BUF_LEN ; i++)
		{
			psDataManager->u16NearDmicTmp[psDataManager->u8NearDmicTmp_Buf_Idx][psDataManager->u16NearDmicTmpWIdx] = 
				(psDataManager->au32DMIC_Buff[psDataManager->u8DMIC_Buff_Index][i]>>8)&0xffff;
			psDataManager->u16NearDmicTmpWIdx += 1;
		}
		
		if(psDataManager->u16NearDmicTmpWIdx >= COLSONIC_DAT_LEN)
		{
			psDataManager->u16NearDmicTmpWIdx = 0;
			psDataManager->bNearReady = TRUE;
			psDataManager->pu16Near = &psDataManager->u16NearDmicTmp[psDataManager->u8NearDmicTmp_Buf_Idx][0];
			psDataManager->u8NearDmicTmp_Buf_Idx ^= 0x1;
		}
	}
	
	// Switch to another buffer row.
	psDataManager->u8DMIC_Buff_Index ^= 0x1; 	
}

void pdma_spi_i2s_tx_handler(DATA_MANAGER* psDataManager)
{	
	uint32_t i, j;
	uint32_t* pu32Tmp;

	if(psDataManager->psUacPlayback->u8AudioSpeakerState == UAC_STOP_AUDIO_SPEAK)
	{
		memset(&psDataManager->au32SPI_I2S_Tx_Buff[psDataManager->u8SPI_I2S_Tx_Buff_Index][0],
		0,
		(I2S_TX_BUF_LEN<<2));
	}
	else
	{
		if(psDataManager->psUacPlayback->u16AvalCnt < I2S_TX_BUF_LEN) // i2s 2ch 16bits
		{
			// Lack data from USB
			memset(&psDataManager->au32SPI_I2S_Tx_Buff[psDataManager->u8SPI_I2S_Tx_Buff_Index][0],
					0,
					(I2S_TX_BUF_LEN<<2));	
		}	
		else
		{
			for (i = 0 ; i < I2S_TX_BUF_LEN ; i++)
			{
				psDataManager->au32SPI_I2S_Tx_Buff[psDataManager->u8SPI_I2S_Tx_Buff_Index][i] 
					= psDataManager->psUacPlayback->au32USB_RingBuff[psDataManager->psUacPlayback->u16UAC_Buff_ReadIndex++];
				
				if(psDataManager->psUacPlayback->u16UAC_Buff_ReadIndex >= SPK_RING_BUFFER_LEN)
					psDataManager->psUacPlayback->u16UAC_Buff_ReadIndex = 0;			
				
				psDataManager->psUacPlayback->u16AvalCnt -= 1;		
			}	
		}
	}

	psDataManager->u8SPI_I2S_Tx_Buff_Index ^= 1;	
}


void pdma_spi_i2s_rx_handler(DATA_MANAGER* psDataManager)
{
	uint32_t i, j;
	uint8_t* pu8Tmp;
	uint32_t* pu32Tmp;
	uint32_t* pu32D = &psDataManager->u32Decode[0][0];

	
	if (psDataManager->psUacRecord->u8AudioMicState == UAC_START_AUDIO_RECORD)
	{
		if(psDataManager->u16DecodeAvalCnt < I2S_RX_BUF_LEN)
		{
			for(i = 0; i < I2S_RX_BUF_LEN; i++)
			{
				pu32Tmp = (uint32_t*)&psDataManager->psUacRecord->au8USB_RingBuff[psDataManager->psUacRecord->u16UAC_Buff_WriteIndex];
				*pu32Tmp = 0;
				
				psDataManager->psUacRecord->u16UAC_Buff_WriteIndex += 4;
				if (psDataManager->psUacRecord->u16UAC_Buff_WriteIndex >= MIC_RING_BUFFER_LEN)
					psDataManager->psUacRecord->u16UAC_Buff_WriteIndex = 0;				
			}			
		}
		else
		{
			for(i = 0; i < I2S_RX_BUF_LEN; i++)
			{
				pu32Tmp = (uint32_t*)&psDataManager->psUacRecord->au8USB_RingBuff[psDataManager->psUacRecord->u16UAC_Buff_WriteIndex];
				*pu32Tmp = pu32D[psDataManager->u16DecodeBufRIdx];
				
				if(++psDataManager->u16DecodeBufRIdx >= (COLSONIC_DAT_LEN<<1))
					psDataManager->u16DecodeBufRIdx = 0;
								
				psDataManager->psUacRecord->u16UAC_Buff_WriteIndex += 4;
				if (psDataManager->psUacRecord->u16UAC_Buff_WriteIndex >= MIC_RING_BUFFER_LEN)
					psDataManager->psUacRecord->u16UAC_Buff_WriteIndex = 0;				
			}
			psDataManager->u16DecodeAvalCnt -= I2S_RX_BUF_LEN;
		}
		
		psDataManager->psUacRecord->u16AvalCnt += (I2S_RX_BUF_LEN<<2);
		
		if(psDataManager->psUacRecord->u16AvalCnt >= (MIC_RING_BUFFER_LEN>>1))
			psDataManager->psUacRecord->u8AudioMicState = UAC_BUSY_AUDIO_RECORD;

	}
	else if(psDataManager->psUacRecord->u8AudioMicState == UAC_BUSY_AUDIO_RECORD)
	{
		if(psDataManager->u16DecodeAvalCnt < I2S_RX_BUF_LEN)
		{
			for(i = 0; i < I2S_RX_BUF_LEN; i++)
			{
				pu32Tmp = (uint32_t*)&psDataManager->psUacRecord->au8USB_RingBuff[psDataManager->psUacRecord->u16UAC_Buff_WriteIndex];
				*pu32Tmp = 0;
				
				psDataManager->psUacRecord->u16UAC_Buff_WriteIndex += 4;
				if (psDataManager->psUacRecord->u16UAC_Buff_WriteIndex >= MIC_RING_BUFFER_LEN)
					psDataManager->psUacRecord->u16UAC_Buff_WriteIndex = 0;				
			}			
		}
		else
		{
			for(i = 0; i < I2S_RX_BUF_LEN; i++)
			{
				pu32Tmp = (uint32_t*)&psDataManager->psUacRecord->au8USB_RingBuff[psDataManager->psUacRecord->u16UAC_Buff_WriteIndex];
				*pu32Tmp = pu32D[psDataManager->u16DecodeBufRIdx];
				
				if(++psDataManager->u16DecodeBufRIdx >= (COLSONIC_DAT_LEN<<1))
					psDataManager->u16DecodeBufRIdx = 0;
				
				psDataManager->psUacRecord->u16UAC_Buff_WriteIndex += 4;				
				if (psDataManager->psUacRecord->u16UAC_Buff_WriteIndex >= MIC_RING_BUFFER_LEN)
					psDataManager->psUacRecord->u16UAC_Buff_WriteIndex = 0;				
			}
			
			psDataManager->u16DecodeAvalCnt -= I2S_RX_BUF_LEN;
		}
		
		psDataManager->psUacRecord->u16AvalCnt += (I2S_RX_BUF_LEN<<2);
	}
	
	if(psDataManager->bAlgEnable)
	{	
		for (i = 0 ; i < I2S_RX_BUF_LEN ; i++)
		{
			int16_t left, right;
			right = (int16_t)((psDataManager->au32SPI_I2S_Rx_Buff[psDataManager->u8SPI_I2S_Rx_Buff_Index][i]>>16)&0xffff);
			left =(int16_t)((psDataManager->au32SPI_I2S_Rx_Buff[psDataManager->u8SPI_I2S_Rx_Buff_Index][i])&0xffff);
			psDataManager->u16FarI2STmp[psDataManager->u8FarI2STmp_Buf_Idx][psDataManager->u16FarI2STmpWIdx]
			    = (right/2)+(left/2);
			psDataManager->u16FarI2STmpWIdx++;
		}
		
		if(psDataManager->u16FarI2STmpWIdx >= COLSONIC_DAT_LEN)
		{
			psDataManager->u16FarI2STmpWIdx = 0;
			psDataManager->bFarReady = TRUE;
			psDataManager->pu16Far = &psDataManager->u16FarI2STmp[psDataManager->u8FarI2STmp_Buf_Idx][0];
			psDataManager->u8FarI2STmp_Buf_Idx ^= 0x1;
		}
	}
	
	psDataManager->u8SPI_I2S_Rx_Buff_Index ^= 0x1; 	
}
