#ifndef _HAL_DMA_H_
#define _HAL_DMA_H_

#include "stm32f1xx.h"

#ifndef NULL_PTR
#define NULL_PTR ((void*) 0x00)

#endif

typedef void (*HAL_DMA_CallBackFunction)(void);

extern void HAL_SetCallBackFunction(DMA_Channel_TypeDef* DMA_Channel, HAL_DMA_CallBackFunction cb);


#endif
