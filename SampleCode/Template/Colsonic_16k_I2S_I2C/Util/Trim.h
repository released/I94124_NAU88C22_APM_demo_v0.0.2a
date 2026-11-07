#ifndef __TRIM_H__
#define __TRIM_H__

#include "Platform.h"

void IRC_IRQHandler(void);
void HIRC_AutoTrim_Init(void);
void HIRC_AutoTrim_RefSof(void);

#endif