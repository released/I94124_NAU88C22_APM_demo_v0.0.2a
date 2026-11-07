#ifndef __SETTING_H__
#define __SETTING_H__

#include "constant.h"

// if need to use SPI1_I2S , EVM : PC0(X32_OUT) , PC1(X32_IN)
#define USB_EVB_SPI2_I2S

//#define TEST_FAR_END_DELAY  // Delay 100ms   

#define NAU88C22

#define ALGORITHM_RATE  	 RATE_16K
#define ALGORITHM_DATA_BIT	 BIT_16

#define UAC_RATE	 	ALGORITHM_RATE   
#define UAC_DATA_BIT	ALGORITHM_DATA_BIT	   

#define DMIC_RATE		ALGORITHM_RATE


#endif
