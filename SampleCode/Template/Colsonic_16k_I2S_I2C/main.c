#include <stdio.h>
#include "misc.h"
#include "colsonic_para_set.h"
#include "i2c_handler.h"

//----------------------------------------------------------------------------
//  MAIN function
//----------------------------------------------------------------------------
/*
 * Application entry: clocks -> APM -> codec control -> audio PDMA -> services.
 * PDMA interrupts move PCM blocks; this foreground loop runs the APM and
 * the UART parameter protocol. I2C commands are also dispatched here.
 */
int main(void)
{	
	SysClk_Init();
	
	Colsonic_Init();
	
	/*
		MCU to codec I2C
		I2C1 SCL : PD0
		I2C1 SDA : PD1
	*/
	Codec_Init();
	
	/*
		MCU to codec I2S
		I2S0_MCLK : PD2
		I2S0_LRCK : PD3
		I2S0_DI : PD4
		I2S0_DO : PD5
		I2S0_BCLK : PD6
	*/
	Audio_Ip_Init();
	
	/*
		MCU to SOC I2C 
		I2C0_SCL : PD14
		I2C0_SDA : PD15
	
		MCU to SOC I2S 
		SPI2_MISO : PA7
		SPI2_MOSI : PA8
		SPI2_SS : PA9
		SPI2_CLK : PA10
		SPI2I2S_MCLK : PA12
	*/
	Misc();

	printf("\r\n/**************************************************************/\r\n");
	printf("##MCU Chip I94100##\r\n");
	printf("##FW Ver V%2X.%2X\r\n",DSP_VER_MAJOR, DSP_VER_MINOR);
	printf("Build Date: %s, time: %s\r\n", __DATE__, __TIME__);
	printf("##SysClk HCLK== %d\r\n",CLK_GetHCLKFreq());
	printf("##SysClk CPU== %d\r\n",CLK_GetCPUFreq());
  	printf("##Start APP PASS##\r\n");

	while(1)
	{
		Colsonic_Work_Handle();
		Main_Console_Handle();
		I2C_Cmd_Handler();
	}
}

