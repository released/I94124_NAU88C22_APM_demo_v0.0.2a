
void EP2_Handler(void)
{
    int i;
    uint8_t *pu8buf;
    uint16_t u16Byte_Count;
	
    uint16_t u16Frames;
    uint16_t frameIndex;
    uint8_t  b0;
    uint8_t  b1;
    uint8_t  b2;
    uint8_t  b3;
    uint16_t rightSample;

    if (g_sUacRecord.u8AudioMicState == UAC_BUSY_AUDIO_RECORD)
    {
        // This section calculate the data number in UAC_RingBuffer.
        // And set the next transfer size depends on buffer threshold.		
		if(g_sUacRecord.u16AvalCnt >= MIC_BUFF_UPPER_THRE)
		{
			u16Byte_Count = MIC_BASE + MIC_ADD;
		}
		else if(g_sUacRecord.u16AvalCnt <= MIC_BUFF_LOWER_THRE)
		{
			u16Byte_Count = MIC_BASE - MIC_SUB;
		}
		else
		{
			u16Byte_Count = MIC_BASE;
		}
		
        pu8buf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2));

        #if 1

        /* If no data available, output pure silence */
        if (g_sUacRecord.u16AvalCnt == 0U)
        {
            for (i = 0U; i < u16Byte_Count; i++)
            {
                pu8buf[i] = 0U;
            }
        }
        else
        {
            /* Each stereo frame = 4 bytes (16-bit × 2 channels) */
            u16Frames = (uint16_t)(u16Byte_Count >> 2); /* u16Byte_Count / 4 */

            for (frameIndex = 0U; frameIndex < u16Frames; frameIndex++)
            {
                if (g_sUacRecord.u16AvalCnt >= 4U)
                {
                    /* Fetch one stereo frame (4 bytes) from record ring buffer */
                    b0 = g_sUacRecord.au8USB_RingBuff[g_sUacRecord.u16UAC_Buff_ReadIndex];
                    g_sUacRecord.u16UAC_Buff_ReadIndex++;
                    if (g_sUacRecord.u16UAC_Buff_ReadIndex >= MIC_RING_BUFFER_LEN)
                    {
                        g_sUacRecord.u16UAC_Buff_ReadIndex = 0U;
                    }

                    b1 = g_sUacRecord.au8USB_RingBuff[g_sUacRecord.u16UAC_Buff_ReadIndex];
                    g_sUacRecord.u16UAC_Buff_ReadIndex++;
                    if (g_sUacRecord.u16UAC_Buff_ReadIndex >= MIC_RING_BUFFER_LEN)
                    {
                        g_sUacRecord.u16UAC_Buff_ReadIndex = 0U;
                    }

                    b2 = g_sUacRecord.au8USB_RingBuff[g_sUacRecord.u16UAC_Buff_ReadIndex];
                    g_sUacRecord.u16UAC_Buff_ReadIndex++;
                    if (g_sUacRecord.u16UAC_Buff_ReadIndex >= MIC_RING_BUFFER_LEN)
                    {
                        g_sUacRecord.u16UAC_Buff_ReadIndex = 0U;
                    }

                    b3 = g_sUacRecord.au8USB_RingBuff[g_sUacRecord.u16UAC_Buff_ReadIndex];
                    g_sUacRecord.u16UAC_Buff_ReadIndex++;
                    if (g_sUacRecord.u16UAC_Buff_ReadIndex >= MIC_RING_BUFFER_LEN)
                    {
                        g_sUacRecord.u16UAC_Buff_ReadIndex = 0U;
                    }

                    g_sUacRecord.u16AvalCnt =
                        (uint16_t)(g_sUacRecord.u16AvalCnt - 4U);

                    //LOW 16 bits
                    rightSample = (uint16_t)((uint16_t)b0 |
                                              ((uint16_t)b1 << 8));
                    //HIGH 16 bits
                    // rightSample = (uint16_t)((uint16_t)b2 |
                    //                           ((uint16_t)b3 << 8));

                    /*
                     * Duplicate right channel to both left and right:
                     *   - Low  16 bits  = rightSample
                     *   - High 16 bits  = rightSample
                     *
                     * For USB host this means both L/R channels carry
                     * the same AEC-cleaned signal.
                     */
                    pu8buf[(uint16_t)(frameIndex * 4U) + 0U] =
                        (uint8_t)(rightSample & 0xFFU);
                    pu8buf[(uint16_t)(frameIndex * 4U) + 1U] =
                        (uint8_t)((rightSample >> 8) & 0xFFU);
                    pu8buf[(uint16_t)(frameIndex * 4U) + 2U] =
                        (uint8_t)(rightSample & 0xFFU);
                    pu8buf[(uint16_t)(frameIndex * 4U) + 3U] =
                        (uint8_t)((rightSample >> 8) & 0xFFU);
                }
                else
                {
                    /*
                     * Not enough data for a full frame in ring buffer.
                     * Output silence for this and remaining frames.
                     */
                    pu8buf[(uint16_t)(frameIndex * 4U) + 0U] = 0U;
                    pu8buf[(uint16_t)(frameIndex * 4U) + 1U] = 0U;
                    pu8buf[(uint16_t)(frameIndex * 4U) + 2U] = 0U;
                    pu8buf[(uint16_t)(frameIndex * 4U) + 3U] = 0U;
                }
            }

            /* Zero padding for any remaining bytes if u16Byte_Count is not multiple of 4 */
            for (i = (uint16_t)(u16Frames * 4U); i < u16Byte_Count; i++)
            {
                pu8buf[i] = 0U;
            }
        }        

        #else
		for ( i= 0 ; i < u16Byte_Count ; i++)
        {
            if(g_sUacRecord.u16AvalCnt == 0)
            {
                pu8buf[i] = 0x0;
            }
            else
            {
				pu8buf[i] = g_sUacRecord.au8USB_RingBuff[g_sUacRecord.u16UAC_Buff_ReadIndex];
				
                if (++g_sUacRecord.u16UAC_Buff_ReadIndex >= MIC_RING_BUFFER_LEN)
                    g_sUacRecord.u16UAC_Buff_ReadIndex = 0;
				
				g_sUacRecord.u16AvalCnt--;
            }
        }
		#endif

        USBD_SET_PAYLOAD_LEN(EP2, u16Byte_Count);
    }
    else
    {
        USBD_SET_PAYLOAD_LEN(EP2, 0);
    }

    return;
}