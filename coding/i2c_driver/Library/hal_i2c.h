#ifndef _HAL_I2C_H_
#define _HAL_I2C_H_

#include "stm32f103xb.h"

/* Define and typedef */
#define STM32_SLAVE_ADDRESS 0x20u
#define CLOCK_SPEED 10000u
#define FREQ        8u
typedef enum
{
    HAL_OK = 1,
    HAL_N_OK = 0
} Status_t ;

/* Private function prototypes */
void HAL_IP_I2C_Init(uint16_t clock_speed,uint16_t freq_mhz);
void HAL_IP_I2C_EnableISR(void);
Status_t HAL_IP_I2C_Receive(uint8_t *pData, uint8_t size);
Status_t HAL_IP_I2C_Transmit(uint8_t *pData, uint8_t size);
Status_t HAL_IP_I2C_Master_Transmit(uint8_t slaveAddr, uint8_t *pData, uint8_t size);
Status_t HAL_IP_I2C_Master_Receive(uint8_t slaveAddr, uint8_t *pData, uint8_t size);
Status_t HAL_IP_I2C_Slave_Receive_IT(uint8_t *pData,uint8_t size,Status_t* status);
Status_t HAL_IP_I2C_Slave_Transmit_IT(uint8_t *pData,uint8_t size);
Status_t HAL_IP_I2C_Master_Transmit_IT(uint8_t *pData,uint8_t size);
Status_t HAL_IP_I2C_Master_S_IT(uint8_t *pData,uint8_t size);


#endif
