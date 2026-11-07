#ifndef __INIT_DEF_H__
#define __INIT_DEF_H__

#include "constant.h"
#include "setting.h"

/**
  * @brief   Provide user selections of HIRC's frequency.
  * @details 
  * SYSCLK_HIRC_CLK_49152 : HIRC's frequency is 49.152 MHz.
  * SYSCLK_HIRC_CLK_48000 : HIRC's frequency is 48 MHz.
  */
#define SYSCLK_HIRC_CLK   (SYSCLK_HIRC_CLK_48000) 

/**
  * @brief   Provide user selections of HCLK's clock source.
  * @details 
  * SYSCLK_HCLK_CLK_HIRC     : HCLK's clock source is from HIRC.
  * SYSCLK_HCLK_CLK_HXT      : HCLK's clock source is from HXT.
  * SYSCLK_HCLK_CLK_PLL_HIRC : HCLK's clock source is from PLL, and PLL's clock source is from HIRC.
  * SYSCLK_HCLK_CLK_PLL_HXT  : HCLK's clock source is from PLL, and PLL's clock source is from HXT.
  */	
#define SYSCLK_HCLK_CLK   (SYSCLK_HCLK_CLK_PLL_HXT)

/**
  * @brief   Provide user config the PLL frequency if HCLK's clock source is from PLL.
  * @details 
  * (1) SYSCLK_PLL_CLK must be in the range of 50000~200000 (KHz).
  * (2) SYSCLK_PLL_CLK's unit is KHz.
  * (3) When the PLL's source clock is from 12.288MHz(HXT) or 49.152MHz(HIRC), 
  *     SYSCLK_PLL_CLK must be multiple of '1024'.
  * (4) When the PLL's source clock is from 12MHz(HXT) or 48MHz(HIRC), 
  *     SYSCLK_PLL_CLK must be multiple of '1000'.
  */	
#define SYSCLK_PLL_CLK   (196608UL)	


// Compile check.
#if ((SYSCLK_HCLK_CLK==SYSCLK_HCLK_CLK_HXT)||(SYSCLK_HCLK_CLK==SYSCLK_HCLK_CLK_PLL_HXT))
	#if (SYSCLK_HCLK_CLK==SYSCLK_HCLK_CLK_PLL_HXT)
		#if (((SYSCLK_PLL_CLK%1000)!=0)&&(((__HXT/1000)%1024)!=0))
			#error "Please check 'SYSCLK_PLL_CLK' must be multiple of '1000' depending on current HXT frequency."
		#elif (((SYSCLK_PLL_CLK%1024)!=0)&&(((__HXT/1000)%1024)==0))
			#error "Please check 'SYSCLK_PLL_CLK' must be multiple of '1024' depending on current HXT frequency."
		#endif
	#endif
#elif (SYSCLK_HCLK_CLK==SYSCLK_HCLK_CLK_PLL_HIRC)
	#if ((SYSCLK_HIRC_CLK==SYSCLK_HIRC_CLK_49152)&&((SYSCLK_PLL_CLK%1024)!=0))
		#error "Please check 'SYSCLK_PLL_CLK' must be multiple of '1024' because current HIRC clock is 49.152MHz."
	#elif ((SYSCLK_HIRC_CLK==SYSCLK_HIRC_CLK_48000)&&((SYSCLK_PLL_CLK%1000)!=0))
		#error "Please check 'SYSCLK_PLL_CLK' must be multiple of '1000' because current HIRC clock is 48MHz."
	#endif
#endif


#define DPWM_MFP  			   SYS->GPC_MFPH
#define DPWM_PIN_MASK		   (SYS_GPC_MFPH_PC10MFP_Msk | \
								SYS_GPC_MFPH_PC11MFP_Msk | \
								SYS_GPC_MFPH_PC12MFP_Msk | \
								SYS_GPC_MFPH_PC13MFP_Msk)
#define DPWM_PIN			   (SYS_GPC_MFPH_PC10MFP_DPWM_RN | \
								SYS_GPC_MFPH_PC11MFP_DPWM_RP | \
								SYS_GPC_MFPH_PC12MFP_DPWM_LN | \
								SYS_GPC_MFPH_PC13MFP_DPWM_LP) 

#if defined(NT_I941A)
#define I2C1_MFP			    SYS->GPD_MFPH
#define I2C1_PIN_MASK          (SYS_GPD_MFPH_PD14MFP_Msk|SYS_GPD_MFPH_PD15MFP_Msk)
#define I2C1_PIN               (SYS_GPD_MFPH_PD14MFP_I2C1_SCL|SYS_GPD_MFPH_PD15MFP_I2C1_SDA)
#endif
#if defined(NT_I941)
#define I2C1_MFP 		    	SYS->GPD_MFPL
#define I2C1_PIN_MASK          (SYS_GPD_MFPL_PD0MFP_Msk|SYS_GPD_MFPL_PD1MFP_Msk)
#define I2C1_PIN               (SYS_GPD_MFPL_PD0MFP_I2C1_SCL|SYS_GPD_MFPL_PD1MFP_I2C1_SDA)
#endif

#define DMIC_MFP  			SYS->GPA_MFPL
#define DMIC_PIN_MASK 	   (SYS_GPA_MFPL_PA0MFP_Msk | \
								SYS_GPA_MFPL_PA1MFP_Msk | \
								SYS_GPA_MFPL_PA2MFP_Msk | \
								SYS_GPA_MFPL_PA3MFP_Msk)
#define DMIC_PIN			   (SYS_GPA_MFPL_PA0MFP_DMIC_DAT0 | \
								SYS_GPA_MFPL_PA1MFP_DMIC_CLK0 | \
								SYS_GPA_MFPL_PA2MFP_DMIC_DAT1 | \
								SYS_GPA_MFPL_PA3MFP_DMIC_CLK1)							

//#define DMIC_MFP              SYS->GPD_MFPL
//#define DMIC_PIN_MASK 	   (SYS_GPD_MFPL_PD3MFP_Msk | \
//								SYS_GPD_MFPL_PD4MFP_Msk | \
//								SYS_GPD_MFPL_PD5MFP_Msk | \
//								SYS_GPD_MFPL_PD6MFP_Msk)
//#define DMIC_PIN			   (SYS_GPD_MFPL_PD3MFP_DMIC_CLK1 | \
//								SYS_GPD_MFPL_PD4MFP_DMIC_DAT1 | \
//								SYS_GPD_MFPL_PD5MFP_DMIC_CLK0 | \
//								SYS_GPD_MFPL_PD6MFP_DMIC_DAT0)							

#define I2S_MFP                SYS->GPD_MFPL
#define I2S_MASTER_PIN_MASK    (SYS_GPD_MFPL_PD2MFP_Msk| \
								SYS_GPD_MFPL_PD3MFP_Msk| \
								SYS_GPD_MFPL_PD4MFP_Msk| \
								SYS_GPD_MFPL_PD5MFP_Msk| \
								SYS_GPD_MFPL_PD6MFP_Msk)
#define I2S_MASTER_PIN         (SYS_GPD_MFPL_PD2MFP_I2S0_MCLK| \
								SYS_GPD_MFPL_PD3MFP_I2S0_LRCK| \
								SYS_GPD_MFPL_PD4MFP_I2S0_DI| \
								SYS_GPD_MFPL_PD5MFP_I2S0_DO| \
								SYS_GPD_MFPL_PD6MFP_I2S0_BCLK)

#define SET_SPI1_I2S_PIN		PC->SMTEN = 0x1f; \
								(SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC0MFP_Msk | \
												   SYS_GPC_MFPL_PC1MFP_Msk |  \
												   SYS_GPC_MFPL_PC2MFP_Msk |  \
												   SYS_GPC_MFPL_PC3MFP_Msk |  \
												   SYS_GPC_MFPL_PC4MFP_Msk)); \
								(SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC0MFP_SPI1_MOSI | \
												   SYS_GPC_MFPL_PC1MFP_SPI1_MISO | \
												   SYS_GPC_MFPL_PC2MFP_SPI1_CLK | \
												   SYS_GPC_MFPL_PC3MFP_SPI1_SS | \
												   SYS_GPC_MFPL_PC4MFP_SPI1_I2SMCLK)); 
												   
#if defined(NT_I941)
#define SET_UART_PIN		   (SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk)); \
							   (SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD)); 
#endif
#if defined(NT_I941A)
#define SET_UART_PIN		   (SYS->GPA_MFPL &= ~SYS_GPA_MFPL_PA7MFP_Msk); \
                               (SYS->GPA_MFPH &= ~SYS_GPA_MFPH_PA8MFP_Msk); \
							   (SYS->GPA_MFPL |= SYS_GPA_MFPL_PA7MFP_UART0_TXD); \
                               (SYS->GPA_MFPH |= SYS_GPA_MFPH_PA8MFP_UART0_RXD); 
#endif

#define USB_MFP                SYS->GPB_MFPH
#define USB_PIN_MASK           (SYS_GPB_MFPH_PB13MFP_Msk | SYS_GPB_MFPH_PB14MFP_Msk | SYS_GPB_MFPH_PB15MFP_Msk)
#define USB_PIN				   (SYS_GPB_MFPH_PB13MFP_USBD_DN | SYS_GPB_MFPH_PB14MFP_USBD_DP | SYS_GPB_MFPH_PB15MFP_USBD_VBUS)	


#define I2S0_RX_PDMA_CH     0
#define I2S0_TX_PDMA_CH     1
#define DPWM_PDMA_CH		2
#define DMIC_PDMA_CH		3

#endif
