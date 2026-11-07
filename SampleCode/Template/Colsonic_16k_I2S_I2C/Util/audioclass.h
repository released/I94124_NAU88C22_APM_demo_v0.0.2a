#ifndef __AUDIOCLASS_H__
#define __AUDIOCLASS_H__
 
#include "stdint.h" 
 
 
/* Define the vendor id and product id */
#define USBD_VID     0x0416
#define USBD_PID     0x1204

/* Define EP maximum packet size */
#define EP0_MAX_PKT_SIZE    8     
#define EP1_MAX_PKT_SIZE    EP0_MAX_PKT_SIZE

#define EP3_MAX_PKT_SIZE    (PLAY_RATE*PLAY_CHANNELS*PLAY_SUB_FRM/1000+32)	
#define EP4_MAX_PKT_SIZE    4  
#define EP5_MAX_PKT_SIZE    16  

#define EP2_MAX_PKT_SIZE    (REC_RATE*REC_CHANNELS*MIC_SUB_FRM/1000+32)			
#define EP6_MAX_PKT_SIZE    4	
#define EP7_MAX_PKT_SIZE    4			
#define EP8_MAX_PKT_SIZE    4	


#define SETUP_BUF_BASE      0
#define SETUP_BUF_LEN       8
#define EP0_BUF_BASE        (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP0_BUF_LEN         (EP0_MAX_PKT_SIZE)
#define EP1_BUF_BASE        (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP1_BUF_LEN         (EP1_MAX_PKT_SIZE)
#define EP2_BUF_BASE        (EP1_BUF_BASE + EP1_BUF_LEN)
#define EP2_BUF_LEN         (EP2_MAX_PKT_SIZE)
#define EP3_BUF_BASE        (EP2_BUF_BASE + EP2_BUF_LEN)
#define EP3_BUF_LEN         (EP3_MAX_PKT_SIZE)
#define EP4_BUF_BASE        (EP3_BUF_BASE + EP3_BUF_LEN)
#define EP4_BUF_LEN         (EP4_MAX_PKT_SIZE)
#define EP5_BUF_BASE        (EP4_BUF_BASE + EP4_BUF_LEN)
#define EP5_BUF_LEN         (EP5_MAX_PKT_SIZE)
#define EP6_BUF_BASE        (EP5_BUF_BASE + EP5_BUF_LEN)
#define EP6_BUF_LEN         (EP6_MAX_PKT_SIZE)
#define EP7_BUF_BASE        (EP6_BUF_BASE + EP6_BUF_LEN)
#define EP7_BUF_LEN         (EP7_MAX_PKT_SIZE)
#define EP8_BUF_BASE        (EP7_BUF_BASE + EP7_BUF_LEN)
#define EP8_BUF_LEN         (EP8_MAX_PKT_SIZE)


/* Define Descriptor information */
#define USBD_SELF_POWERED               0
#define USBD_REMOTE_WAKEUP              0
#define USBD_MAX_POWER                  100  /* The unit is in 2mA. ex: 50 * 2mA = 100mA */

#define LEN_CONFIG_AND_SUBORDINATE      (LEN_CONFIG+LEN_INTERFACE+LEN_ENDPOINT*2)


/*!<Define AUDIO Class Specific Request */
#define GET_CUR_VOL		0x81
#define GET_MIN_VOL		0x82
#define GET_MAX_VOL		0x83
#define GET_RES_VOL		0x84


/* Endpoint Control Selectors */
#define EP_CONTROL_UNDEFINED        0x0
#define SAMPLING_FREQ_CONTROL       0x1


/*-------------------------------------------------------------*/
void USB_EP_Init(void);
void AUDIO_ClassRequest(void);
void AUDIO_SetInterface(void);

/*-------------------------------------------------------------*/
void EP2_Handler(void);
void EP3_Handler(void);
void EP4_Handler(void);
void EP6_Handler(void);
void EP7_Handler(void);
void EP8_Handler(void);


#endif
