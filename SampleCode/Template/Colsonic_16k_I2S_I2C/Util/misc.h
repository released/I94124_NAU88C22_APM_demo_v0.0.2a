#ifndef __MISC_H__
#define __MISC_H__


#include "Platform.h"


void hw_play_reset(void);
void hw_play_start(void);
void hw_play_close(void);
void hw_rec_start(void);
void hw_rec_close(void);

void SysClk_Init(void);
void Codec_Init(void);
void Audio_Ip_Init(void);
void Misc(void);

void Colsonic_Init(void);
void Colsonic_Work_Handle(void);

#endif 
