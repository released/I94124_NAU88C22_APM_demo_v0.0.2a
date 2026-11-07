#include "i2c_handler.h"
#include "i2c_cmd.h"
#include "codec.h"

#define DEBUG
#if defined(DEBUG)
#include <stdio.h>
#endif

typedef struct _i2c_rec
{
	uint8_t u8Cmd;
	uint8_t au8Buf[33];
	uint8_t u8RecCnt;
}I2C_REC_T;

typedef struct _i2c_cmd
{
	uint8_t u8Cmd;	
	uint8_t* pu8Dat;
	uint8_t bAval;
}I2C_CMD_T;


I2C_REC_T s_i2c_rec;
I2C_CMD_T s_i2c_cmd = {0,0,0};

// global variables
/*
 * Start_Audio/Stop_Audio publish this state after completing the transition.
 * Boot auto-start uses the same path as SYS_CMD, so the first STOP works.
 * Volatile is required because the I2C0 ISR reads foreground updates.
 */
volatile BOOL g_bStreaming = AUDIO_PATH_STOP;
BOOL g_bAEC = AEC_NR_ON;
BOOL g_bMute = AUDIO_SPK_UNMUTE;
#if defined(CODEC_ALC_FUNC)
BOOL g_bALC = ALC_OFF;
#endif
BOOL g_bSpkBst = SPKBST_ENABLE;

enum
{
	idx_L_DAC    = 0,
	idx_R_DAC    = 1,
	idx_L_ADC    = 2,
	idx_R_ADC    = 3,
	idx_L_SPK    = 4,
  idx_R_SPK    = 5,
#if defined(CODEC_ALC_FUNC)
	idx_ALC_CTRL = 6,
#endif
	idx_OUT_CTRL,
	idx_L_PGA,
	idx_R_PGA,

	idx_CODEC_MAX ,
};

typedef struct {
	int id;
	int reg;
} codec_reg_info_t;

static const codec_reg_info_t codec_table[] = {
	{idx_L_DAC,			CODEC_RD_L_DAC_REG},
	{idx_R_DAC,			CODEC_RD_R_DAC_REG},
	{idx_L_ADC,			CODEC_RD_L_ADC_REG},
	{idx_R_ADC,			CODEC_RD_R_ADC_REG},
	{idx_L_SPK,			CODEC_RD_L_SPK_REG},
	{idx_R_SPK,			CODEC_RD_R_SPK_REG},
#if defined(CODEC_ALC_FUNC)
	{idx_ALC_CTRL,	CODEC_ALC_CTRL_REG},
#endif
	{idx_OUT_CTRL,	CODEC_OUT_CTRL_REG},
	{idx_L_PGA,			CODEC_RD_L_PGA_REG},
	{idx_R_PGA,			CODEC_RD_R_PGA_REG},
};


volatile uint8_t codec_reg_data[idx_CODEC_MAX][2] = {0};
uint8_t reg_addr = 0;
/*
 * Reply to the selected CMD with one byte. No multi-byte auto-increment.
 * Volume/gain reads use the periodically refreshed codec cache, not a fresh
 * I2C1 transaction. Unsupported commands (including 0x16/0x17) reply 0xFE.
 */
void reg_read_reply(I2C_T* i2c,uint8_t reg)
{
	uint8_t reply = 0;

	switch(reg)
	{
		case SYS_CMD:
			reply = g_bStreaming;
			break;

		case AEC_NR_CMD:
			reply = g_bAEC;
			break;

#if defined(CODEC_ALC_FUNC)
		case ALC_CMD:
			reply = g_bALC;
			break;
#endif

		case SPKBST_CMD:
			reply = g_bSpkBst;
			break;

		case DAC_VOLUME_CMD:
			reply = codec_reg_data[idx_L_DAC][1];
			break;

		case SPKOUT_VOLUME_MUTE_CMD:
			reply = g_bMute;
			break;

		case ADC_VOLUME_CMD:
			reply = codec_reg_data[idx_L_ADC][1];
			break;
		
		case DSP_FW_VER_MAJ_CMD:
			reply = DSP_VER_MAJOR;
			break;
		
		case DSP_FW_VER_MIN_CMD:
			reply = DSP_VER_MINOR;
			break;

		case PGA_GAIN_CMD:
			reply = codec_reg_data[idx_L_PGA][1] & PGA_GAIN_MAX;
			break;

		case SPKOUT_VOLUME_GAIN_CMD:
			reply = codec_reg_data[idx_L_SPK][1] & SPKOUT_VOLUME_GAIN_MAX;
			break;

		default:	// UNKNOWN command
			reply = 0xFE;
			break;
	}

#if defined(DEBUG)
	printf("\r\n%s:[reg]0x%02X,[reply]0x%02X\r\n",__FUNCTION__,reg,reply);
#endif

	I2C_SET_DATA(i2c, reply);
}

/*
 * I2C0 ISR state machine: first byte CMD, then LEN, then DATA bytes.
 * STOP/repeated START publishes a write only when LEN matches DATA count.
 * For a read, select CMD alone then repeated START into SLA+R.
 * Only one pending command is stored, pointing into the reusable RX buffer;
 * there is no queue, execution ACK, or busy handshake. Do not burst writes.
 * Implemented value writes need LEN=1; this parser does not validate the
 * minimum payload length for each command.
 */
void I2C_Cmd_Receiver(I2C_T* i2c)
{
	if(I2C_GET_TIMEOUT_FLAG(i2c))
        I2C_ClearTimeoutFlag(i2c);
	else
	{
	    switch(I2C_GET_STATUS(i2c))
        {
        /* SLA+W has been receive; ACK has been return */
        case 0x60:
			s_i2c_rec.u8Cmd = 0;
			s_i2c_rec.au8Buf[0] = 0xff;
		    s_i2c_rec.u8RecCnt = 0;		
            I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI | I2C_CTL_AA);
            break;
        // Slave Receive Arbitration Lost 
        case 0x68:			
			I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI | I2C_CTL_AA);
            break;		
		/* Previously address with own SLA address Data has been received; ACK has been returned*/
		case 0x80:
			if(s_i2c_rec.u8Cmd == 0) // first
			{
				s_i2c_rec.u8Cmd = I2C_GET_DATA(i2c);
				reg_addr = s_i2c_rec.u8Cmd;
			}
			else
			{
				s_i2c_rec.au8Buf[s_i2c_rec.u8RecCnt] = I2C_GET_DATA(i2c);
				s_i2c_rec.u8RecCnt++;
			}
			
			if(s_i2c_rec.u8RecCnt == 1) // get data length
				if(s_i2c_rec.au8Buf[0] >= 32){
					I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI); // illegal length NAK
					break;
				}
			if(s_i2c_rec.u8RecCnt >= 32) // over buffer
			{
				I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI); // NAK
				break;
			}
		     
			I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI | I2C_CTL_AA);
			break;
		/* Previously addressed with own SLA address; NOT ACK has been returned */
		case 0x88:
			s_i2c_rec.u8Cmd = 0;
			s_i2c_rec.u8RecCnt = 0;	
			I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI | I2C_CTL_AA);
			break;  
		/* A STOP or repeated START has been received while still addressed as Slave/Receiver*/ 
        case 0xA0:
			if(s_i2c_rec.u8Cmd != 0 && (s_i2c_rec.au8Buf[0] == (s_i2c_rec.u8RecCnt-1)))
			{				
				s_i2c_cmd.u8Cmd = s_i2c_rec.u8Cmd;
				s_i2c_cmd.pu8Dat = &s_i2c_rec.au8Buf[1];
				s_i2c_cmd.bAval = TRUE;
				s_i2c_rec.u8Cmd = 0;
			}
			else
			{
				s_i2c_rec.u8Cmd = 0;
				s_i2c_rec.u8RecCnt = 0;	
			}				
			I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI | I2C_CTL_AA);
            break;	
		
		// TODO
		// Slave transmit
        /* Own SLA+R has been receive; ACK has been return */
        case 0xA8:
			reg_read_reply(i2c, reg_addr);
			I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI | I2C_CTL_AA);
			reg_addr = 0;
			break;	
        // Address Transmit Arbitration Lost 
        case 0xB0:
			I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI | I2C_CTL_AA);
            break;	
		/* Data byte in I2CDAT has been transmitted ACK has been received */
		case 0xB8:
			I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI | I2C_CTL_AA);
			break;	
		/* Data byte in I2CDAT has been transmitted Not ACK has been received */
		case 0xC0:
			I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI | I2C_CTL_AA);
			break; 	
		/* Last data in I2CDAT has been transmitted ACK has been received */
        case 0xC8:
			I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI | I2C_CTL_AA);
            break;		
		default:
			I2C_SET_CONTROL_REG(i2c, I2C_CTL_SI | I2C_CTL_AA);
            break;
		}	
	}
}


extern void Start_Audio(void);
extern void Stop_Audio(void);
extern void Set_AEC_ByPass(uint8_t u8OnOff);
extern volatile uint8_t tmr3_flag;

/*
 * Foreground control path: refresh cached codec registers on the timer flag,
 * then execute one pending command. I2C0 ACK only acknowledges transport.
 * Codec register writes go out over the separate I2C1 master bus.
 * 0x13 only selects output packing; 0x15 mutes codec speakers, not I2S PCM.
 * Read/modify/write commands retain the original cache-byte expressions;
 * this documentation change does not correct or reinterpret register bits.
 */
void I2C_Cmd_Handler(void)
{
	if (tmr3_flag)
	{
		uint8_t i = 0;
		const int table_size = sizeof(codec_table) / sizeof(codec_table[0]);

		for(i = 0; i < table_size ; i++)
		{
			NAU88C22_Read(I2C1,(uint8_t*)&codec_reg_data[codec_table[i].id][0],codec_table[i].reg);
// #if defined(DEBUG)
// 			printf("Codec Reg[0x%02X]: 0x%02X\r\n", codec_table[i].reg, codec_reg_data[codec_table[i].id][1]);
// #endif
		}

		tmr3_flag = 0;
	}

	if(s_i2c_cmd.bAval)
	{
		uint8_t data_reg;
		uint16_t value;

		switch(s_i2c_cmd.u8Cmd)
		{
			case SYS_CMD:
				if(s_i2c_cmd.pu8Dat[0] == AUDIO_PATH_START)
				{
					if(g_bStreaming != AUDIO_PATH_START)
					{
						/* The lifecycle routine also publishes the reply state. */
						Start_Audio();
					}
				}
				else if(s_i2c_cmd.pu8Dat[0] == AUDIO_PATH_STOP)
				{
					if(g_bStreaming != AUDIO_PATH_STOP)
					{
						/* The lifecycle routine also publishes the reply state. */
						Stop_Audio();
					}
				}
				break;

			case AEC_NR_CMD:
				if(s_i2c_cmd.pu8Dat[0] == AEC_NR_ON)
				{
					if(g_bAEC != AEC_NR_ON)
					{
						Set_AEC_ByPass(FALSE);
						g_bAEC = AEC_NR_ON;
					}						
				}
				else if(s_i2c_cmd.pu8Dat[0] == AEC_NR_BYPASS)
				{
					if(g_bAEC != AEC_NR_BYPASS)
					{
						Set_AEC_ByPass(TRUE);
						g_bAEC = AEC_NR_BYPASS;
					}
				}
				break;

#if defined(CODEC_ALC_FUNC)
			case ALC_CMD:
				if(s_i2c_cmd.pu8Dat[0] == ALC_ON)
				{
					if(g_bALC != ALC_ON)
					{
						value = codec_reg_data[idx_ALC_CTRL][0] | 0x180;
						//value = 0x1B8;
						Set_NAU88C22_Reg(I2C1, CODEC_ALC_CTRL_REG, value);
						g_bALC = ALC_ON;
					}
				}
				else if(s_i2c_cmd.pu8Dat[0] == ALC_OFF)
				{
					if(g_bALC != ALC_OFF)
					{
						value = codec_reg_data[idx_ALC_CTRL][0] & 0x07F;
						//value = 0x38;
						Set_NAU88C22_Reg(I2C1, CODEC_ALC_CTRL_REG, value);
						g_bALC = ALC_OFF;
					}
				}
				break;
#endif

			case SPKBST_CMD:
				if(s_i2c_cmd.pu8Dat[0] == SPKBST_ENABLE)
				{
					if(g_bSpkBst != SPKBST_ENABLE)
					{
						value = codec_reg_data[idx_OUT_CTRL][0] | SPKBST_UPDATE_BIT;
						Set_NAU88C22_Reg(I2C1, CODEC_OUT_CTRL_REG, value);
						g_bSpkBst = SPKBST_ENABLE;
					}
				}
				else if(s_i2c_cmd.pu8Dat[0] == SPKBST_DISABLE)
				{
					if(g_bSpkBst != SPKBST_DISABLE)
					{
						value = codec_reg_data[idx_OUT_CTRL][0] & ~SPKBST_UPDATE_BIT;
						Set_NAU88C22_Reg(I2C1, CODEC_OUT_CTRL_REG, value);
						g_bSpkBst = SPKBST_DISABLE;
					}
				}
				break;

			case DAC_VOLUME_CMD:
				value = (s_i2c_cmd.pu8Dat[0] & DAC_VOLUME_MAX) | UPDATE_BIT;
				Set_NAU88C22_Reg(I2C1, CODEC_RD_L_DAC_REG, value);
				Set_NAU88C22_Reg(I2C1, CODEC_RD_R_DAC_REG, value);
			break;
			
			case SPKOUT_VOLUME_MUTE_CMD:
				if(s_i2c_cmd.pu8Dat[0] == AUDIO_SPK_UNMUTE)
				{
					if(g_bMute != AUDIO_SPK_UNMUTE)
					{
						value = codec_reg_data[idx_L_SPK][0] & ~AUDIO_SPK_MUTE_UPDATE_BIT;
						Set_NAU88C22_Reg(I2C1, CODEC_RD_L_SPK_REG, value);
						Set_NAU88C22_Reg(I2C1, CODEC_RD_R_SPK_REG, value);
						g_bMute = AUDIO_SPK_UNMUTE;
					}
				}					
				else if(s_i2c_cmd.pu8Dat[0] == AUDIO_SPK_MUTE)
				{
					if(g_bMute != AUDIO_SPK_MUTE)
					{
						value = codec_reg_data[idx_L_SPK][0] | AUDIO_SPK_MUTE_UPDATE_BIT;
						Set_NAU88C22_Reg(I2C1, CODEC_RD_L_SPK_REG, value);
						Set_NAU88C22_Reg(I2C1, CODEC_RD_R_SPK_REG, value);
						g_bMute = AUDIO_SPK_MUTE;
					}
				}					
				break;

			case ADC_VOLUME_CMD:
				value = (s_i2c_cmd.pu8Dat[0] & ADC_VOLUME_MAX) | UPDATE_BIT;
				Set_NAU88C22_Reg(I2C1, CODEC_RD_L_ADC_REG, value);
				Set_NAU88C22_Reg(I2C1, CODEC_RD_R_ADC_REG, value);
				break;

			case PGA_GAIN_CMD:
				value = (codec_reg_data[idx_L_PGA][0] & ~PGA_GAIN_MAX) |
					(s_i2c_cmd.pu8Dat[0] & PGA_GAIN_MAX) | UPDATE_BIT;
				Set_NAU88C22_Reg(I2C1, CODEC_RD_L_PGA_REG, value);
				Set_NAU88C22_Reg(I2C1, CODEC_RD_R_PGA_REG, value);
				break;

			case SPKOUT_VOLUME_GAIN_CMD:
				value = (codec_reg_data[idx_L_SPK][0] & ~SPKOUT_VOLUME_GAIN_MAX) |
					(s_i2c_cmd.pu8Dat[0] & SPKOUT_VOLUME_GAIN_MAX) | UPDATE_BIT;
				Set_NAU88C22_Reg(I2C1, CODEC_RD_L_SPK_REG, value);
				Set_NAU88C22_Reg(I2C1, CODEC_RD_R_SPK_REG, value);
				break;

			case SPKOUT_VOLUME_UP_CMD:
				if( (codec_reg_data[idx_L_SPK][1] & SPKOUT_VOLUME_GAIN_MAX) < SPKOUT_VOLUME_GAIN_MAX )
				{
					value = (codec_reg_data[idx_L_SPK][1] + 1) | UPDATE_BIT;
					Set_NAU88C22_Reg(I2C1, CODEC_RD_L_SPK_REG, value);
					Set_NAU88C22_Reg(I2C1, CODEC_RD_R_SPK_REG, value);
				}
				break;

			case SPKOUT_VOLUME_DOWN_CMD:
				if( (codec_reg_data[idx_L_SPK][1] & SPKOUT_VOLUME_GAIN_MAX) > 0 )
				{
					value = (codec_reg_data[idx_L_SPK][1] - 1) | UPDATE_BIT;
					Set_NAU88C22_Reg(I2C1, CODEC_RD_L_SPK_REG, value);
					Set_NAU88C22_Reg(I2C1, CODEC_RD_R_SPK_REG, value);
				}
				break;

			default:
#if defined(DEBUG)
					printf("%s: I2C does not support the cmd 0x%02X\r\n", __FUNCTION__, s_i2c_cmd.u8Cmd);
#endif
				break;
		}

		s_i2c_cmd.bAval = FALSE;
	}
}
