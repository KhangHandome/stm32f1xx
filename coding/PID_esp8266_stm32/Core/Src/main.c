/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdint.h"
#include "hal_dma.h"
#include "hal_usart.h"
#include "string.h"
#include "PID.h"
#include "stdlib.h"
#include "stdio.h"
/*Prototype */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void HAL_RxCallBackFunction(void);
static int ParsePIDString(const char* input, PID_t* pid);
static void float_to_string(float value, uint8_t* data);
/*
 * Typedef and enum
 */
typedef enum
{
	READ_UART_READY,
	READ_UART_DONE,
} UART_STATE_t;
typedef enum
{
	CAL_PID_READY,
	CAL_PID_DONE,
} CAL_PID_STATE_t;
/*
 * Variable
 */
UART_STATE_t uart1_state = READ_UART_READY;
CAL_PID_STATE_t cal_pid_state = CAL_PID_READY;
float speed = 0 ;
uint8_t data_read[32] = {0};
uint8_t data_send[32] = {0};
HAL_UART_t uart1_config = {
//	RX : PA10, TX : PA9
	.uart_port = USART1,
	.clock_input = 8000000,
	.baudrate = 9600,
	.use_dma = HAL_UART_DMA_DISABLE,
	.tx_interrupt = HAL_UART_INTERRUPT_DISABLE,
	.rx_interrupt = HAL_UART_INTERRUPT_ENABLE,
	.swap_pins = HAL_UART_SWAP_DISABLE,
	.dma_rx_buffer = data_read,
	.dma_rx_size = sizeof(data_read)
};
PID_t pid_config = {
		.Kd = 0,
		.Ki = 0,
		.Kp = 0,
		.integral = 0,
		.output = 0,
		.previos_error = 0,
		.setpoint = 0
};
static void HAL_RxCallBackFunction(void)
{
	static uint16_t index = 0 ;
	uint8_t data_rx       = 0 ;
	data_rx = HAL_UART_Read(&uart1_config);
	if (data_rx != '\n' )
	{
		uart1_state = READ_UART_READY;
		data_read[index++] = data_rx;
	}
	else
	{
		data_read[index] = data_rx;
		uart1_state = READ_UART_DONE;
		index = 0 ;
	}
}
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  HAL_UART_Init(&uart1_config);
  HAL_UART_SetRxCallBack(&uart1_config, HAL_RxCallBackFunction);
  memset(data_read,'\0',sizeof(data_read));
  while (1)
  {
    /* USER CODE END WHILE */
	switch (uart1_state) {
		case READ_UART_READY:
			/*
			 * Do nothing
			 */
			break;
		case READ_UART_DONE:
			ParsePIDString((char*)data_read,&pid_config);
			memset(data_read,'\0',sizeof(data_read));
			uart1_state = READ_UART_READY;
			break;
		default:
			break;
	}
	CalculatorPID(&pid_config,(float*)&speed);
	float_to_string(speed, data_send);
	HAL_UART_Transfer(&uart1_config, (uint8_t*)"CURRENT:");
	HAL_UART_Transfer(&uart1_config, data_send);
	HAL_Delay(50);
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

int ParsePIDString(const char* input, PID_t* pid)
{
    // Kiểm tra tiền tố "PID:"
    if (strncmp(input, "PID:", 4) != 0) {
        return -1; // Không đúng định dạng
    }

    // Tạo bản sao chuỗi để thao tác (tránh thay đổi input gốc)
    char buffer[64];
    strncpy(buffer, input + 4, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0'; // đảm bảo null-terminated

    // Tách các phần bằng dấu phẩy
    char* token = strtok(buffer, ",");
    if (token == NULL) return -1;
    pid->Kp = atof(token);

    token = strtok(NULL, ",");
    if (token == NULL) return -1;
    pid->Ki = atof(token);

    token = strtok(NULL, ",");
    if (token == NULL) return -1;
    pid->Kd = atof(token);

    token = strtok(NULL, ",");
    if (token == NULL) return -1;
    pid->setpoint = atof(token);


    return 0; // OK
}
void float_to_string(float value, uint8_t* data) {
    int int_part = (int)value;
    int frac_part = (int)((value - int_part) * 100);  // 2 chữ số sau dấu thập phân

    // Xử lý số âm
    int is_negative = 0;
    if (value < 0) {
        is_negative = 1;
        int_part = -int_part;
        frac_part = -frac_part;
    }

    int idx = 0;

    if (is_negative) {
        data[idx++] = '-';
    }

    // Chuyển phần nguyên sang ký tự
    if (int_part == 0) {
        data[idx++] = '0';
    } else {
        char rev[10];
        int rev_idx = 0;
        int temp = int_part;
        while (temp > 0) {
            rev[rev_idx++] = (temp % 10) + '0';
            temp /= 10;
        }
        while (rev_idx > 0) {
            data[idx++] = rev[--rev_idx];
        }
    }

    // Thêm dấu chấm
    data[idx++] = '.';

    // Đảm bảo luôn có 2 chữ số phần thập phân
    data[idx++] = (frac_part / 10) + '0';
    data[idx++] = (frac_part % 10) + '0';

    // Thêm newline
    data[idx++] = '\n';

    // Kết thúc chuỗi nếu cần
    data[idx] = '\0';
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

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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
