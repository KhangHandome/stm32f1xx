#include "lora.h"

/*
 * Prototype static function
 */
static void DRV_Lora_ReadRegister(const uint8_t reg, uint8_t *data);
static void DRV_Lora_WriteRegister(const uint8_t reg, uint8_t data);
/*
 * Definition function
 */
void DRV_Lora_Init()
{
	uint8_t rev_data = 0 ;
	DRV_Lora_ReadRegister(REG_OP_MODE, &rev_data);

	/* Set to sleep mode */
	DRV_Lora_WriteRegister(REG_OP_MODE, SLEEP_MODE);
	DRV_Lora_ReadRegister(REG_OP_MODE, &rev_data);

	/* Set mode to lora mode */
	DRV_Lora_WriteRegister(REG_OP_MODE, 1 << 7 );
	DRV_Lora_ReadRegister(REG_OP_MODE, &rev_data);

	/* Set mode to stand by mode */
	DRV_Lora_WriteRegister(REG_OP_MODE, STANDBY_MODE);
	/* Read back data from register OP */
	DRV_Lora_ReadRegister(REG_OP_MODE, &rev_data);

    /* Configure LoRa parameters */
	DRV_Lora_WriteRegister(REG_MODEM_CONFIG_1, 0x72); // BW=125kHz, CR=4/5, Implicit Header OFF
	DRV_Lora_WriteRegister(REG_MODEM_CONFIG_2, 0x74); // SF=7, TxContinuousMode=OFF, CRC=ON
	DRV_Lora_WriteRegister(REG_MODEM_CONFIG_3, 0x04); // LowDataRateOptimize=OFF, AGC=ON

    // Frequency (ví dụ 433MHz)
    DRV_Lora_WriteRegister(REG_FRF_MSB, 0x6C);
    DRV_Lora_WriteRegister(REG_FRF_MID, 0x40);
    DRV_Lora_WriteRegister(REG_FRF_LSB, 0x00);

    // Sync word
    DRV_Lora_WriteRegister(REG_SYNC_WORD, 0x12);

	/* Reset address pointer in FIFO, base, tx, rx */
	DRV_Lora_WriteRegister(REG_FIFO_ADDR_PTR, 0x00);
	DRV_Lora_WriteRegister(REG_FIFO_TX_BASE_ADDR, 0x00);
	DRV_Lora_WriteRegister(REG_FIFO_RX_BASE_ADDR, 0x00);

	/* Bật PA_BOOST, Max Power (khoảng +17dBm)*/
	DRV_Lora_WriteRegister(REG_PA_CONFIG, 0x8F);

	/* Read back data from register OP */
	DRV_Lora_ReadRegister(REG_OP_MODE, &rev_data);

	/* Read back data from register OP */
	DRV_Lora_ReadRegister(REG_SYNC_WORD, &rev_data);

	/* Read back data from register OP */
	DRV_Lora_ReadRegister(REG_IRQ_FLAGS_MASK, &rev_data);

//	DRV_Lora_WriteRegister(REG_IRQ_FLAGS_MASK, (0xFF & (~(1 << 6 ))));
//
//	DRV_Lora_WriteRegister(0x40, 0x00); // Đưa RxDone ra chân DIO0

	/* Read back data from register OP */
	DRV_Lora_ReadRegister(REG_IRQ_FLAGS_MASK, &rev_data);

	/* Clear IRQ */
	DRV_Lora_WriteRegister(REG_IRQ_FLAGS, 0xFF);

	/* Switch mode to receive continuous mode */
//	DRV_Lora_WriteRegister(REG_OP_MODE, RECEIVE_CONTINUOUS_MODE);
//	DRV_Lora_ReadRegister(REG_OP_MODE, &rev_data);
}
void DRV_Lora_Deinit()
{

}
void DRV_Lora_Reset()
{
	/*
	* Reset lora
	*/
	HAL_GPIO_WritePin(RST_LORA_GPIO_Port, RST_LORA_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(RST_LORA_GPIO_Port, RST_LORA_Pin, GPIO_PIN_SET);
	HAL_Delay(10);
}
void DRV_Lora_Transmit(uint8_t *data, uint8_t size)
{
    uint8_t addr = 0 ;
    uint8_t response = 0 ;
	/* Switch to standby mode */
	DRV_Lora_WriteRegister(REG_OP_MODE, STANDBY_MODE);
	/* Clear IRQ */
	DRV_Lora_WriteRegister(REG_IRQ_FLAGS, 0xFF);
	/* Reset address pointer in FIFO, base, tx, rx */
	DRV_Lora_WriteRegister(REG_FIFO_ADDR_PTR, 0x00);
	DRV_Lora_WriteRegister(REG_FIFO_TX_BASE_ADDR, 0x00);
	/* Write payload to FIFO */
	HAL_GPIO_WritePin(LORA_CS_PIN_GPIO_Port, LORA_CS_PIN_Pin, GPIO_PIN_RESET );
	addr = (REG_FIFO | WRITE_TO_LORA);
    HAL_SPI_Transmit(&hspi1, &addr, 1, 100);
    HAL_SPI_Transmit(&hspi1, data, size, 1000);
	HAL_GPIO_WritePin(LORA_CS_PIN_GPIO_Port, LORA_CS_PIN_Pin, GPIO_PIN_SET );
	/* Set payload length */
	DRV_Lora_WriteRegister(REG_PAYLOAD_LENGTH, size);
	/* Move to tx mode */
	DRV_Lora_WriteRegister(REG_OP_MODE, TRANSMIT_MODE);
	/* Waiting data complete send */
	while( (response & (1 << 3 )) == 0 )
	{
		DRV_Lora_ReadRegister(REG_IRQ_FLAGS, &response);
	}
	DRV_Lora_WriteRegister(REG_IRQ_FLAGS, (1<<3));
	/* Read back data in irq flags*/
	DRV_Lora_ReadRegister(REG_IRQ_FLAGS, &response);
}
Std_return_type DRV_Lora_Receive(uint8_t* data, uint8_t maxSize)
{
    uint8_t l_rx_fifo_adress = 0 ;
    uint8_t l_fifo_adress    = 0 ;
    uint8_t response         = 0 ;
    uint8_t l_numOfBytes     = 0 ;
    uint8_t l_address        = 0 ;
    uint8_t l_data[20]      = {0};
	/* Switch mode to Rx continuous */
	DRV_Lora_WriteRegister(REG_OP_MODE, RECEIVE_CONTINUOUS_MODE);
	DRV_Lora_ReadRegister(REG_OP_MODE, &response);

	while( (response & (1 << 6 )) == 0 )
	{
		DRV_Lora_ReadRegister(REG_IRQ_FLAGS, &response);
	}

	/* Read payload which indicate the number of bytes */
	DRV_Lora_ReadRegister(REG_RX_NB_BYTES, &l_numOfBytes);

	/* Read the reg fifo rx current address */
	DRV_Lora_ReadRegister(REG_FIFO_RX_CURRENT_ADDR, &l_rx_fifo_adress);

	/* Setting the fifo current address by fifo rx current address */
	DRV_Lora_WriteRegister(REG_FIFO_ADDR_PTR, l_rx_fifo_adress);

	/* Read the reg fifo current address after set by fifo rx current address */
	DRV_Lora_ReadRegister(REG_FIFO_ADDR_PTR, &l_fifo_adress);

	/* Read data from fifo */
	HAL_GPIO_WritePin(LORA_CS_PIN_GPIO_Port, LORA_CS_PIN_Pin, GPIO_PIN_RESET );
	l_address = (REG_FIFO & READ_FROM_LORA);
	HAL_SPI_Transmit(&hspi1, &l_address, 1, 100);
	HAL_SPI_Receive(&hspi1, data, l_numOfBytes, 100);
	HAL_GPIO_WritePin(LORA_CS_PIN_GPIO_Port, LORA_CS_PIN_Pin, GPIO_PIN_SET );

	/* Clear irq flag for rx */
	DRV_Lora_WriteRegister(REG_IRQ_FLAGS, (1<<6));

	/* Read back data in irq flags*/
	DRV_Lora_ReadRegister(REG_IRQ_FLAGS, &response);

	return E_OK;
}

uint8_t DRV_Lora_GetVersionInfor()
{
	uint8_t version = 0 ;
	DRV_Lora_ReadRegister(REG_VERSION,&version);
	return version ;
}
static void DRV_Lora_ReadRegister(const uint8_t reg, uint8_t *data)
{
	uint8_t l_reg = 0 ;
	l_reg = (reg) & READ_FROM_LORA ;
	/* Set CS to low lever*/
	HAL_GPIO_WritePin(LORA_CS_PIN_GPIO_Port, LORA_CS_PIN_Pin, GPIO_PIN_RESET );
	/* Transmit address which want to read data on MOSI */
	HAL_SPI_Transmit(&hspi1, &l_reg , sizeof(uint8_t), 1000);
	/* Read data which transmit for master on MISO */
	HAL_SPI_Receive(&hspi1, data, sizeof(uint8_t), 1000);
	/* Set CS to high lever */
	HAL_GPIO_WritePin(LORA_CS_PIN_GPIO_Port, LORA_CS_PIN_Pin, GPIO_PIN_SET );
}
static void DRV_Lora_WriteRegister(const uint8_t reg, uint8_t data)
{
	uint8_t l_reg = 0 ;
	l_reg = (reg) | WRITE_TO_LORA ;
	/* Set CS to low lever*/
	HAL_GPIO_WritePin(LORA_CS_PIN_GPIO_Port, LORA_CS_PIN_Pin, GPIO_PIN_RESET );
	/* Transmit address which want to write data for MOSI */
	HAL_SPI_Transmit(&hspi1, &l_reg , sizeof(uint8_t), 1000);
	/*
	 * Write data to reg in Lora
	 */
	HAL_SPI_Transmit(&hspi1, &data, sizeof(uint8_t), 1000);
	/* Read data which transmit for master on MISO */
	/* Set CS to high lever */
	HAL_GPIO_WritePin(LORA_CS_PIN_GPIO_Port, LORA_CS_PIN_Pin, GPIO_PIN_SET );
}
