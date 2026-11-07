/****************************************************************
 *                                                              *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved.  *
 *                                                              *
 ****************************************************************/

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#define	__NUVOTON__						/* Nuvoton platform */

#include "I94100.h"		        /* H/W SFR definition */
#include "NVTTypes.h"				  /* Nuvoton type definition */

// Audio I94124 MCU FW version
#define DSP_VER_MAJOR	0x00
#define DSP_VER_MINOR	0x01

// Audio process unit pcm bits
#define APU_PCM_BITS  (16)
// Audio process unit pcm bits
#define ADC_PCM_BITS  (16)

#endif /* __PLATFORM_H__ */
