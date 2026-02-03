/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "DRV_Lora.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
LORA_HandleTypedef Lora_Instance_0 =
{
		.SPI_Instance    = &hspi1,
		.ChipSelectPin   = LORA_CS_PIN_Pin,
		.ChipSelectPort  = LORA_CS_PIN_GPIO_Port,
		.ResetPin        = LORA_RST_PIN_Pin,
		.ResetPort       = LORA_RST_PIN_GPIO_Port,
		.Dio_0_Pin       = LORA_DIO0_PIN_Pin,
		.Dio_0_Port      = LORA_DIO0_PIN_GPIO_Port,
//		.OperationMode   = LORA_MODE_INTERRUPT,
		.OperationMode   = LORA_MODE_POLLING,
		.Bandwidth       = LORA_BW_125_KHZ,
		.Frequency       = 433000000,
		.SpreadingFactor = LORA_SF_7,
		.CodingRate      = LORA_CR_4_5,
		.Power           = LORA_POWER_MAX,
		.RxCallback      = (void*)(0x00),
		.TxCallback      = (void*)(0x00)

};

char *data_test = "Testing transmit and receive for module LoRa";
uint8_t data_rev[50] = {0};

typedef enum {
	IDLE,
	TRANSMIT,
	RECEIVE,
	CONFIRM
} StateMachine_t ;
/* USER CODE END 0 */
StateMachine_t StateMachine = IDLE ;
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  DRV_LoraInit(&Lora_Instance_0);
  DRV_LoraSwitchMode(&Lora_Instance_0, LORA_RECEIVE_CONTINUOUS_STATE);
//  StateMachine = RECEIVE;
//  NVIC_EnableIRQ(EXTI0_IRQn);
  LORA_ReturnTypedef status  ;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//	  MASTER
	  DRV_LoraTransmit(&Lora_Instance_0, (uint8_t*) data_test, 50, 100);
	  DRV_LoraReceive(&Lora_Instance_0, data_rev, 50, 100);
	  if(data_rev[0] == 'T' )
	  {
		  HAL_GPIO_TogglePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin);
		  data_rev[0] = 0 ;
	  }
	  HAL_Delay(50);
//    Slave
//	  DRV_LoraReceive(&Lora_Instance_0, data_rev, 50, 100);
//	  if(data_rev[0] == 'T')
//	  {
//		  DRV_LoraTransmit(&Lora_Instance_0, (uint8_t*) "Testing complete", 50, 100);
//		  HAL_GPIO_TogglePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin);
//		  data_rev[0] = '\0';
//	  }
//	  if(data_rev[0] == 'T')
//	  {
//		  HAL_GPIO_TogglePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin);
//		  status = DRV_LoraTransmit(&Lora_Instance_0, (uint8_t*) "Testing complete", 50,10);
//		  DRV_LoraSwitchMode(&Lora_Instance_0, LORA_RECEIVE_CONTINUOUS_STATE);
//	  	  data_rev[0] = '\0';
//	  }
//	  if ( StateMachine == CONFIRM )
//	  {
//		  DRV_LoraTransmit(&Lora_Instance_0, (uint8_t*) "Testing complete", 50,10);
//		  DRV_LoraSwitchMode(&Lora_Instance_0, LORA_RECEIVE_CONTINUOUS_STATE);
//	  }
//	  if(data_rev[0] == 'C' && StateMachine == CONFIRM)
//	  {
//		  StateMachine = RECEIVE;
//	  }
//
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void EXTI0_IRQHandler()
{
	DRV_LoraReceive(&Lora_Instance_0, data_rev, 50, 10);
	__HAL_GPIO_EXTI_CLEAR_IT(LORA_DIO0_PIN_Pin);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
