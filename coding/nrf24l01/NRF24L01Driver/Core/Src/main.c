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
#include "DRV_NRF24L01.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE BEGIN PD */
/* Comment dòng này nếu nạp cho board NHẬN, mở comment nếu nạp cho board PHÁT */
#define IS_TRANSMITTER_NODE
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t masterAddress0[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
uint8_t masterAddress1[5] = {0x12, 0x13, 0x14, 0x15, 0x16};
uint8_t masterAddress2[4] = {0x15, 0x16, 0x17, 0x18};
uint8_t dummyPayload[32];
uint32_t txCounter = 0;
volatile uint8_t flag = 0 ;
volatile uint8_t transmited_flag = 0 ;
volatile uint8_t received_flag = 0 ;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE BEGIN 0 */
NRF24L01_HandleTypedef Instance0 =
{
    .SPI_Instance = &hspi1,
    .ChipSelectPin = CHIP_Select_Pin,
    .ChipSelectPort = CHIP_Select_GPIO_Port,
    .InterruptPin = CHIP_Irq_Pin,
    .InterruptPort = CHIP_Irq_GPIO_Port,
    .ChipEnablePin = CHIP_Enable_Pin,
    .ChipEnablePort  = CHIP_Enable_GPIO_Port,
    .InterruptMode = TRUE,
    .NRF24L01_OutputPower = NRF24_MEDIUM_POWER,
    .NRF24L01_AirDataDate = NRF24L01_1MBPS,
    .FrequencyChannel     = 120,
    .AutoRetransmitCount = 10,
    .AutoRetransmitDelay = 10,
	.DynamicPayloadEnable = TRUE,
	.PayloadWithAckEnable = FALSE,
	.State = NRF24_UNINIT,
    // Gán địa chỉ mặc định
    .RxAddressP0 = {0x11, 0x22, 0x33, 0x44, 0x55},
    .RxAddressP1 = {0x12, 0x13, 0x14, 0x15, 0x16},
	.RxAddressP2_5 = {0x15,0x16,0x17,0x18},
    .TxAddress   = {0x11, 0x22, 0x33, 0x44, 0x55}
};
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	uint8_t pipeID = 0 ;
	uint32_t counter = 0 ;
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
  if(DRV_Nrf24l01Init(&Instance0) != STD_E_OK)
  {
	Error_Handler(); // Nếu Init lỗi sẽ kẹt ở đây
  }
  DRV_Nrf24l01SwitchMode(&Instance0,NRF24_RECEIVE_MODE);
#ifndef IS_TRANSMITTER_NODE

  NVIC_EnableIRQ(EXTI4_IRQn);
#endif
  NVIC_EnableIRQ(EXTI4_IRQn);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
#ifdef IS_TRANSMITTER_NODE
	  uint8_t status = 0 ;
      /* --- KỊCH BẢN PHÁT (TX) --- */
	  status = DRV_Nrf24l01Transmit(&Instance0,2, &masterAddress2[1], (uint8_t*)"FPT SOFT WARE ");
	  while( transmited_flag == 0 );
	  transmited_flag = 0;
	  DRV_Nrf24l01SwitchMode(&Instance0, NRF24_RECEIVE_MODE);
	  while( received_flag == 0 );
	  received_flag = 0 ;
	  if ( status == STD_E_OK)
	  {
		  DRV_Nrf24l01Receive(&Instance0,&pipeID,dummyPayload, 32);
	      if(dummyPayload[0] == 'C')
	      {
	    	  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	    	  dummyPayload[0] = '\0';

	      }
	  }
	  else
	  {
		  counter ++ ;
	  }
      HAL_Delay(100); // Gửi mỗi 0.5 giây

#else
      /* --- KỊCH BẢN NHẬN (RX) --- */
      if ( DRV_Nrf24l01DataReceiveAvailable(&Instance0) == 1)
      {
          DRV_Nrf24l01Receive(&Instance0,&pipeID, dummyPayload, 10 );
      }
      if(dummyPayload[0] == 'F')
      {
    	  DRV_Nrf24l01Transmit(&Instance0,2, &masterAddress2[1], (uint8_t*)"Complete");
    	  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    	  dummyPayload[0] = '\0';
      }
#endif
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
void EXTI4_IRQHandler()
{
	uint8_t status = 0 ;
	status = DRV_Nrf24l01GetStatus(&Instance0);
	if ( ((status >> 5) & 0x01) == 1  )
	{
		transmited_flag = 1 ;
		DRV_Nrf24l01ClearIRQ(&Instance0, 1 << 5 );
	}
	if ( ((status >> 6) & 0x01) == 1  )
	{
		received_flag = 1 ;
		DRV_Nrf24l01ClearIRQ(&Instance0, 1 << 6 );
	}
	__HAL_GPIO_EXTI_CLEAR_IT(CHIP_Irq_Pin);
	flag = 1 ;
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
