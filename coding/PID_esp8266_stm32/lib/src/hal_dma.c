#include "hal_dma.h"
#include "stdint.h"
#include "stm32f1xx.h"
static HAL_DMA_CallBackFunction DMA1_Chanel5_CallBackFunction = NULL_PTR;
void HAL_SetCallBackFunction(DMA_Channel_TypeDef* DMA_Channel, HAL_DMA_CallBackFunction cb)
{
	if(DMA_Channel == DMA1_Channel5)
	{
		DMA1_Chanel5_CallBackFunction = cb ;
	}
}
void DMA1_Channel5_IRQHandler()
{
	if(DMA1_Chanel5_CallBackFunction != NULL_PTR)
	{
		DMA1_Chanel5_CallBackFunction();
	}
	else
	{
// Do nothing
	}
}
