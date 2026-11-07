#ifndef __UAC_HANDLER_H__
#define __UAC_HANDLER_H__

#include "stdint.h" 
#include "usb_audio_def.h"
#include "buf_def.h"

typedef struct _uac_playback_t
{
	uint8_t  u8AudioSpeakerState;  
	uint32_t u32PlaySampleRate;
	uint8_t  u8BitRes;
	uint8_t  u8SubFrm;
	uint8_t  u8Channel;

	uint32_t au32USB_RingBuff[SPK_RING_BUFFER_LEN];
	uint16_t u16UAC_Buff_ReadIndex;
	uint16_t u16UAC_Buff_WriteIndex;	
	uint16_t u16AvalCnt;  
	
	// parameters
	uint8_t  usbd_PlayMute;       /* Play MUTE control. 0 = normal. 1 = MUTE */
	int16_t  usbd_PlayVolume;    /* Play channel volume. */
	int16_t  usbd_PlayMaxVolume; 
	int16_t  usbd_PlayMinVolume; 
	int16_t  usbd_PlayResVolume; 
	
	int16_t  usbd_PlayCurVolume;   
	uint8_t usbd_PlayVolumeIndex;
} UAC_PLAYBACK_T;

typedef struct _uac_recoder_t
{
	uint8_t  u8AudioMicState;
	uint8_t  u8BitRes;
	uint8_t  u8SubFrm;
	uint8_t  u8Channel;
	uint32_t u32RecRate;	
	
	uint8_t au8USB_RingBuff[MIC_RING_BUFFER_LEN];
	uint16_t u16UAC_Buff_ReadIndex;
	uint16_t u16UAC_Buff_WriteIndex;
	uint16_t u16AvalCnt;  
	
	// parameters
	uint8_t usbd_RecMute;       /* Record MUTE control. 0 = normal. 1 = MUTE */
	int16_t usbd_RecVolume;    /* Record channel volume. */
	int16_t usbd_RecMaxVolume;  
	int16_t usbd_RecMinVolume;  
	int16_t usbd_RecResVolume;  

} UAC_RECODER_T;


/*
 * PCM ownership map (see the sample Markdown guide):
 * au32I2S_*_Buff: hardware PDMA rows, [2][32] stereo words each.
 * u16NearI2STmp/u16FarI2STmp: CPU-built [2][320] mono frame storage.
 * pu16Near/pu16Far: aliases of published rows, not copied frame queues.
 * u16DecodeTmp: 320 mono processed samples written by the APM.
 * u32Decode: [2][320] packed stereo words, consumed 32 words at a time.
 * u16DecodeAvalCnt and its read index use words, never bytes.
 * USB playback ring counters use uint32_t words; record counters use bytes.
 * The internal bypass flag defaults to zero; no switch is exposed here.
 */
typedef struct _data_manager_t
{
    //I2S
	uint32_t au32I2S_Tx_Buff[2][I2S_TX_BUF_LEN]; 
	uint8_t u8I2S_Tx_Buff_Index;	
	uint32_t au32I2S_Rx_Buff[2][I2S_RX_BUF_LEN]; 
	uint8_t u8I2S_Rx_Buff_Index;
	// SPI_I2S
	uint32_t au32SPI_I2S_Tx_Buff[2][I2S_TX_BUF_LEN]; 
	uint8_t u8SPI_I2S_Tx_Buff_Index;
	uint32_t au32SPI_I2S_Rx_Buff[2][I2S_RX_BUF_LEN]; 
	uint8_t u8SPI_I2S_Rx_Buff_Index;
    // DMIC
	uint32_t au32DMIC_Buff[2][DMIC_BUF_LEN]; 
	uint8_t u8DMIC_Buff_Index;	
	
// Far end
	uint16_t u16FarI2STmp[2][COLSONIC_DAT_LEN];
	uint16_t u16FarI2STmpWIdx;	
	uint8_t u8FarI2STmp_Buf_Idx;

// Near end
	uint16_t u16NearI2STmp[2][COLSONIC_DAT_LEN];
	uint16_t u16NearI2STmpWIdx;
	uint8_t u8NearI2STmp_Buf_Idx;
	uint16_t u16NearDmicTmp[2][COLSONIC_DAT_LEN];
	uint16_t u16NearDmicTmpWIdx;	
	uint8_t u8NearDmicTmp_Buf_Idx;
	
	// Colsonic
	uint8_t bAlgEnable;
	uint16_t* pu16Near;
	uint8_t bNearReady;
	uint16_t* pu16Far;
    uint8_t bFarReady;
	uint8_t bDoAECReady;
	uint16_t u16DecodeTmp[COLSONIC_DAT_LEN];
	uint32_t u32Decode[2][COLSONIC_DAT_LEN];
	uint8_t u8Decode_Buf_Idx;
	uint16_t u16DecodeBufRIdx;
	uint16_t u16DecodeAvalCnt;  
		
	uint8_t status; // for debug
	uint8_t bAlgByPass;
// USB 
	UAC_PLAYBACK_T* psUacPlayback;
	UAC_RECODER_T* psUacRecord;
}DATA_MANAGER;

#endif //__UAC_HANDLER_H__

/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/


  
