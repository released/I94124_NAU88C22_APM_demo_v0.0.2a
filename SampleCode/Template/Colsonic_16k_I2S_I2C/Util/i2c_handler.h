#ifndef __I2C_HANDLER_H__
#define __I2C_HANDLER_H__

#include "Platform.h"

/* Updated by audio lifecycle routines; read by the I2C0 reply ISR.
 * This reports the completed software transition, not DMA health. */
extern volatile BOOL g_bStreaming;

void I2C_Cmd_Receiver(I2C_T* i2c);
void I2C_Cmd_Handler(void);

#endif
