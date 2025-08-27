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


/*
 * @brief : Initializes the timer peripheral with the specified configuration
 * @param : TimerInit - Timer configuration structure
 */
void HAL_Timer_Init(HAL_TimerInit_t* TimerInit)
{

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
