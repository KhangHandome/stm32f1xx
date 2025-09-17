<<<<<<< HEAD
#include "main.h"
#include "HAL_Timer.h"
static void CallbackFunction();

static HAL_TimerInit_t TIM2_Config =
{
		.Timer = TIM2,
		.Prescale_Value = 79,
		.Auto_Reload_Value = 65535,
		.TIM_AutoReloadEnable = TIM_AUTO_RELOAD_ENALBE,
		.HAL_Timer_Center_Aligned_Mode = HAL_TIM_EDGE_ALIGNED_MODE,
		.HAL_Timer_Counter_Dir = HAL_TIM_COUNTER_DONW,
		.HAL_Timer_Mode = HAL_TIM_CONTINUOUS_MODE,
		.HAL_Timer_UpdateReqSrc = HAL_TIMER_UPDATE_REQ_SRC_ALL,
		.HAL_Timer_UpdateEventState = HAL_TIMER_UEV_ENABLED,
		.Timer_Irq = TIM_IRQ_ENABLE,
		.HAL_Timer_Channel =
		{
				{
					.HAL_TIM_Channel_Enable = HAL_TIM_Channel_Enable,
					.HAL_Timer_Capture_Compare_Select = HAL_TIM_CAPTURE_INPUT_TI1,
					.HAL_Channel_Config_Pin = { .HAL_Timer_Channel_Config_Input_Edge = CHANNEL_CONFIG_INPUT_RISING_EDGE},
					.HAL_TIM_Capture_Compare_Mode =
					{
							.Capture_Mode = {
									.HAL_TIM_Inputr_Capture_Filter = HAL_TIM_ICF_NO_FILTER,
									.HAL_Timer_Input_Capture_Prescaler_t = HAL_TIM_ICPSC_DIV1,
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
volatile uint8_t measure_done = 0 ;
volatile uint32_t distance = 0 ;
uint16_t req = 0 ;


void delay_us(uint32_t us) {
    // Dùng SysTick làm delay us
    SysTick->LOAD = 8 * us - 1; // 8MHz -> 1us
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
    SysTick->CTRL = 0;
}

int main(void)
{
  __HAL_RCC_TIM2_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);   // Xóa cấu hình cũ
  GPIOA->CRL |= (0x01 << GPIO_CRL_CNF0_Pos);         // CNF0 = 01: Input floating
  // MODE0 = 00: input (đã clear ở trên)

  GPIOA->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1);    // Xóa bit cũ
  GPIOA->CRL |= (GPIO_CRL_MODE1_1 | GPIO_CRL_MODE1_0); // MODE0 = 11: output 50MHz
  GPIOA->CRL |= (GPIO_CRL_CNF1_1);                     // CNF0 = 10: Alternate function push-pull

  GPIOA->CRL &= ~(GPIO_CRL_MODE2 | GPIO_CRL_CNF2);    // Xóa bit cũ
  GPIOA->CRL |= (GPIO_CRL_MODE2_1 | GPIO_CRL_MODE2_0); // MODE0 = 11: output 50MHz
  GPIOA->CRL |= (GPIO_CRL_CNF2_1);

  // PA0 (Trigger) - Output push-pull, 50MHz
  GPIOA->CRL &= ~(0xF << (1 * 4));   // Clear CNF0[1:0] & MODE0[1:0]
  GPIOA->CRL |=  (0x3 << (1 * 4));   // MODE0 = 11 (Output 50MHz)
  GPIOA->CRL |=  (0x0 << (1 * 4 + 2)); // CNF0 = 00 (Push-pull)        // CNF6 = 00 (General purpose push-pull)

  NVIC_EnableIRQ(TIM2_IRQn);
  HAL_Timer_Init(&TIM2_Config);
  HAL_Timer_Set_CallbackFunction(&TIM2_Config);
  HAL_Timer_Start(&TIM2_Config);
  while (1)
  {
    GPIOA->BSRR = GPIO_BSRR_BS1; // PA1 = 1
    delay_us(7);
    GPIOA->BSRR = GPIO_BSRR_BR1; // PA1 = 0
	while(measure_done == 0 );
	measure_done = 0 ;
	for(volatile uint32_t i=0; i<100000; i++);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
static void CallbackFunction()
{
    static uint8_t status = 0;         // 0 = rising, 1 = falling
    static uint16_t cnt_rise = 0;
    static uint16_t cnt_fall = 0;
    static uint32_t pulse_high = 0;    // kết quả đo độ rộng xung cao
    static uint16_t counter = 0 ;
    if ( HAL_Timer_GetInterruptSource(&TIM2_Config) == TIMER_INT_CC1 )
    {
        if (status == 0)  // Rising edge
        {
            HAL_Timer_Set_Input_Edge(&TIM2_Config, 1, CHANNEL_CONFIG_INPUT_FAILING_EDGE);
            cnt_rise = HAL_Timer_GetCapture_CompareRegister(&TIM2_Config, 1);
            status = 1;
            counter = 0 ;
        }
        else              // Falling edge
        {
            HAL_Timer_Set_Input_Edge(&TIM2_Config, 1, CHANNEL_CONFIG_INPUT_RISING_EDGE);
            cnt_fall = HAL_Timer_GetCapture_CompareRegister(&TIM2_Config, 1);
            if (cnt_rise >= cnt_fall)
            {
                pulse_high = cnt_rise - cnt_fall;
            }
            else
            {
                pulse_high = (TIM2_Config.Auto_Reload_Value + cnt_rise - cnt_fall + 1); // xử lý tràn counter
            }
            pulse_high = pulse_high + counter * TIM2_Config.Auto_Reload_Value;
            status = 0;
            measure_done = 1 ;
            distance = pulse_high * 343 / 20000;
        }
    }
    if ( HAL_Timer_GetInterruptSource(&TIM2_Config) == TIMER_INT_UPDATE)
    {
    	counter += 1 ;
    }
    // Ở đây bạn có thể xử lý `pulse_high` (độ rộng mức cao, đơn vị = tick timer)
}
=======
#include "main.h"
#include "HAL_Timer.h"
static void CallbackFunction();

static HAL_TimerInit_t TIM2_Config =
{
		.Timer = TIM2,
		.Prescale_Value = 79,
		.Auto_Reload_Value = 65535,
		.TIM_AutoReloadEnable = TIM_AUTO_RELOAD_ENALBE,
		.HAL_Timer_Center_Aligned_Mode = HAL_TIM_EDGE_ALIGNED_MODE,
		.HAL_Timer_Counter_Dir = HAL_TIM_COUNTER_DONW,
		.HAL_Timer_Mode = HAL_TIM_CONTINUOUS_MODE,
		.HAL_Timer_UpdateReqSrc = HAL_TIMER_UPDATE_REQ_SRC_ALL,
		.HAL_Timer_UpdateEventState = HAL_TIMER_UEV_ENABLED,
		.Timer_Irq = TIM_IRQ_ENABLE,
		.HAL_Timer_Channel =
		{
				{
					.HAL_TIM_Channel_Enable = HAL_TIM_Channel_Enable,
					.HAL_Timer_Capture_Compare_Select = HAL_TIM_CAPTURE_INPUT_TI1,
					.HAL_Channel_Config_Pin = { .HAL_Timer_Channel_Config_Input_Edge = CHANNEL_CONFIG_INPUT_RISING_EDGE},
					.HAL_TIM_Capture_Compare_Mode =
					{
							.Capture_Mode = {
									.HAL_TIM_Inputr_Capture_Filter = HAL_TIM_ICF_NO_FILTER,
									.HAL_Timer_Input_Capture_Prescaler_t = HAL_TIM_ICPSC_DIV1,
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
volatile uint8_t measure_done = 0 ;
volatile uint32_t distance = 0 ;
uint16_t req = 0 ;


void delay_us(uint32_t us) {
    // Dùng SysTick làm delay us
    SysTick->LOAD = 8 * us - 1; // 8MHz -> 1us
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
    SysTick->CTRL = 0;
}

int main(void)
{
  __HAL_RCC_TIM2_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);   // Xóa cấu hình cũ
  GPIOA->CRL |= (0x01 << GPIO_CRL_CNF0_Pos);         // CNF0 = 01: Input floating
  // MODE0 = 00: input (đã clear ở trên)

  GPIOA->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1);    // Xóa bit cũ
  GPIOA->CRL |= (GPIO_CRL_MODE1_1 | GPIO_CRL_MODE1_0); // MODE0 = 11: output 50MHz
  GPIOA->CRL |= (GPIO_CRL_CNF1_1);                     // CNF0 = 10: Alternate function push-pull

  GPIOA->CRL &= ~(GPIO_CRL_MODE2 | GPIO_CRL_CNF2);    // Xóa bit cũ
  GPIOA->CRL |= (GPIO_CRL_MODE2_1 | GPIO_CRL_MODE2_0); // MODE0 = 11: output 50MHz
  GPIOA->CRL |= (GPIO_CRL_CNF2_1);

  // PA0 (Trigger) - Output push-pull, 50MHz
  GPIOA->CRL &= ~(0xF << (1 * 4));   // Clear CNF0[1:0] & MODE0[1:0]
  GPIOA->CRL |=  (0x3 << (1 * 4));   // MODE0 = 11 (Output 50MHz)
  GPIOA->CRL |=  (0x0 << (1 * 4 + 2)); // CNF0 = 00 (Push-pull)        // CNF6 = 00 (General purpose push-pull)

  NVIC_EnableIRQ(TIM2_IRQn);
  HAL_Timer_Init(&TIM2_Config);
  HAL_Timer_Set_CallbackFunction(&TIM2_Config);
  HAL_Timer_Start(&TIM2_Config);
  while (1)
  {
    GPIOA->BSRR = GPIO_BSRR_BS1; // PA1 = 1
    delay_us(7);
    GPIOA->BSRR = GPIO_BSRR_BR1; // PA1 = 0
	while(measure_done == 0 );
	measure_done = 0 ;
	for(volatile uint32_t i=0; i<100000; i++);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
static void CallbackFunction()
{
    static uint8_t status = 0;         // 0 = rising, 1 = falling
    static uint16_t cnt_rise = 0;
    static uint16_t cnt_fall = 0;
    static uint32_t pulse_high = 0;    // kết quả đo độ rộng xung cao
    static uint16_t counter = 0 ;
    if ( HAL_Timer_GetInterruptSource(&TIM2_Config) == TIMER_INT_CC1 )
    {
        if (status == 0)  // Rising edge
        {
            HAL_Timer_Set_Input_Edge(&TIM2_Config, 1, CHANNEL_CONFIG_INPUT_FAILING_EDGE);
            cnt_rise = HAL_Timer_GetCapture_CompareRegister(&TIM2_Config, 1);
            status = 1;
            counter = 0 ;
        }
        else              // Falling edge
        {
            HAL_Timer_Set_Input_Edge(&TIM2_Config, 1, CHANNEL_CONFIG_INPUT_RISING_EDGE);
            cnt_fall = HAL_Timer_GetCapture_CompareRegister(&TIM2_Config, 1);
            if (cnt_rise >= cnt_fall)
            {
                pulse_high = cnt_rise - cnt_fall;
            }
            else
            {
                pulse_high = (TIM2_Config.Auto_Reload_Value + cnt_rise - cnt_fall + 1); // xử lý tràn counter
            }
            pulse_high = pulse_high + counter * TIM2_Config.Auto_Reload_Value;
            status = 0;
            measure_done = 1 ;
            distance = pulse_high * 343 / 20000;
        }
    }
    if ( HAL_Timer_GetInterruptSource(&TIM2_Config) == TIMER_INT_UPDATE)
    {
    	counter += 1 ;
    }
    // Ở đây bạn có thể xử lý `pulse_high` (độ rộng mức cao, đơn vị = tick timer)
}
>>>>>>> 577c4389665029f4d923455370503e00bef7ac2d
