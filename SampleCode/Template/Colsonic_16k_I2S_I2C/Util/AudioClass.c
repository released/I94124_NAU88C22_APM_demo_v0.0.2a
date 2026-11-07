/*!<Includes */
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "usb_audio_const.h"
#include "audioclass.h"
#include "pdma_config.h"
#include "usb_audio_def.h"
#include "Vol.h"
#include "data_handler.h"

#if 0
#define DBG_PRINTF      printf
#else
#define DBG_PRINTF(...)
#endif

/*Get MAX or MIN value*/
#define GetMax( x1, x2 ) ( ( x1 ) > ( x2 ) ? ( x1 ) : ( x2 ) )
#define GetMin( x1, x2 ) ( ( x1 ) > ( x2 ) ? ( x2 ) : ( x1 ) )

// Extern Variable Declaration
extern DATA_MANAGER g_sDataManager;


/*--------------------------------------------------------------------------*/
// Extern Functions
extern void hw_play_reset(void);
extern void hw_play_start(void);
extern void hw_play_close(void);
extern void hw_rec_start(void);
extern void hw_rec_close(void);
//----------------------------------------------------------------------------
//  Global variables for Control Pipe
//----------------------------------------------------------------------------

/*--------------------------------------------------------------------------*/
void USBD_IRQHandler(void)
{
    uint32_t u32IntSts = USBD_GET_INT_FLAG();
    uint32_t u32State = USBD_GET_BUS_STATE();

    if(u32IntSts & USBD_INTSTS_FLDET)
    {
        // Floating detect
        USBD_CLR_INT_FLAG(USBD_INTSTS_FLDET);

        if(USBD_IS_ATTACHED())
        {
            // Enable GPB15(VBUS) pull down state to solute suspend event issue.
            GPIO_EnablePullState(PB,BIT15,GPIO_PUSEL_PULL_DOWN);
            /* USB Plug In */
            USBD_ENABLE_USB();
        }
        else
        {
            // Disable GPB15 pull down state.
            GPIO_DisablePullState(PB,BIT15);
            /* USB Un-plug */
            USBD_DISABLE_USB();
        }
    }

    if ( u32IntSts & USBD_INTSTS_SOFIF_Msk )
    {
        /* Clear event flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_SOFIF_Msk);
    }

    if(u32IntSts & USBD_INTSTS_BUS)
    {
        /* Clear event flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_BUS);

        if(u32State & USBD_STATE_USBRST) {
            /* Bus reset */
            USBD_ENABLE_USB();
            USBD_SwReset();
            DBG_PRINTF("Bus reset\n");
        }
        if(u32State & USBD_STATE_SUSPEND) {
            /* Enable USB but disable PHY */
            USBD_DISABLE_PHY();
            DBG_PRINTF("Suspend\n");
        }
        if(u32State & USBD_STATE_RESUME) {
            /* Enable USB and enable PHY */
            USBD_ENABLE_USB();
            DBG_PRINTF("Resume\n");
        }
    }

    if(u32IntSts & USBD_INTSTS_USB) {

        // EP events
        if(u32IntSts & USBD_INTSTS_EP0) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP0);
            // control IN
            USBD_CtrlIn();
        }

        if(u32IntSts & USBD_INTSTS_EP1) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP1);
            // control OUT
            USBD_CtrlOut();
        }

        if(u32IntSts & USBD_INTSTS_EP2)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP2);
            EP2_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP3)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP3);
            EP3_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP4)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP4);
            EP4_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP5)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP5);
        }

        if(u32IntSts & USBD_INTSTS_EP6) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP6);
        }

        if(u32IntSts & USBD_INTSTS_EP7) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP7);
        }

        if(u32IntSts & USBD_INTSTS_EP8) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP8);
        }

        // USB event
        if(u32IntSts & USBD_INTSTS_SETUP) {
            // Setup packet
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_SETUP);

            /* Clear the data IN/OUT ready flag of control end-points */
            USBD_STOP_TRANSACTION(EP0);
            USBD_STOP_TRANSACTION(EP1);

            USBD_ProcessSetupPacket();
        }
    }
}

void EP2_Handler(void)
{
    int i;
    uint8_t *pu8buf;
    uint16_t u16Byte_Count;
	

    if (g_sDataManager.psUacRecord->u8AudioMicState == UAC_BUSY_AUDIO_RECORD)
    {
        // This section calculate the data number in UAC_RingBuffer.
        // And set the next transfer size depends on buffer threshold.		
		if(g_sDataManager.psUacRecord->u16AvalCnt >= MIC_BUFF_UPPER_THRE)
		{
			u16Byte_Count = MIC_BASE + MIC_ADD;
		}
		else if(g_sDataManager.psUacRecord->u16AvalCnt <= MIC_BUFF_LOWER_THRE)
		{
			u16Byte_Count = MIC_BASE - MIC_SUB;
		}
		else
		{
			u16Byte_Count = MIC_BASE;
		}
		
        pu8buf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2));

		for ( i= 0 ; i < u16Byte_Count ; i++)
        {
            if(g_sDataManager.psUacRecord->u16AvalCnt == 0)
            {
                pu8buf[i] = 0x0;
            }
            else
            {
				pu8buf[i] = g_sDataManager.psUacRecord->au8USB_RingBuff[g_sDataManager.psUacRecord->u16UAC_Buff_ReadIndex];
				
                if (++g_sDataManager.psUacRecord->u16UAC_Buff_ReadIndex >= MIC_RING_BUFFER_LEN)
                    g_sDataManager.psUacRecord->u16UAC_Buff_ReadIndex = 0;
				
				g_sDataManager.psUacRecord->u16AvalCnt--;
            }
        }
		
        USBD_SET_PAYLOAD_LEN(EP2, u16Byte_Count);
    }
    else
    {
        USBD_SET_PAYLOAD_LEN(EP2, 0);
    }

    return;
}

void EP3_Handler(void)
{
    uint32_t i, tmp;
    uint16_t u16Outlen;
    uint32_t *pu32buf;
	uint8_t *pu8buf; 

	pu32buf = (uint32_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP3));
    u16Outlen = USBD_GET_PAYLOAD_LEN(EP3);
	u16Outlen = u16Outlen>>2;

    if (g_sDataManager.psUacPlayback->u8AudioSpeakerState == UAC_BUSY_AUDIO_SPEAK)
    {
		tmp = SPK_RING_BUFFER_LEN - g_sDataManager.psUacPlayback->u16AvalCnt;
		if(tmp >= u16Outlen)
			tmp = u16Outlen;
		
        for (i = 0; i < tmp; i++)
        {
            // Get data from End-Point buffer.
            g_sDataManager.psUacPlayback->au32USB_RingBuff[g_sDataManager.psUacPlayback->u16UAC_Buff_WriteIndex] = pu32buf[i];

            // Ring buffer write pointer turn around.
            if (++g_sDataManager.psUacPlayback->u16UAC_Buff_WriteIndex >= SPK_RING_BUFFER_LEN)
                g_sDataManager.psUacPlayback->u16UAC_Buff_WriteIndex = 0;
        }

		g_sDataManager.psUacPlayback->u16AvalCnt += tmp;
    }
    // Audio start. To setup Buffer Control.
    else if (g_sDataManager.psUacPlayback->u8AudioSpeakerState == UAC_START_AUDIO_SPEAK)
    {
        for (i = 0; i < u16Outlen; i++)
        {
            // Get data from End-Point buffer.
            g_sDataManager.psUacPlayback->au32USB_RingBuff[g_sDataManager.psUacPlayback->u16UAC_Buff_WriteIndex] = pu32buf[i];

            // Ring buffer write pointer turn around.
            if (++g_sDataManager.psUacPlayback->u16UAC_Buff_WriteIndex >= SPK_RING_BUFFER_LEN)
                g_sDataManager.psUacPlayback->u16UAC_Buff_WriteIndex = 0;
        }

		g_sDataManager.psUacPlayback->u16AvalCnt += u16Outlen;

		if(g_sDataManager.psUacPlayback->u16UAC_Buff_WriteIndex >= (SPK_RING_BUFFER_LEN>>1))
		{
			g_sDataManager.psUacPlayback->u8AudioSpeakerState = UAC_BUSY_AUDIO_SPEAK;
						
			hw_play_start();
		}
		
		// Set Payload to start EP4
        pu8buf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_SYNC));
        pu8buf[0] = (SPK_BUFFER_BASE >> 0) & 0xff;;
        pu8buf[1] = (SPK_BUFFER_BASE >> 8) & 0xff;;
        pu8buf[2] = (SPK_BUFFER_BASE >> 16) & 0xff;;
        USBD_SET_PAYLOAD_LEN(EP_SYNC, 3);		
    }

    /* Prepare for nex OUT packet */
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);
}

void EP4_Handler(void)
{
	uint8_t *pu8buf;
    uint32_t u32Temp;

    if (g_sDataManager.psUacPlayback->u8AudioSpeakerState == UAC_START_AUDIO_SPEAK)
    {
    }
	else if(g_sDataManager.psUacPlayback->u8AudioSpeakerState == UAC_BUSY_AUDIO_SPEAK)
	{				
		// Adjust the amount of data that EP3 transfer from host.
		// Ring buffer is close to overflow.
		if (g_sDataManager.psUacPlayback->u16AvalCnt > SPK_BUFF_UPPER_THRE)
		{
			u32Temp = SPK_BUFFER_BASE - SPK_BUFFER_SUB;
		}
		// Ring buffer is close to underflow.
		else if (g_sDataManager.psUacPlayback->u16AvalCnt < SPK_BUFF_LOWER_THRE)
		{
			u32Temp = SPK_BUFFER_BASE + SPK_BUFFER_ADD;
		}
		else
		{
			u32Temp = SPK_BUFFER_BASE;
		}

		pu8buf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP4));

		pu8buf[0] = (u32Temp >> 0) & 0xff;
        pu8buf[1] = (u32Temp >> 8) & 0xff;
		pu8buf[2] = (u32Temp >> 16) & 0xff;
		USBD_SET_PAYLOAD_LEN(EP4, 3);
	}
	else
	{
	}
}

void EP5_Handler(void)
{
	
}

void USB_EP_Init(void)
{
    uint8_t* pu8;
    uint8_t i;
    uint8_t *pSerial = (uint8_t*)__TIME__;

    /* Init setup packet buffer */
    /* Buffer range for setup packet -> [0 ~ 0x7] */
    USBD->STBUFSEG = SETUP_BUF_BASE;

    /*****************************************************/
    /* EP0 ==> control IN endpoint, address 0 */
    USBD_CONFIG_EP(EP0, USBD_CFG_CSTALL | USBD_CFG_EPMODE_IN | 0);
    /* Buffer range for EP0 */
    USBD_SET_EP_BUF_ADDR(EP0, EP0_BUF_BASE);

    /* EP1 ==> control OUT endpoint, address 0 */
    USBD_CONFIG_EP(EP1, USBD_CFG_CSTALL | USBD_CFG_EPMODE_OUT | 0);
    /* Buffer range for EP1 */
    USBD_SET_EP_BUF_ADDR(EP1, EP1_BUF_BASE);

    /*****************************************************/
    /* EP2 ==> Iso IN endpoint, address 2 */
    USBD_CONFIG_EP(EP2, USBD_CFG_EPMODE_IN | USBD_CFG_TYPE_ISO | EP2);
    /* Buffer range for EP2 */
    USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);

    /* EP3 ==> Iso Out endpoint, address 3 */
    USBD_CONFIG_EP(EP3, USBD_CFG_EPMODE_OUT | USBD_CFG_TYPE_ISO | EP3);
    /* Buffer range for EP3 */
    USBD_SET_EP_BUF_ADDR(EP3, EP3_BUF_BASE);

    /* trigger to receive OUT data */
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);

    /* EP4 ==> Interrupt IN endpoint number 4 */
    USBD_CONFIG_EP(EP4, USBD_CFG_EPMODE_IN | USBD_CFG_TYPE_ISO | EP4);
    /* Buffer range for EP4 */
    USBD_SET_EP_BUF_ADDR(EP4, EP4_BUF_BASE);

//    /* EP6 ==> Iso IN endpoint, address 6 */
//    USBD_CONFIG_EP(EP6, USBD_CFG_EPMODE_IN | USBD_CFG_TYPE_ISO | EP6);
//    /* Buffer range for EP6 */
//    USBD_SET_EP_BUF_ADDR(EP6, EP6_BUF_BASE);

//    /* EP7 ==> Iso IN endpoint, address 7 */
//    USBD_CONFIG_EP(EP7, USBD_CFG_EPMODE_IN | USBD_CFG_TYPE_ISO | EP7);
//    /* Buffer range for EP7 */
//    USBD_SET_EP_BUF_ADDR(EP7, EP7_BUF_BASE);

    /* EP8 ==> Iso IN endpoint, address 8 */
//    USBD_CONFIG_EP(EP8, USBD_CFG_EPMODE_IN | USBD_CFG_TYPE_ISO | EP8);
//    /* Buffer range for EP8 */
//    USBD_SET_EP_BUF_ADDR(EP8, EP8_BUF_BASE);

    pu8 = (uint8_t *)gsInfo.gu8StringDesc[3];

    for (i = 0; i < 8; i++)
        pu8[pu8[0] - 16 + i * 2] = pSerial[i];
}

void AUDIO_ClassRequest(void)
{
    uint8_t buf[8];
    uint32_t u32Temp;

    USBD_GetSetupPacket(buf);

    if((buf[0] & 0x7f) == 0x22) 	//bmRequestType= class, endpoint
    {
        if(buf[0] & 0x80)		// Device to host
        {
            switch(buf[1])
            {
            case UAC_GET_CUR:
            {
                if(  buf[3]==SAMPLING_FREQ_CONTROL && (buf[4]==EP_SPK))
                {
                    u32Temp = g_sDataManager.psUacPlayback->u32PlaySampleRate;
                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 0) = (uint8_t)u32Temp;
                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = (uint8_t)(u32Temp >> 8);
                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 2) = (uint8_t)(u32Temp >> 16);

                    /* Data stage */
                    USBD_SET_DATA1(EP0);
                    USBD_SET_PAYLOAD_LEN(EP0, 3);
                    printf("Get_CUR: %08X\n",g_sDataManager.psUacPlayback->u32PlaySampleRate);
                }
				else if(buf[3]==SAMPLING_FREQ_CONTROL && (buf[4]==(EP_INPUT|EP_REC)))
				{
					u32Temp = g_sDataManager.psUacRecord->u32RecRate;
                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 0) = (uint8_t)u32Temp;
                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = (uint8_t)(u32Temp >> 8);
                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 2) = (uint8_t)(u32Temp >> 16);

                    /* Data stage */
                    USBD_SET_DATA1(EP0);
                    USBD_SET_PAYLOAD_LEN(EP0, 3);
                    printf("Get_CUR: %08X\n",g_sDataManager.psUacRecord->u32RecRate);
				}
                else
                {
                    {
                        uint8_t i;
                        puts("error 1: ");
                        for(i=0; i<8; i++)
                            printf(" %2x", buf[i]);

                        printf("\n");
                    }
                    /* error endpoint or un identified Control*/
                    /* Setup error, stall the device */
                    USBD_SetStall(0);
                    printf("Get_CUR USBD_SetStall(0): %08X\n",g_sDataManager.psUacPlayback->u32PlaySampleRate);
                }

                // Trigger next Control Out DATA1 Transaction.
                /* Status stage */
                USBD_PrepareCtrlOut(0, 0);
                break;
            }

            default:
            {
                /* Setup error, stall the device */
                /*unidentify CONTROL*/
                USBD_SetStall(0);
            }
            }
        }
        else		// Host to device
        {
            switch(buf[1])
            {
            case UAC_SET_CUR:
            {
                if(buf[3]==SAMPLING_FREQ_CONTROL && (buf[4]==EP_SPK))
                {
                    USBD_PrepareCtrlOut((uint8_t *)&g_sDataManager.psUacPlayback->u32PlaySampleRate, buf[6]);

                    /* Status stage */
                    USBD_SET_DATA1(EP0);
                    USBD_SET_PAYLOAD_LEN(EP0, 0);
                }
				else if(buf[3]==SAMPLING_FREQ_CONTROL && (buf[4]==(EP_INPUT|EP_REC)))
				{
                    USBD_PrepareCtrlOut((uint8_t *)&g_sDataManager.psUacRecord->u32RecRate, buf[6]);

                    /* Status stage */
                    USBD_SET_DATA1(EP0);
                    USBD_SET_PAYLOAD_LEN(EP0, 0);					
				}
                else
                {
                    {
                        uint8_t i;
                        puts("error 2: ");
                        for(i=0; i<8; i++)
                            printf(" %2x", buf[i]);

                        printf("\n");
                    }
                    /* Setup error, stall the device */
                    /* Unidentify CONTROL*/
                    /* STALL control pipe */
                    USBD_SetStall(0);
                }

                break;
            }
            default:
            {
                /*unimplement CONTROL or wrong endpoint number*/
                /* Setup error, stall the device */
                USBD_SetStall(0);
            }
            }
        }

    }
    else		/*Feature unit control*/
    {
        if (buf[0] & 0x80)    // Device to host
        {

            switch(buf[1])
            {
            case UAC_GET_CUR:
            {
                switch(buf[3])
                {
                case MUTE_CONTROL:
                {
                    if(REC_FEATURE_UNITID == buf[5])
                    {
                        u32Temp = g_sDataManager.psUacRecord->usbd_RecMute;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                    }
                    else if(PLAY_FEATURE_UNITID == buf[5])
                    {
                        u32Temp = g_sDataManager.psUacPlayback->usbd_PlayMute;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                    }

                    /* Data stage */
                    USBD_SET_DATA1(EP0);
                    USBD_SET_PAYLOAD_LEN(EP0, 1);
                    break;
                }
                case VOLUME_CONTROL:
                {
                    if(REC_FEATURE_UNITID == buf[5])
                    {
						u32Temp = g_sDataManager.psUacRecord->usbd_RecVolume;
						M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
						u32Temp = g_sDataManager.psUacRecord->usbd_RecVolume >> 8;
						M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                        
                    }
                    else if(PLAY_FEATURE_UNITID == buf[5])
                    {
						u32Temp = g_sDataManager.psUacPlayback->usbd_PlayVolume;
						M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
						u32Temp = g_sDataManager.psUacPlayback->usbd_PlayVolume >> 8;
						M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;                        
                    }

                    /* Data stage */
                    USBD_SET_DATA1(EP0);
                    USBD_SET_PAYLOAD_LEN(EP0, 2);
                    break;
                }
                default:
                {
                    /* Setup error, stall the device */
                    USBD_SetStall(0);
                }
                }

                // Trigger next Control Out DATA1 Transaction.
                /* Status stage */
                USBD_PrepareCtrlOut(0, 0);
                break;
            }

            case UAC_GET_MIN:
            {
                switch(buf[3])
                {
                case VOLUME_CONTROL:
                {
                    if(REC_FEATURE_UNITID == buf[5])
                    {
                        u32Temp = g_sDataManager.psUacRecord->usbd_RecMinVolume;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                        u32Temp = g_sDataManager.psUacRecord->usbd_RecMinVolume >> 8;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                    }
                    else if(PLAY_FEATURE_UNITID == buf[5])
                    {
                        u32Temp = g_sDataManager.psUacPlayback->usbd_PlayMinVolume;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                        u32Temp = g_sDataManager.psUacPlayback->usbd_PlayMinVolume >> 8;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                    }
                    /* Data stage */
                    USBD_SET_DATA1(EP0);
                    USBD_SET_PAYLOAD_LEN(EP0, 2);
                    break;
                }
                default:
                    /* STALL control pipe */
                    USBD_SetStall(0);
                }
                // Trigger next Control Out DATA1 Transaction.
                /* Status stage */
                USBD_PrepareCtrlOut(0, 0);
                break;
            }

            case UAC_GET_MAX:
            {
                switch(buf[3])
                {
                case VOLUME_CONTROL:
                {
                    if(REC_FEATURE_UNITID == buf[5])
                    {
                        u32Temp = g_sDataManager.psUacRecord->usbd_RecMaxVolume;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                        u32Temp = g_sDataManager.psUacRecord->usbd_RecMaxVolume >> 8;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                    }
                    else if(PLAY_FEATURE_UNITID == buf[5])
                    {
                        u32Temp = g_sDataManager.psUacPlayback->usbd_PlayMaxVolume;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                        u32Temp = g_sDataManager.psUacPlayback->usbd_PlayMaxVolume >> 8;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                    }
                    /* Data stage */
                    USBD_SET_DATA1(EP0);
                    USBD_SET_PAYLOAD_LEN(EP0, 2);
                    break;
                }
                default:

                    /* STALL control pipe */
                    USBD_SetStall(0);
                }

                // Trigger next Control Out DATA1 Transaction.
                /* Status stage */
                USBD_PrepareCtrlOut(0, 0);
                break;
            }

            case UAC_GET_RES:
            {
                switch(buf[3])
                {
                case VOLUME_CONTROL:
                {
                    if(REC_FEATURE_UNITID == buf[5])
                    {
                        u32Temp = g_sDataManager.psUacRecord->usbd_RecResVolume;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                        u32Temp = g_sDataManager.psUacRecord->usbd_RecResVolume >> 8;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                    }
                    else if(PLAY_FEATURE_UNITID == buf[5])
                    {
                        u32Temp = g_sDataManager.psUacPlayback->usbd_PlayResVolume;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                        u32Temp = g_sDataManager.psUacPlayback->usbd_PlayResVolume >> 8;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) =u32Temp;
                    }
                    /* Data stage */
                    USBD_SET_DATA1(EP0);
                    USBD_SET_PAYLOAD_LEN(EP0, 2);
                    break;
                }
                default:
                    /* STALL control pipe */
                    USBD_SetStall(0);
                }
                // Trigger next Control Out DATA1 Transaction.
                /* Status stage */
                USBD_PrepareCtrlOut(0, 0);
                break;
            }

            default:
            {
                /* Setup error, stall the device */
                USBD_SetStall(0);
            }
            }
        }
        else
        {
            // Host to device
            switch(buf[1])
            {
            case UAC_SET_CUR:
            {
                switch(buf[3])
                {
                case MUTE_CONTROL:
                    if(REC_FEATURE_UNITID == buf[5])
                        USBD_PrepareCtrlOut((uint8_t *)&g_sDataManager.psUacRecord->usbd_RecMute, buf[6]);
                    else if(PLAY_FEATURE_UNITID == buf[5])
                    {
                        USBD_PrepareCtrlOut((uint8_t *)&g_sDataManager.psUacPlayback->usbd_PlayMute, buf[6]);
                    }
                    /* Status stage */
                    USBD_SET_DATA1(EP0);
                    USBD_SET_PAYLOAD_LEN(EP0, 0);
                    break;

                case VOLUME_CONTROL:
                    if(REC_FEATURE_UNITID == buf[5])
                    {
						/* Prepare the buffer for new record volume of right channel */
						USBD_PrepareCtrlOut((uint8_t *)&g_sDataManager.psUacRecord->usbd_RecVolume, buf[6]);                        
                    }
                    else if(PLAY_FEATURE_UNITID == buf[5])
                    {
						/* Prepare the buffer for new play volume of right channel */
						USBD_PrepareCtrlOut((uint8_t *)&g_sDataManager.psUacPlayback->usbd_PlayVolume, buf[6]);                        
                    }

                    /* Status stage */
                    USBD_SET_DATA1(EP0);
                    USBD_SET_PAYLOAD_LEN(EP0, 0);
                    break;

                default:
                    /* STALL control pipe */
                    USBD_SetStall(0);
                    break;
                }
                break;
            }
            // HID
            case HID_SET_REPORT:
            {
                if(buf[3] == 2)
                {
                    /* Request Type = Output */
                    USBD_SET_DATA1(EP1);
                    USBD_SET_PAYLOAD_LEN(EP1, buf[6]);

                    /* Status stage */
                    USBD_PrepareCtrlIn(0, 0);
                }
                break;
            }
            case HID_SET_IDLE:
            {
                /* Status stage */
                USBD_SET_DATA1(EP0);
                USBD_SET_PAYLOAD_LEN(EP0, 0);
                break;
            }
            case HID_SET_PROTOCOL:

            default:
            {
                /* Setup error, stall the device */
                USBD_SetStall(0);
                break;
            }
            }
        }
    }
}

/**
 * @brief       Set Interface standard request
 *
 * @param[in]   u32AltInterface Interface
 *
 * @return      None
 *
 * @details     This function is used to set UAC Class relative setting
 */
void AUDIO_SetInterface(void)
{
    uint8_t buf[8];
    uint8_t u32AltInterface;
    uint8_t g_usbd_UsbInterface;

    USBD_GetSetupPacket(buf);

    u32AltInterface = buf[2];
    g_usbd_UsbInterface = buf[4];
	
	if ( g_usbd_UsbInterface == REC_IFNUM )
    {
        if (u32AltInterface == 1)
        {
			g_sDataManager.psUacRecord->u16UAC_Buff_ReadIndex = 0;
            g_sDataManager.psUacRecord->u16UAC_Buff_WriteIndex = 0;
			g_sDataManager.psUacRecord->u16AvalCnt = 0;
			
            g_sDataManager.psUacRecord->u8AudioMicState = UAC_START_AUDIO_RECORD;

            USBD_SET_DATA1(EP_REC);
            USBD_SET_PAYLOAD_LEN(EP_REC, 0);
						
			hw_rec_start();
        }
        else
        {
            g_sDataManager.psUacRecord->u8AudioMicState = UAC_STOP_AUDIO_RECORD;
            
            USBD_SET_DATA1(EP_REC);
            USBD_SET_PAYLOAD_LEN(EP_REC, 0);
			
			hw_rec_close();		
        }	
    }
	
	if ( g_usbd_UsbInterface == PLAY_IFNUM )
	{
		if (u32AltInterface == 1)
		{
            g_sDataManager.psUacPlayback->u8AudioSpeakerState = UAC_START_AUDIO_SPEAK;
            g_sDataManager.psUacPlayback->u16UAC_Buff_ReadIndex = 0;
            g_sDataManager.psUacPlayback->u16UAC_Buff_WriteIndex = 0;
			g_sDataManager.psUacPlayback->u16AvalCnt = 0;
            			
			hw_play_reset();
		}
		else
		{
            g_sDataManager.psUacPlayback->u8AudioSpeakerState = UAC_STOP_AUDIO_SPEAK;

			hw_play_close();
		}
	}
}
