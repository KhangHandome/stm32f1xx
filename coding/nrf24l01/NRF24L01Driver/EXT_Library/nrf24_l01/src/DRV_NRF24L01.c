#include "DRV_NRF24L01.h"
#include "stdint.h"
#define READ_REGISTER(x) ((x) & 0x1F)
#define WRITE_REGISTER(x) ((x) | 0x20 )
static void DRV_Nrf_ReadRegister(uint8_t reg, uint8_t* destinationPtr);
static void DRV_Nrf_WriteRegister(uint8_t reg, uint8_t data);

static void DRV_Nrf_ReadRegister(uint8_t reg, uint8_t* destinationPtr)
{
	uint8_t cmd = READ_REGISTER(reg);
	/* Turn down CS */
	HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
	HAL_SPI_Receive(&hspi1, destinationPtr, 1, 10);
	/* Turn up CS */
}
static void DRV_Nrf_WriteRegister(uint8_t reg, uint8_t data)
{
	uint8_t cmd = WRITE_REGISTER(reg);
	/* Turn down CS */
	HAL_SPI_Transmit(&hspi1, &cmd , 1, 10);
	HAL_SPI_Transmit(&hspi1, &data, 1, 10);
	/* Turn up CS */
}
void DRV_Nrf24l01Init()
{
	
}
void DRV_Nrf24l01Deinit();
void DRV_Nrf24l01Receive();
void DRV_Nrf24l01Transmit();
void DRV_Nrf24l01SetChannel();
void DRV_Nrf24l01SetDataRate();
void DRV_Nrf24l01SetPALevel();
void DRV_Nrf24l01OpenWritingPipe();
void DRV_Nrf24l01OpenReadingPiple();
void DRV_Nrf24l01SwitchMode();