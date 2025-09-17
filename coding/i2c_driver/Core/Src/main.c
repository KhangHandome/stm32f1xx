#include "hal_i2c.h"
#include "stm32f103xb.h"

static void LED_Init(void);
static void LED_Toggle(void);

int main(void)
{
    Status_t status = HAL_OK;
    uint8_t data_rx[20];

    /* Enable GPIO ports */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    LED_Init();
    HAL_IP_I2C_Init(CLOCK_SPEED,FREQ);
    HAL_IP_I2C_EnableISR();
    HAL_IP_I2C_Slave_Receive_IT(data_rx,10, &status );
    while (1)
    {
        /* Slave Receive Example */
/*        status = HAL_IP_I2C_Receive(data_rx, 14); */
        if (status == HAL_OK)
        {
            LED_Toggle();
            status = HAL_N_OK;
        }
    }
}

/* ---------------- LED ---------------- */
static void LED_Init(void)
{
    GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
    GPIOC->CRH |= GPIO_CRH_MODE13_0;  // Output mode 10MHz
}

static void LED_Toggle(void)
{
    GPIOC->ODR ^= GPIO_ODR_ODR13;
}
