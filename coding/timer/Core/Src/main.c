#include "main.h"
#include "HAL_Timer.h"
static void CallbackFunction();

static HAL_TimerInit_t TIM2_Config =
{
		.Timer = TIM2,
		.Prescale_Value = 79,
		.Auto_Reload_Value = 2000,
		.TIM_AutoReloadEnable = TIM_AUTO_RELOAD_ENALBE,
		.HAL_Timer_Center_Aligned_Mode = HAL_TIM_EDGE_ALIGNED_MODE,
		.HAL_Timer_Counter_Dir = HAL_TIM_COUNTER_UP,
		.HAL_Timer_Mode = HAL_TIM_ONE_PULSE_MODE,
		.HAL_Timer_UpdateReqSrc = HAL_TIMER_UPDATE_REQ_SRC_ALL,
		.HAL_Timer_UpdateEventState = HAL_TIMER_UEV_ENABLED,
		.Timer_Irq = TIM_IRQ_ENABLE,
		.HAL_Timer_Channel =
		{
				{
					.HAL_TIM_Channel_Enable = HAL_TIM_Channel_Enable,
					.HAL_TIM_Capture_Compare_Register = 500,
					.HAL_Channel_Config_Pin = { .HAL_Timer_Channel_Config_Output = CHANNEL_CONFIG_OUTPUT_ACTIVE_HIGH},
					.HAL_TIM_Capture_Compare_Mode =
					{
							.Compare_Mode = {
									.HAL_Timer_Capture_Compare_Select = HAL_TIM_COMPARE_OUTPUT,
									.Output_Compare_Fast_Enable = HAL_TIM_DISABLE,
									.Output_Compare_Preload = HAL_TIM_ENABLE,
									.HAL_Timer_Output_Compare_Mode = HAL_TIM_OCM_PWM1,
									.Output_Compare_Clear_Enable = HAL_TIM_DISABLE
							},
					},
					.HAL_TIM_Capture_Compare_IRQ = TIM_IRQ_ENABLE
				},
				{0},
				{0},
				{0}
		},
		.CallbackFunction = CallbackFunction
};
int main(void)
{
  __HAL_RCC_TIM2_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);    // Xóa bit cũ
  GPIOA->CRL |= (GPIO_CRL_MODE0_1 | GPIO_CRL_MODE0_0); // MODE0 = 11: output 50MHz
  GPIOA->CRL |= (GPIO_CRL_CNF0_1);                     // CNF0 = 10: Alternate function push-pull

  NVIC_EnableIRQ(TIM2_IRQn);
  HAL_Timer_Init(&TIM2_Config);
  HAL_Timer_Set_CallbackFunction(&TIM2_Config);
  HAL_Timer_Start(&TIM2_Config);
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
static void CallbackFunction()
{
	HAL_Timer_Start(&TIM2_Config);
}
