#include "HAL_Timer.h"
#include "main.h"
/*
 * @brief : Callback function pointers for STM32F103 timers
 */
static CallbackFunction_t CallbackFunction_TIM1; /* Callback for TIM1 interrupt */
static CallbackFunction_t CallbackFunction_TIM2; /* Callback for TIM2 interrupt */
static CallbackFunction_t CallbackFunction_TIM3; /* Callback for TIM3 interrupt */
static CallbackFunction_t CallbackFunction_TIM4; /* Callback for TIM4 interrupt */
#ifdef TIM5
static CallbackFunction_t CallbackFunction_TIM5; /* Callback for TIM5 interrupt, available in some STM32F103 models */
#endif

static void HAL_Timer_ConfigChannel(TIM_TypeDef *TIMx, uint8_t channel, HAL_Timer_Channel_Config_t *HAL_Timer_Channel_Config);
static void HAL_Timer_ConfigChannel(TIM_TypeDef *TIMx, uint8_t channel, HAL_Timer_Channel_Config_t *HAL_Timer_Channel_Config)
{
    uint32_t shift = (channel % 2) * 8;
    volatile uint32_t *ccmr;

    // Xác định dùng CCMR1 (CH1, CH2) hay CCMR2 (CH3, CH4)
    if (channel < 2) {
        ccmr = &TIMx->CCMR1;
    } else {
        ccmr = &TIMx->CCMR2;
    }

    // Xóa bits cũ trong CCMR tương ứng với channel
    *ccmr &= ~(0xFF << shift);

    if ( HAL_Timer_Channel_Config->HAL_TIM_Channel_Enable == HAL_TIM_Channel_Enable)
    {
    	if (HAL_Timer_Channel_Config->HAL_TIM_Capture_Compare_Mode.Compare_Mode.HAL_Timer_Capture_Compare_Select == HAL_TIM_COMPARE_OUTPUT)
		{
			// Cấu hình Output Compare Mode
			HAL_TIM_Output_Compare_Mode_t *oc = &HAL_Timer_Channel_Config->HAL_TIM_Capture_Compare_Mode.Compare_Mode;
			uint16_t val = 0;
			val |= (oc->HAL_Timer_Capture_Compare_Select & 0x3) << (shift + 0);  // CCxS
			val |= (oc->Output_Compare_Fast_Enable & 0x1) << (shift + 2);         // OCxFE
			val |= (oc->Output_Compare_Preload & 0x1) << (shift + 3);             // OCxPE
			val |= (oc->HAL_Timer_Output_Compare_Mode & 0x7) << (shift + 4);      // OCxM
			val |= (oc->Output_Compare_Clear_Enable & 0x1) << (shift + 7);        // OCxCE
			*ccmr |= val;
		    // Thiết lập giá trị CCR ban đầu
		    switch(channel) {
		        case 0: TIMx->CCR1 = HAL_Timer_Channel_Config->HAL_TIM_Capture_Compare_Register; break;
		        case 1: TIMx->CCR2 = HAL_Timer_Channel_Config->HAL_TIM_Capture_Compare_Register; break;
		        case 2: TIMx->CCR3 = HAL_Timer_Channel_Config->HAL_TIM_Capture_Compare_Register; break;
		        case 3: TIMx->CCR4 = HAL_Timer_Channel_Config->HAL_TIM_Capture_Compare_Register; break;
		    }
		    TIMx->CCER |= ((uint8_t)HAL_Timer_Channel_Config->HAL_Channel_Config_Pin.HAL_Timer_Channel_Config_Output << (channel * 4 + TIM_CCER_CC1P_Pos)); // Config Capture/Compare 1 output polarity
		}
		else
		{
			// Cấu hình Input Capture Mode
			HAL_TIM_Input_Capture_Mode_t *ic = &HAL_Timer_Channel_Config->HAL_TIM_Capture_Compare_Mode.Capture_Mode;
			uint32_t val = 0;
			val |= (ic->HAL_Timer_Capture_Compare_Select & 0x3) << (shift + 0);       // CCxS
			val |= (ic->HAL_Timer_Input_Capture_Prescaler_t & 0x3) << (shift + 2);    // ICxPSC
			val |= (ic->HAL_TIM_Inputr_Capture_Filter & 0xF) << (shift + 4);          // ICxF
			*ccmr |= val;
		    TIMx->CCER |= ( (uint8_t)HAL_Timer_Channel_Config->HAL_Channel_Config_Pin.HAL_Timer_Channel_Config_Input_Edge << (channel * 4 + TIM_CCER_CC1P_Pos)); // Config Capture/Compare 1 output polarity
		}
    	if(HAL_Timer_Channel_Config->HAL_TIM_Capture_Compare_IRQ == TIM_IRQ_ENABLE)
    	{
    		TIMx->DIER &= ~(1 << ( channel +1 )); /* Reset bit irq */
    		TIMx->DIER |= ( 1 << (channel + 1));
    	}
        // Enable kênh trong CCER nếu cần
        // Mỗi channel CCER chiếm 4 bit: CCxE (enable), CCxP (polarity)
        TIMx->CCER |= (1 << (channel * 4)); // Bật CCxE mặc định
    }
}
/*
 * @brief : Initializes the timer peripheral with the specified configuration
 * @param : TimerInit - Timer configuration structure
 */
void HAL_Timer_Init(HAL_TimerInit_t* TimerInit)
{
	uint8_t channel = 0 ;
	/*Setup prescale for timer*/
    TimerInit->Timer->PSC = TimerInit->Prescale_Value;
    /*Setup up autoreload value for time*/
    TimerInit->Timer->ARR = TimerInit->Auto_Reload_Value;
    /*Setup auto reload preload */
    TimerInit->Timer->CR1 &= TIM_CR1_ARPE_Msk;
    TimerInit->Timer->CR1 |= (TimerInit->TIM_AutoReloadEnable <<TIM_CR1_ARPE_Pos);
    /*Setup center aligned mode */
    TimerInit->Timer->CR1 |= (TimerInit->HAL_Timer_Center_Aligned_Mode << TIM_CR1_CMS_Pos);
    /*Setup conter up or counter down */
    TimerInit->Timer->CR1 |= (TimerInit->HAL_Timer_Counter_Dir << TIM_CR1_DIR_Pos);
    /*Setup timer stop at UEV ( Update event ) or not*/
    TimerInit->Timer->CR1 |= (TimerInit->HAL_Timer_Mode << TIM_CR1_OPM_Pos);
    /*Setup timer update request source */
    TimerInit->Timer->CR1 |= (TimerInit->HAL_Timer_UpdateReqSrc << TIM_CR1_URS_Pos);
    /*Setup timer update event, when disable the registers of tim does not update */
    TimerInit->Timer->CR1 |= (TimerInit->HAL_Timer_UpdateEventState << TIM_CR1_UDIS_Pos);
    /*Setup interrupt for timer */
    TimerInit->Timer->DIER |= ( TimerInit->Timer_Irq << TIM_DIER_UIE_Pos);
    for ( channel = 0 ; channel < MAX_CHANNEL_TIMER; channel ++)
    {
        HAL_Timer_Channel_Config_t *HAL_Timer_Channel_Config = &TimerInit->HAL_Timer_Channel[channel];
        HAL_Timer_ConfigChannel(TimerInit->Timer, channel, HAL_Timer_Channel_Config);
    }
}

/*
 * @brief : Initializes and configures the timer with the provided setup parameters
 * @param : TimerInit - Timer configuration structure
 */
void HAL_Timer_Setup(HAL_TimerInit_t* TimerInit)
{

}

/*
 * @brief : Starts the timer with the specified configuration
 * @param : TimerInit - Timer configuration structure
 */
void HAL_Timer_Start(HAL_TimerInit_t* TimerInit)
{
	TimerInit->Timer->CR1 |= TIM_CR1_CEN;
}

/*
 * @brief : Sets the callback function for timer interrupts
 * @param : TimerInit - Timer configuration structure
 */
void HAL_Timer_Set_CallbackFunction(HAL_TimerInit_t* TimerInit)
{
	switch ((uint32_t)(TimerInit->Timer))
	{
		case (uint32_t) TIM1: break ;
		case (uint32_t) TIM2:
			CallbackFunction_TIM2 = TimerInit->CallbackFunction;
			break ;
		default : break ;
	}
}

/*
 * @brief : Clears the timer's interrupt flag
 * @param : TimerInit - Timer configuration structure
 */
void HAL_Timer_ClearInterruptFlag(HAL_TimerInit_t* TimerInit){}

/*
 * @brief : Returns the timer's interrupt flag status
 * @param : TimerInit - Timer configuration structure
 */
void HAL_Timer_GetInterruptFlag(HAL_TimerInit_t* TimerInit){}

/*
 * @brief : Returns the value of the capture/compare register
 * @param : TimerInit - Timer configuration structure
 */
void HAL_Timer_GetCapture_CompareRegister(HAL_TimerInit_t* TimerInit){}

/*
 * @brief : Interrupt Service Routine for TIM1 Break interrupt
 */
void TIM1_BRK_IRQHandler(void) {
    if (TIM1->SR & TIM_SR_BIF) { /* Check if Break interrupt flag is set */
        TIM1->SR &= ~TIM_SR_BIF; /* Clear Break interrupt flag */
        if (CallbackFunction_TIM1 != NULL) { /* Check if callback is set */
            CallbackFunction_TIM1(); /* Call the user-defined callback */
        }
    }
}

/*
 * @brief : Interrupt Service Routine for TIM1 Update interrupt
 */
void TIM1_UP_IRQHandler(void) {
    if (TIM1->SR & TIM_SR_UIF) { /* Check if Update interrupt flag is set */
        TIM1->SR &= ~TIM_SR_UIF; /* Clear Update interrupt flag */
        if (CallbackFunction_TIM1 != NULL) { /* Check if callback is set */
            CallbackFunction_TIM1(); /* Call the user-defined callback */
        }
    }
}

/*
 * @brief : Interrupt Service Routine for TIM1 Trigger and Commutation interrupts
 */
void TIM1_TRG_COM_IRQHandler(void) {
    if (TIM1->SR & TIM_SR_TIF) { /* Check if Trigger interrupt flag is set */
        TIM1->SR &= ~TIM_SR_TIF; /* Clear Trigger interrupt flag */
        if (CallbackFunction_TIM1 != NULL) { /* Check if callback is set */
            CallbackFunction_TIM1(); /* Call the user-defined callback */
        }
    }
    if (TIM1->SR & TIM_SR_COMIF) { /* Check if Commutation interrupt flag is set */
        TIM1->SR &= ~TIM_SR_COMIF; /* Clear Commutation interrupt flag */
        if (CallbackFunction_TIM1 != NULL) { /* Check if callback is set */
            CallbackFunction_TIM1(); /* Call the user-defined callback */
        }
    }
}

/*
 * @brief : Interrupt Service Routine for TIM1 Capture/Compare interrupt
 */
void TIM1_CC_IRQHandler(void) {
    if (TIM1->SR & (TIM_SR_CC1IF | TIM_SR_CC2IF | TIM_SR_CC3IF | TIM_SR_CC4IF)) { /* Check if any Capture/Compare interrupt flag is set */
        TIM1->SR &= ~(TIM_SR_CC1IF | TIM_SR_CC2IF | TIM_SR_CC3IF | TIM_SR_CC4IF); /* Clear all Capture/Compare interrupt flags */
        if (CallbackFunction_TIM1 != NULL) { /* Check if callback is set */
            CallbackFunction_TIM1(); /* Call the user-defined callback */
        }
    }
}

/*
 * @brief : Interrupt Service Routine for TIM2 Update interrupt
 */
void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) { /* Check if Update interrupt flag is set */
        TIM2->SR &= ~TIM_SR_UIF; /* Clear Update interrupt flag */
        if (CallbackFunction_TIM2 != NULL) { /* Check if callback is set */
            CallbackFunction_TIM2(); /* Call the user-defined callback */
        }
    }
}

/*
 * @brief : Interrupt Service Routine for TIM3 Update interrupt
 */
void TIM3_IRQHandler(void) {
    if (TIM3->SR & TIM_SR_UIF) { /* Check if Update interrupt flag is set */
        TIM3->SR &= ~TIM_SR_UIF; /* Clear Update interrupt flag */
        if (CallbackFunction_TIM3 != NULL) { /* Check if callback is set */
            CallbackFunction_TIM3(); /* Call the user-defined callback */
        }
    }
}

/*
 * @brief : Interrupt Service Routine for TIM4 Update interrupt
 */
void TIM4_IRQHandler(void) {
    if (TIM4->SR & TIM_SR_UIF) { /* Check if Update interrupt flag is set */
        TIM4->SR &= ~TIM_SR_UIF; /* Clear Update interrupt flag */
        if (CallbackFunction_TIM4 != NULL) { /* Check if callback is set */
            CallbackFunction_TIM4(); /* Call the user-defined callback */
        }
    }
}

#ifdef TIM5
/*
 * @brief : Interrupt Service Routine for TIM5 Update interrupt
 */
void TIM5_IRQHandler(void) {
    if (TIM5->SR & TIM_SR_UIF) { /* Check if Update interrupt flag is set */
        TIM5->SR &= ~TIM_SR_UIF; /* Clear Update interrupt flag */
        if (CallbackFunction_TIM5 != NULL) { /* Check if callback is set */
            CallbackFunction_TIM5(); /* Call the user-defined callback */
        }
    }
}
#endif
