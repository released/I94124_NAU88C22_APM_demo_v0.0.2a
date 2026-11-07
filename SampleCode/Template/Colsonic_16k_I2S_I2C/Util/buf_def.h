#ifndef __BUF_DEF_H__
#define __BUF_DEF_H__



/*
 * At 16 kHz: 320 mono samples = one 20 ms APM frame.
 * One I2S PDMA row holds 32 uint32_t stereo words = 128 bytes = 2 ms.
 * Ten PDMA completions therefore assemble one Near/Far frame.
 */
#define COLSONIC_DAT_LEN    320 // 320, 1ch, 16kHz, 16bits
#define FRAME_LEN		 	32
#define FRAME_COUNT			10

#define I2S_TX_BUF_LEN	 (FRAME_LEN)
#define I2S_RX_BUF_LEN	 (FRAME_LEN)
#define DMIC_BUF_LEN	 (FRAME_LEN)


#endif
