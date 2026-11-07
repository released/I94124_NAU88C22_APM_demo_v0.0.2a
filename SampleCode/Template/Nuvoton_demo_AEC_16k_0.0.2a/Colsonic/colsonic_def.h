#ifndef COLSONICDEF_H_
#define COLSONICDEF_H_

#include <stdio.h>
#include <stdlib.h>

/* Both transports use the USB baseline library, profile and module layout.
 * The previous I2C-only 75 KiB pool is too small for this selected layout.
 * Keep these values consistent with colsonic_def.c and the paired library.
 */
#define COLSONIC_BUF_LEN_MODE (148000)
#define COLSONIC_AEC_LEN (800)

extern unsigned long int u32CMDDataBegin;
extern unsigned long int u32CMDDataEnd;
extern volatile __attribute__((aligned(4))) unsigned char APM_MemPool[COLSONIC_BUF_LEN_MODE];
extern unsigned int colsonic_lib_buf_len;
extern unsigned int colsonic_lib_aec_len;

#endif
