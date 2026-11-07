#ifndef __SETTING_H__
#define __SETTING_H__

#include "constant.h"

// Supported EVB:  
// 1. NT_I941  
// 2. NT_I941A  
// 3. CUSTOM (User-defined)  
#define NT_I941

#if !defined(NT_I941) && !defined(NT_I941A) 
   #if defined(CUSTOM)
    #warning "User must check all Custom part is correct"
   #else
	#error "Must set a board"
   #endif
#endif

#ifdef NT_I941
#define NAU88C22
#define NEAR_END_DMIC		 	0
#endif
#ifdef NT_I941A
#define NAU88L21
#define NEAR_END_DMIC		 	0
#endif

#define ALGORITHM_RATE  	 RATE_16K
#define ALGORITHM_DATA_BIT	 BIT_16

#define UAC_RATE	 	ALGORITHM_RATE   
#define UAC_DATA_BIT	ALGORITHM_DATA_BIT	   

#define DMIC_RATE		ALGORITHM_RATE


#endif
