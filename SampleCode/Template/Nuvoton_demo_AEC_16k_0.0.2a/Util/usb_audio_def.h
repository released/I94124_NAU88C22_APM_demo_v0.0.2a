#ifndef __USB_AUDIO_DEF_H__
#define __USB_AUDIO_DEF_H__

#include "setting.h"

/*!<Define Audio information */
#define PLAY_RATE      		UAC_RATE
#define PLAY_BIT_RES		(UAC_DATA_BIT)
#define PLAY_SUB_FRM		(PLAY_BIT_RES>>3)
#define PLAY_CHANNELS    	2         		/* Number of channels. */

/* Microphone */
#define REC_RATE            UAC_RATE    /* The record sampling rate. Must be the same with PLAY_RATE */
#define MIC_BIT_RES			(UAC_DATA_BIT)
#define MIC_SUB_FRM			(MIC_BIT_RES>>3)
#define REC_CHANNELS    	2  


/**!<Define UAC interface */
#define TOTAL_INTERFACE_NUM  0x3

#define UAC_AC_IFNUM     0x0
#define REC_IFNUM	     0x1
#define PLAY_IFNUM       0x2
#define HID_IFNUM        0x6

#define EP_REC				 EP2
#define EP_REC_PKT_SIZE		 EP2_MAX_PKT_SIZE

#define EP_SPK				 EP3
#define EP_SPK_PKT_SIZE		 EP3_MAX_PKT_SIZE

#define EP_SYNC				 EP4
#define EP_SYNC_PKT_SIZE     EP4_MAX_PKT_SIZE


/* Define Descriptor information */
#if(PLAY_CHANNELS == 1)
    #define PLAY_CH_CFG     1
#endif
#if(PLAY_CHANNELS == 2)
    #define PLAY_CH_CFG     3
#endif

#if(REC_CHANNELS == 1)
    #define REC_CH_CFG     0x1
#endif
#if(REC_CHANNELS == 2)
    #define REC_CH_CFG     0x3
#endif
#if(REC_CHANNELS == 3)
    #define REC_CH_CFG     0x7
#endif
#if(REC_CHANNELS == 4)
    #define REC_CH_CFG     0xf
#endif


#define PLAY_RATE_LO    (PLAY_RATE & 0xFF)
#define PLAY_RATE_MD    ((PLAY_RATE >> 8) & 0xFF)
#define PLAY_RATE_HI    ((PLAY_RATE >> 16) & 0xFF)

 
#define PDMA2PLAY_BUFF_LEN     (PLAY_RATE*PLAY_CHANNELS/1000)  
#define REC2PDMA_BUFF_LEN      (REC_RATE*REC_CHANNELS/1000) 

#define SPK_RING_BUFFER_LEN    (15*PLAY_RATE/1000)
#define SPK_BUFF_UPPER_THRE    (10*PLAY_RATE/1000)   
#define SPK_BUFF_LOWER_THRE    (5*PLAY_RATE/1000)   

#define SPK_BUFFER_BASE			((PLAY_RATE/1000)<<14)
#define SPK_BUFFER_ADD			(SPK_BUFFER_BASE>>4)
#define SPK_BUFFER_SUB			(SPK_BUFFER_BASE>>4)

#define MIC_RING_BUFFER_LEN    (80*REC_RATE*REC_CHANNELS/1000)
#define MIC_BUFF_UPPER_THRE    (60*REC_RATE*REC_CHANNELS/1000)   
#define MIC_BUFF_LOWER_THRE    (40*REC_RATE*REC_CHANNELS/1000)   

#define MIC_BASE               (REC_RATE*REC_CHANNELS*MIC_SUB_FRM/1000)
#define MIC_ADD                (REC_CHANNELS*MIC_SUB_FRM*2)   
#define MIC_SUB                (REC_CHANNELS*MIC_SUB_FRM*2)   


#endif //__USB_AUDIO_DEF_H__

