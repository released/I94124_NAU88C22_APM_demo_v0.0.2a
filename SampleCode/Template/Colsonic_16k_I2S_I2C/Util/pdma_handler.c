#include "pdma_handler.h"
#include "usb_audio_def.h"
#include "usb_audio_const.h"
#include "string.h"


/*
 * I2S0 TX completion: refill one codec playback row (32 stereo words).
 * Source is a host SPI I2S RX row; short input or digital mute gives zeros.
 * Copy the prepared codec TX row low16 into Far, including inserted zeros.
 * Far is a digital reference before codec DAC/speaker gain and acoustics.
 * It is assembled at refill time, not an analog speaker measurement.
 */
void pdma_i2s_tx_handler(DATA_MANAGER* psDataManager)
{
	uint32_t i, j;
	uint32_t* pu32Tmp;

	// For far end, get one ch data
	if(psDataManager->u16SPI_I2S_Rx_AvalCnt < I2S_TX_BUF_LEN)
	{
		memset(&psDataManager->au32I2S_Tx_Buff[psDataManager->u8I2S_Tx_Buff_Index][0],
				0,
				(I2S_TX_BUF_LEN<<2));	
	}
	else
	{
		if(psDataManager->bI2S_Tx_Mute)
		{
			memset(&psDataManager->au32I2S_Tx_Buff[psDataManager->u8I2S_Tx_Buff_Index][0],
					0,
					(I2S_TX_BUF_LEN<<2));	
		}
		else
		{
			memcpy(&psDataManager->au32I2S_Tx_Buff[psDataManager->u8I2S_Tx_Buff_Index][0],
				   &psDataManager->au32SPI_I2S_Rx_Buff[psDataManager->u16SPI_I2S_Rx_Buff_ReadIndex][0],
				   (I2S_TX_BUF_LEN<<2)); 			
		}
		
		psDataManager->u16SPI_I2S_Rx_Buff_ReadIndex ^= 1;
		psDataManager->u16SPI_I2S_Rx_AvalCnt -= I2S_TX_BUF_LEN;
	}

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
	
	
	psDataManager->u8I2S_Tx_Buff_Index ^= 0x1;
}

void pdma_dpwm_handler(DATA_MANAGER* psDataManager)
{
//	uint32_t i, j;
//	uint8_t* pu8Tmp;
//	uint16_t* pu16Tmp;
//	int16_t i16Tmp;
//	int32_t i32Tmp;
		
	// 16bits
//	{
//		for (i = 0 ; i < PDMA2PLAY_BUFF_LEN ; i++)
//		{
//			if(psUacPlayback->u16AvalCnt >= 2)
//			{
//				if(psUacPlayback->usbd_PlayMute)
//				{
//					psUacPlayback->au32PDMA2PLAY_Buff[psUacPlayback->u8Play_Buff_Index][i] = 0;
//					psUacPlayback->u16UAC_Buff_ReadIndex += 2;
//				}
//				else
//				{
//					pu16Tmp = (uint16_t*)&psUacPlayback->au32PDMA2PLAY_Buff[psUacPlayback->u8Play_Buff_Index][i];
//					pu8Tmp = (uint8_t*)&psUacPlayback->au32PDMA2PLAY_Buff[psUacPlayback->u8Play_Buff_Index][i];
//					
//					for(j = 0;j < 2; j++)
//					{
//						pu8Tmp[j] = psUacPlayback->au8USB_RingBuff[psUacPlayback->u16UAC_Buff_ReadIndex++];
//						if(psUacPlayback->u16UAC_Buff_ReadIndex >= SPK_RING_BUFFER_LEN)
//							psUacPlayback->u16UAC_Buff_ReadIndex = 0;
//					}										
//				}						
//				
//				psUacPlayback->u16AvalCnt -= 2;
//			}
//			else
//			{
//				psUacPlayback->au32PDMA2PLAY_Buff[psUacPlayback->u8Play_Buff_Index][i] = 0; 
//			}
//		}	
//	}

//	psUacPlayback->u8Play_Buff_Index ^= 0x1; 		
}

/*
 * I2S0 RX completion: extract low16 from 32 stereo words as raw Near.
 * After 320 samples publish pu16Near/bNearReady and switch frame row.
 * No APM processing is done inside this handler.
 */
/* Ready flags are notifications only. There are no frame sequence IDs
 * or timestamps to prove Near/Far alignment, and no ownership lock
 * prevents row reuse while the foreground APM is still reading. */
void pdma_i2s_rx_handler(DATA_MANAGER* psDataManager)
{	
	// For near end, get one ch data
    uint32_t i;

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
		
	psDataManager->u8I2S_Rx_Buff_Index ^= 0x1; 	
}

void pdma_dmic_handler(DATA_MANAGER* psDataManager)
{
	uint32_t i, j, tmp;
	uint32_t* pu32Tmp;
	
	// For near end, get one ch data

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
	
	
	// Switch to another buffer row.
	psDataManager->u8DMIC_Buff_Index ^= 0x1; 	
}

/*
 * Host uplink: drain 32 packed words from the 640-word u32Decode queue
 * into the completed SPI TX row; output zeros when fewer words are ready.
 * Copy packed words unchanged: normal L=Near/R=Decode, bypass L=Far/R=Near.
 * The DMA sends that row to the host on its next turn.
 */
void pdma_spi_i2s_tx_handler(DATA_MANAGER* psDataManager)
{
	uint32_t i;
	uint32_t* pu32D = &psDataManager->u32Decode[0][0];
	
	if(psDataManager->u16DecodeAvalCnt < I2S_TX_BUF_LEN)
	{
		memset(&psDataManager->au32SPI_I2S_Tx_Buff[psDataManager->u16SPI_I2S_Tx_Buff_WriteIndex][0],
				0,
				(I2S_TX_BUF_LEN<<2));		
	}
	else
	{
		for(i = 0; i < I2S_TX_BUF_LEN; i++)
		{
			psDataManager->au32SPI_I2S_Tx_Buff[psDataManager->u16SPI_I2S_Tx_Buff_WriteIndex][i] = 
			      pu32D[psDataManager->u16DecodeBufRIdx];
			
			if(++psDataManager->u16DecodeBufRIdx >= (COLSONIC_DAT_LEN<<1))
					psDataManager->u16DecodeBufRIdx = 0;
			
		}
		psDataManager->u16DecodeAvalCnt -= I2S_TX_BUF_LEN;
	}	

	psDataManager->u16SPI_I2S_Tx_Buff_WriteIndex ^= 1;
}


/*
 * Host downlink is already in the SPI RX row written by hardware PDMA.
 * Only publish 32 more available words; codec TX later consumes the row.
 * This counter has no overflow guard or frame sequence tracking.
 */
void pdma_spi_i2s_rx_handler(DATA_MANAGER* psDataManager)
{
	uint32_t i;
	psDataManager->u16SPI_I2S_Rx_AvalCnt += I2S_RX_BUF_LEN;	
}
