#include "init.h"
#include "Trim.h"
#include "audioclass.h"
#include "init_def.h"


uint32_t SysClk_Initiate(void)
{
	// Unlock protected registers
	uint8_t u8Lock = SYS_Unlock();
	
	// 1. Configuration HIRC ====================================================
#if (SYSCLK_HIRC_CLK==SYSCLK_HIRC_CLK_49152)
	// Select trim HIRC Frequency to 49.152MHz
	CLK_SELECT_TRIM_HIRC(CLK_CLKSEL0_HIRCFSEL_49M);
#else
	// Select trim HIRC Frequency to 48MHz
	CLK_SELECT_TRIM_HIRC(CLK_CLKSEL0_HIRCFSEL_48M);
#endif
	// Enable HIRC 
	CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
	// Waiting for HIRC clock ready 
	CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

	// 2. Configuration HXT(if enable HXT) ======================================
//#if ((SYSCLK_HCLK_CLK == SYSCLK_HCLK_CLK_PLL_HXT) || (SYSCLK_HCLK_CLK == SYSCLK_HCLK_CLK_HXT))
    // Enable HXT 
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);
    // Waiting for HIRC clock ready 
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);
//#endif	

	// 3. Configuration HCLK(if source clock is PLL, enable PLL first) ==========
#if ((SYSCLK_HCLK_CLK==SYSCLK_HCLK_CLK_PLL_HIRC)||(SYSCLK_HCLK_CLK==SYSCLK_HCLK_CLK_PLL_HXT))
	// Enable PLL clock
	CLK_EnablePLL(((SYSCLK_HCLK_CLK==SYSCLK_HCLK_CLK_PLL_HIRC)?CLK_PLLCTL_PLLSRC_HIRC:CLK_PLLCTL_PLLSRC_HXT), (SYSCLK_PLL_CLK*1000));
	// Switch HCLK clock source to PLL
	CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_PLL, CLK_CLKDIV0_HCLK(1));
	
	#if (SYSCLK_PLL_CLK > 90000UL)
	CLK_SetPCLKDivider(CLK_PCLKDIV_PCLK0DIV2);
	CLK_SetPCLKDivider(CLK_PCLKDIV_PCLK1DIV2);
	#endif
	
#else
	// Set HCLK clock source to HIRC(SYSCLK_HCLK_CLK_HIRC) or HXT(SYSCLK_HCLK_CLK_HXT)
	CLK_SetHCLK(((SYSCLK_HCLK_CLK==SYSCLK_HCLK_CLK_HIRC)?CLK_CLKSEL0_HCLKSEL_HIRC:CLK_CLKSEL0_HCLKSEL_HXT), CLK_CLKDIV0_HCLK(1));
#endif

	// Update System Core Clock. 
	// User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CyclesPerUs automatically.
	SystemCoreClockUpdate();
	// Lock protected registers. 
	SYS_Lock(u8Lock);	

	return CLK_GetHCLKFreq();
}
//----------------------------------------------------------------------------
//  USB Initial function
//----------------------------------------------------------------------------
void USBD_Init(void)
{
	// MFP select 
	USB_MFP &= ~(USB_PIN_MASK);
	USB_MFP |= USB_PIN;
	
	// Select IP clock source
	CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL4_USBSEL_HIRC, CLK_CLKDIV0_USBD(1));   
	CLK_EnableModuleClock(USBD_MODULE);
	
	// USBD initial.
	USBD_Open(&gsInfo, AUDIO_ClassRequest, (SET_INTERFACE_REQ)AUDIO_SetInterface);
	
	USBD_SetConfigCallback((SET_CONFIG_CB) HIRC_AutoTrim_RefSof);

	// Endpoint configuration.
	USB_EP_Init();
	NVIC_EnableIRQ(USBD_IRQn);
}

//----------------------------------------------------------------------------
//  PDMA Initial function
//----------------------------------------------------------------------------
void PDMA_Init(void)
{
	// Enable PDMA engine clock.
	CLK_EnableModuleClock(PDMA_MODULE);
	// Reset PDMA module.
	SYS_ResetModule(PDMA_RST);

	NVIC_EnableIRQ(PDMA_IRQn);
}

//----------------------------------------------------------------------------
//  DPWM Initial function
//----------------------------------------------------------------------------
void DPWM_Init(uint32_t u32SampleRate, uint32_t u32Bit)
{
	volatile uint32_t n;

	DPWM_MFP &= ~(DPWM_PIN_MASK);
	DPWM_MFP |= DPWM_PIN;

	// Select DPWM clock
	CLK_SetModuleClock(DPWM_MODULE, CLK_CLKSEL2_DPWMSEL_PCLK0, NULL);
	
	// Enable DPWM clock
	CLK_EnableModuleClock(DPWM_MODULE);
	
	// DPWM IPReset
	SYS_ResetModule(DPWM_RST);

	// DPWM control
	DPWM_SET_CLKSET(DPWM, DPWM_CLKSET_512FS);

	//Set sample rate	
	{
		uint32_t u32Ratio, u32Temp, u32DPWMClkSrc, u32DPWMClk, u32ClkDiv, u32ZOHDiv;
		
		/* Get DPWM working clock ratio and clock divisor. */
		u32Ratio = (DPWM->CTL&DPWM_CLKSET_500FS)?125:128;
		u32DPWMClk = (DPWM->CTL&DPWM_CLKSET_500FS)?24000000:24576000;
		/* Get DPWM clock source frequency. */
		u32Temp = CLK->CLKSEL2 & CLK_CLKSEL2_DPWMSEL_Msk;
		switch(u32Temp)
		{
			case CLK_CLKSEL2_DPWMSEL_PCLK0:
				u32DPWMClkSrc = CLK_GetPCLK0Freq();
			break;
			case CLK_CLKSEL2_DPWMSEL_PLL:
				u32DPWMClkSrc = CLK_GetPLLClockFreq();
			break;
			case CLK_CLKSEL2_DPWMSEL_HXT:
				u32DPWMClkSrc = CLK_GetHXTFreq();
			break;
			case CLK_CLKSEL2_DPWMSEL_HIRC:
				u32DPWMClkSrc = __HIRC;
			break;
		}
		
		u32ClkDiv = u32DPWMClkSrc / u32DPWMClk - 1;
		u32ZOHDiv = u32DPWMClk / u32SampleRate / u32Ratio;
		
		DPWM_SET_CLOCKDIV(DPWM, u32ClkDiv);
		DPWM_SET_ZOHDIV(DPWM, u32ZOHDiv);
	}
		
	switch(u32Bit)
	{
		case 24:
			DPWM_SET_FIFODATAWIDTH(DPWM, DPWM_FIFO_DATAWIDTH_MSB24BITS);
			break;
		case 16:
			DPWM_SET_FIFODATAWIDTH(DPWM, DPWM_FIFO_DATAWIDTH_16BITS);
			break;
	}
	DPWM_DISABLE_BIQUAD(DPWM);
}

//----------------------------------------------------------------------------
//  UART0 Initial function
//----------------------------------------------------------------------------
void UART0_Init(void)
{
	// Enable UART module clock
	CLK_EnableModuleClock(UART0_MODULE);

	// Select UART clock source is HIRC
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));
	
	SET_UART_PIN
	
	// Reset UART module
	SYS_ResetModule(UART0_RST);

	// Configure UART0 and set UART0 baud rate
	UART_Open(UART0, 115200);
	
	// Enable Uart0 INT
    UART0->INTEN |=(1<<0); 
    UART0->TOUT |= (0x02<<0);
    NVIC_EnableIRQ(UART0_IRQn);
}


void CODEC_I2C1_Init(void)
{
    // Reset module.
    SYS_ResetModule(I2C1_RST);
    // Enable I2C0 module clock.
    CLK_EnableModuleClock(I2C1_MODULE);
    // Open I2C module and set bus clock.
    I2C_Open(I2C1, 100000);

	I2C1_MFP = (I2C1_MFP & ~I2C1_PIN_MASK)|I2C1_PIN;
	
    I2C_EnableInt(I2C1);
	NVIC_ClearPendingIRQ(I2C1_IRQn);
    NVIC_EnableIRQ(I2C1_IRQn);	
}

//----------------------------------------------------------------------------
//  I2S Initial function
//----------------------------------------------------------------------------
void CODEC_I2S_Init_85L40_2CH(uint32_t u32SampleRate, uint32_t u32Bit, uint32_t u32mclk)
{
    // Set I2S MFP
    I2S_MFP = (I2S_MFP&~I2S_MASTER_PIN_MASK)|I2S_MASTER_PIN;
    // Enable I2S0 clock.
    CLK_EnableModuleClock(I2S0_MODULE);
    // Select I2S0 clock.
    CLK_SetModuleClock(I2S0_MODULE, CLK_CLKSEL3_I2S0SEL_PCLK0, NULL);
    // I2S IPReset.
    SYS_ResetModule(I2S0_RST);

    // Open I2S0 hardware IP
    if(u32Bit == 32)
	{
		I2S_Open(I2S0, I2S_MASTER, u32SampleRate, I2S_DATABIT_32, I2S_TDMCHNUM_2CH, I2S_STEREO, I2S_FORMAT_I2S);
		I2S_SET_CHWIDTH(I2S0, I2S_CHWIDTH_32);
	}
	else if(u32Bit == 24)
	{
		I2S_Open(I2S0, I2S_MASTER, u32SampleRate, I2S_DATABIT_24, I2S_TDMCHNUM_2CH, I2S_STEREO, I2S_FORMAT_I2S);
		I2S_SET_CHWIDTH(I2S0, I2S_CHWIDTH_32);
	}
	else
	{
		I2S_Open(I2S0, I2S_MASTER, u32SampleRate, I2S_DATABIT_16, I2S_TDMCHNUM_2CH, I2S_STEREO, I2S_FORMAT_I2S);
		I2S_SET_CHWIDTH(I2S0, I2S_CHWIDTH_16);		
	}
    // I2S0 Configuration
    I2S_SET_PCMSYNC(I2S0, I2S_PCMSYNC_BCLK);
    I2S_SET_MONO_RX_CHANNEL(I2S0, I2S_MONO_RX_RIGHT);
    I2S_SET_STEREOORDER(I2S0, I2S_ORDER_EVENLOW);
	
    I2S_EnableMCLK(I2S0, u32mclk);

	I2S_CLR_TX_FIFO(I2S0);
	I2S_CLR_RX_FIFO(I2S0);
	
    // Enable I2S
    I2S_ENABLE(I2S0);
}

void CODEC_I2S_Init_8822(uint32_t u32SampleRate, uint32_t u32Bit, uint32_t u32mclk)
{
    // Set I2S MFP
    I2S_MFP = (I2S_MFP&~I2S_MASTER_PIN_MASK)|I2S_MASTER_PIN;
    // Enable I2S0 clock.
    CLK_EnableModuleClock(I2S0_MODULE);
    // Select I2S0 clock.
    CLK_SetModuleClock(I2S0_MODULE, CLK_CLKSEL3_I2S0SEL_PCLK0, NULL);
    // I2S IPReset.
    SYS_ResetModule(I2S0_RST);

    // Open I2S0 hardware IP
    if(u32Bit == 32)
	{
		I2S_Open(I2S0, I2S_MASTER, u32SampleRate, I2S_DATABIT_32, I2S_TDMCHNUM_2CH, I2S_STEREO, I2S_FORMAT_I2S);
		I2S_SET_CHWIDTH(I2S0, I2S_CHWIDTH_32);
	}
	else if(u32Bit == 24)
	{
		I2S_Open(I2S0, I2S_MASTER, u32SampleRate, I2S_DATABIT_24, I2S_TDMCHNUM_2CH, I2S_STEREO, I2S_FORMAT_I2S);
		I2S_SET_CHWIDTH(I2S0, I2S_CHWIDTH_32);
	}
	else
	{
		I2S_Open(I2S0, I2S_MASTER, u32SampleRate, I2S_DATABIT_16, I2S_TDMCHNUM_2CH, I2S_STEREO, I2S_FORMAT_I2S);
		I2S_SET_CHWIDTH(I2S0, I2S_CHWIDTH_16);		
	}
    // I2S0 Configuration
    I2S_SET_PCMSYNC(I2S0, I2S_PCMSYNC_BCLK);
    I2S_SET_MONO_RX_CHANNEL(I2S0, I2S_MONO_RX_RIGHT);
    I2S_SET_STEREOORDER(I2S0, I2S_ORDER_EVENLOW);
	
    I2S_EnableMCLK(I2S0, u32mclk);

	I2S_CLR_TX_FIFO(I2S0);
	I2S_CLR_RX_FIFO(I2S0);
	
    // Enable I2S
    I2S_ENABLE(I2S0);
}
//----------------------------------------------------------------------------
//  DMIC Initial function
//----------------------------------------------------------------------------
void DMIC_Init(uint32_t u32SampleRate)
{
	DMIC_MFP &= ~(DMIC_PIN_MASK);
	DMIC_MFP |= (DMIC_PIN);
	
	// DMIC_CLK Select.
	CLK_SetModuleClock(DMIC_MODULE, CLK_CLKSEL2_DMICSEL_HIRC, NULL);

	// Enable DMIC clock.
	CLK_EnableModuleClock(DMIC_MODULE);
	
	// DPWM IP Reset.
	SYS_ResetModule(DMIC_RST);
	
	{
		uint16_t u16K, u8pdiv, u8mdiv, n;
		uint32_t u32SourceClock,u32BusClock,u32MainClock;

		// Get DMIC clock source.
		switch(CLK->CLKSEL2&CLK_CLKSEL2_DMICSEL_Msk)
		{
			case CLK_CLKSEL2_DMICSEL_PCLK1:  u32SourceClock = CLK_GetPCLK1Freq();      break;
			case CLK_CLKSEL2_DMICSEL_PLL:    u32SourceClock = CLK_GetPLLClockFreq();   break;
			case CLK_CLKSEL2_DMICSEL_HXT:    u32SourceClock = CLK_GetHXTFreq();        break;
			case CLK_CLKSEL2_DMICSEL_HIRC:   u32SourceClock = __HIRC;                  break;
		}

		if(u32SourceClock%(256*u32SampleRate) != 0)
			u16K = 500;
		else
			u16K = 256;
		
		u32MainClock = u32SampleRate*u16K;
		u8pdiv = (u32SourceClock/u32MainClock)-1;
		
		if(u16K == 500)
		{
			for(n=1;n<=2;n++)
			{
				u32BusClock = u32SampleRate*(50*n);
				u8mdiv = (u32MainClock/u32BusClock)-1;
				if(u8mdiv<64 && u8mdiv!=0)
					break;
			}
			DMIC_ENABLE_DOWMSAMPLE(DMIC, DMIC_CTL_DOWNSAMPLE_100_50);
		}
		else
		{
			for(n=0;n<4;n++)
			{
				u32BusClock = u32SampleRate*(32*(1<<n));
				if((u32MainClock%u32BusClock) == 0)
				{
					u8mdiv = (u32MainClock/u32BusClock)-1;
					if(u8mdiv<64 && u8mdiv!=0)
						break;
				}
			}
			DMIC_ENABLE_DOWMSAMPLE(DMIC, (n)<<DMIC_CTL_OSR_Pos);
		}
		DMIC->DIV = (DMIC->DIV & ~(DMIC_DIV_MCLKDIV_Msk|DMIC_DIV_PCLKDIV_Msk)) | 
					(u8mdiv<<DMIC_DIV_MCLKDIV_Pos)|(u8pdiv<<DMIC_DIV_PCLKDIV_Pos);
		
	}
	
	// Set data latch 
	DMIC_SET_LATCHDATA(DMIC, DMIC_CTL_LATCHDATA_CH23R|DMIC_CTL_LATCHDATA_CH01R);
	// FIFO threshold

	DMIC_DISABLE_CHANNEL(DMIC, DMIC_CTL_CH0|DMIC_CTL_CH1|DMIC_CTL_CH2|DMIC_CTL_CH3);
    
	DMIC_ENABLE_CHANNEL(DMIC, DMIC_CTL_CH1);	
}

void TMR0_Init(void)
{
	// Timer Initiate for periodic HIRC Auto Trim.
	/* Enable peripheral clock */
	CLK_EnableModuleClock(TMR0_MODULE);
	/* Peripheral clock source */
	CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);
	
	/* Open Timer0 in periodic mode, enable interrupt and 1 interrupt tick per second */
	TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 1);
	TIMER_EnableInt(TIMER0);
	
	/* Enable Timer0 ~ Timer3 NVIC */
	NVIC_EnableIRQ(TMR0_IRQn);
	
	/* Start Timer0 ~ Timer3 counting */
	TIMER_Start(TIMER0);
}

void CODEC_SPI1_I2S_Init(uint32_t u32SampleRate, uint32_t u32Bit, uint32_t u32mclk)
{
    CLK_SetModuleClock(SPI1_MODULE, CLK_CLKSEL2_SPI1SEL_HIRC, MODULE_NoMsk);
    CLK_EnableModuleClock(SPI1_MODULE);
	
	SET_SPI1_I2S_PIN
	
    if(u32Bit == 32)
	{
		SPI_I2SOpen(SPI1, SPI_I2SMASTER, u32SampleRate, SPI_I2SDATABIT_32, SPI_I2SSTEREO, SPI_I2SFORMAT_I2S);
	}
	else if(u32Bit == 24)
	{
		SPI_I2SOpen(SPI1, SPI_I2SMASTER, u32SampleRate, SPI_I2SDATABIT_24, SPI_I2SSTEREO, SPI_I2SFORMAT_I2S);				
	}
	else
	{
		SPI_I2SOpen(SPI1, SPI_I2SMASTER, u32SampleRate, SPI_I2SDATABIT_16, SPI_I2SSTEREO, SPI_I2SFORMAT_I2S);		
	}
	
	SPI_I2S_RST_TX_FIFO(SPI1);
	SPI_I2S_RST_RX_FIFO(SPI1);	
	
    SPI_I2SEnableMCLK(SPI1, u32mclk);

}

void SPI1_I2S_Master_Init(uint32_t u32SampleRate)
{
	/* Select PCLK1 as the clock source of SPI1 */
	CLK_SetModuleClock(SPI1_MODULE, CLK_CLKSEL2_SPI1SEL_HIRC, MODULE_NoMsk);

	/* Enable peripheral clock */
	CLK_EnableModuleClock(SPI1_MODULE);

	/* Set SPI1 multi-function pins */
	SET_SPI1_I2S_PIN		
	
	SPI_I2SOpen(SPI1, SPI_I2SMASTER, u32SampleRate, SPI_I2SDATABIT_16, SPI_I2SSTEREO, SPI_I2SFORMAT_I2S);
	
	SPI_I2S_RST_TX_FIFO(SPI1);
	SPI_I2S_RST_RX_FIFO(SPI1);	
}
