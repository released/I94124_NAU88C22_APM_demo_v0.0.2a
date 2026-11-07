#ifndef __CONSTANT_H__
#define __CONSTANT_H__

enum Rate {
  RATE_16K = 16000,
  RATE_32K = 32000,
  RATE_48K = 48000
}; 

enum Bit {
  BIT_16 = 16,
  BIT_24 = 24,	
  BIT_32 = 32,
}; 


/**
  * @brief   Provide user selections of HIRC's frequency.
  * @details 
  * SYSCLK_HIRC_CLK_49152 : HIRC's frequency is 49.152 MHz.
  * SYSCLK_HIRC_CLK_48000 : HIRC's frequency is 48 MHz.
  */
#define SYSCLK_HIRC_CLK_49152    (0)   
#define SYSCLK_HIRC_CLK_48000    (1)   

/**
  * @brief   Provide user selections of HCLK's clock source.
  * @details 
  * SYSCLK_HCLK_CLK_HIRC     : HCLK's clock source is from HIRC.
  * SYSCLK_HCLK_CLK_HXT      : HCLK's clock source is from HXT.
  * SYSCLK_HCLK_CLK_PLL_HIRC : HCLK's clock source is from PLL, and PLL's clock source is from HIRC.
  * SYSCLK_HCLK_CLK_PLL_HXT  : HCLK's clock source is from PLL, and PLL's clock source is from HXT.
  */
#define SYSCLK_HCLK_CLK_HIRC     (0)    
#define SYSCLK_HCLK_CLK_HXT      (1)    
#define SYSCLK_HCLK_CLK_PLL_HIRC (2)     
#define SYSCLK_HCLK_CLK_PLL_HXT  (3)    


#endif
