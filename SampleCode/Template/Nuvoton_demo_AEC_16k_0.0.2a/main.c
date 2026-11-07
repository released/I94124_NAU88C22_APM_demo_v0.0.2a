#include "misc.h"
#include "colsonic_para_set.h"

//----------------------------------------------------------------------------
//  MAIN function
//----------------------------------------------------------------------------
/*
 * Application entry: clocks -> APM -> codec control -> audio PDMA -> services.
 * PDMA interrupts move PCM blocks; this foreground loop runs the APM and
 * the UART parameter protocol.
 * USB streaming is selected by the host through alternate interfaces.
 */
int main(void)
{	
	SysClk_Init();
	
	Colsonic_Init();
	
	Codec_Init();
	
	Audio_Ip_Init();
	
	Misc();

	
	while(1)
	{
		Colsonic_Work_Handle();
		Main_Console_Handle();
	}
}

