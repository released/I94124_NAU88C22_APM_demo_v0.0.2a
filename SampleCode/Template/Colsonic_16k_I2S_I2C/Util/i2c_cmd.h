#ifndef __I2C_CMD_H__
#define __I2C_CMD_H__

/*
 * External I2C0 protocol: write CMD, LEN, DATA; read one selected CMD byte.
 * Use LEN=1 for all value writes below; 0x16/0x17 ignore that data byte.
 * 0x19/0x1A are read-only; 0x1B is excluded unless CODEC_ALC_FUNC is defined.
 * 0x13 selects uplink packing (normal Decode/Near, bypass Near/Far).
 * It does not stop APM execution or independently disable noise suppression.
 * See ../../Colsonic_16k_I2S_I2C.md for the complete command matrix.
 */
#define UPDATE_BIT                (1 << 8)

//streaming on&off switch
#define SYS_CMD                   0x12
#define AUDIO_PATH_STOP           0
#define AUDIO_PATH_START          1

//AEC on&off switch
#define AEC_NR_CMD                0x13
#define AEC_NR_BYPASS             0
#define AEC_NR_ON                 1

// DAC index vlaue
#define DAC_VOLUME_CMD            0x14
#define DAC_VOLUME_MAX            0xFF

//speaker out mute on&off switch
#define SPKOUT_VOLUME_MUTE_CMD    0x15
#define AUDIO_SPK_MUTE_UPDATE_BIT (1 << 6)
#define AUDIO_SPK_UNMUTE          0
#define AUDIO_SPK_MUTE            1

#define SPKOUT_VOLUME_UP_CMD         0x16
#define SPKOUT_VOLUME_DOWN_CMD       0x17

// ADC index vlaue
#define ADC_VOLUME_CMD            0x18
#define ADC_VOLUME_MAX            0xFF

// MCU Major FW version
#define DSP_FW_VER_MAJ_CMD        0x19
// MCU Major FW version
#define DSP_FW_VER_MIN_CMD        0x1A

// ALC Control Enable/Disable
//#define CODEC_ALC_FUNC
#if defined(CODEC_ALC_FUNC)
#define ALC_CMD                   0x1B
#define ALC_OFF                   0
#define ALC_ON                    1
#endif

//speaker boost on&off switch
#define SPKBST_CMD                0x1C
#define SPKBST_DISABLE            0
#define SPKBST_ENABLE             1
#define SPKBST_UPDATE_BIT         (1 << 2)

// L/R input PGA gain, set/get codec register 0x2D/2E
#define PGA_GAIN_CMD              0x1D
#define PGA_GAIN_MAX              0x3F

//speaker out volume gain index value
#define SPKOUT_VOLUME_GAIN_CMD    0x1E
#define SPKOUT_VOLUME_GAIN_MAX    0x3F

// codec register
#define CODEC_RD_L_DAC_REG        0x0B
#define CODEC_RD_R_DAC_REG        0x0C
#define CODEC_RD_L_ADC_REG        0x0F
#define CODEC_RD_R_ADC_REG        0x10
#define CODEC_ALC_CTRL_REG        0x20
#define CODEC_RD_L_PGA_REG        0x2D
#define CODEC_RD_R_PGA_REG        0x2E
#define CODEC_OUT_CTRL_REG        0x31
#define CODEC_RD_L_SPK_REG        0x36
#define CODEC_RD_R_SPK_REG        0x37

#endif
