#include "DRV_Lora.h"

/*
 * Static prototype function
 */
static LORA_ReturnTypedef DRV_Lora_ReadRegister(LORA_HandleTypedef *LORA_Instance,const uint8_t reg, uint8_t *data);
static LORA_ReturnTypedef DRV_Lora_WriteRegister(LORA_HandleTypedef *LORA_Instance,const uint8_t reg, uint8_t data);
static LORA_ReturnTypedef DRV_Lora_ReadFifo(LORA_HandleTypedef *LORA_Instance, uint8_t* data, uint16_t length);
static LORA_ReturnTypedef DRV_Lora_WriteFifo(LORA_HandleTypedef *LORA_Instance, uint8_t* data, uint16_t length);
/*
 * Defintion function
 */
LORA_ReturnTypedef DRV_LoraInit(LORA_HandleTypedef* LORA_Instance)
{
	uint8_t Config = 0 ;
	LORA_ReturnTypedef retVal = STD_E_OK;
    /* Reset LORA */
    HAL_GPIO_WritePin(LORA_Instance->ResetPort,LORA_Instance->ResetPin,GPIO_PIN_RESET);
    HAL_Delay(2);
    HAL_GPIO_WritePin(LORA_Instance->ResetPort,LORA_Instance->ResetPin,GPIO_PIN_SET);
    HAL_Delay(20);

    /* Move to sleep state */
    DRV_Lora_WriteRegister(LORA_Instance,REG_OP_MODE,LORA_SLEEP_STATE);

    /* Move to LoRa mode */
    DRV_Lora_WriteRegister(LORA_Instance,REG_OP_MODE,LORA_MODE_SLEEP | ( 1 << 7 ));

    /* Configure LoRa parameters */
    // RegModemConfig1: Bandwidth | CodingRate | ImplicitHeaderOff
	DRV_Lora_WriteRegister(LORA_Instance, REG_MODEM_CONFIG_1, (uint8_t) (LORA_Instance->Bandwidth << 4) | (LORA_Instance->CodingRate << 1));

	// RegModemConfig2: SpreadingFactor | CrcOn
	DRV_Lora_WriteRegister(LORA_Instance, REG_MODEM_CONFIG_2, (LORA_Instance->SpreadingFactor << 4) | 0x04);
	if ((LORA_Instance->Bandwidth < LORA_BW_125_KHZ) || (LORA_Instance->SpreadingFactor > LORA_SF_10))
	{
		Config = ( 1 << 3 ) | 0x04 ;
	}
	else
	{
		Config = ( 1 << 3);
	}
	// RegModemConfig3: Config AGC and LowDataRateOptimize
	DRV_Lora_WriteRegister(LORA_Instance, REG_MODEM_CONFIG_3, Config);

    /* Config frequency */
    DRV_LoraConfigFrequency(LORA_Instance);

    /* Config sync word */
    DRV_Lora_WriteRegister(LORA_Instance, 0x39, 0x12);

    /* Reset address pointer */
	DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_ADDR_PTR, 0x00); // RegFifoAddrPtr
	DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_TX_BASE_ADDR, 0x00); // RegFifoTxBaseAddr
	DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_RX_BASE_ADDR, 0x00); // RegFifoRxBaseAddr

    /* Enable PA Boost, Max Power */
	/* --- Configure Power (PA_BOOST Mode) --- */
	uint8_t pa_config_val = 0;
	uint8_t pa_dac_val = 0x84; // Giá trị mặc định cho RegPaDac (0x4D)
	switch (LORA_Instance->Power)
	{
	    case LORA_POWER_LOW:
	        // Thiết lập 2 dBm (Mức thấp nhất để test trong phòng)
	        // Pout = 2 + OutputPower => OutputPower = 0
	        pa_config_val = 0x80 | 0x00;
	        break;

	    case LORA_POWER_BALANCE:
	        // Thiết lập 14 dBm (Mức trung bình, an toàn cho mọi nguồn điện)
	        // Pout = 2 + OutputPower => OutputPower = 12 (0x0C)
	        pa_config_val = 0x80 | 0x0C;
	        break;

	    case LORA_POWER_MAX:
	        // Thiết lập 17 dBm (Mức mạnh nhất thông thường)
	        // Pout = 2 + OutputPower => OutputPower = 15 (0x0F)
	        pa_config_val = 0x80 | 0x0F;

	        /* Ghi chú: Nếu muốn lên 20dBm, cần set pa_dac_val = 0x87 nhưng 17dBm là mức an toàn nhất cho module Ra-02 */
	        break;

	    default:
	        // Mặc định chọn Balance nếu người dùng quên set
	        pa_config_val = 0x80 | 0x0C;
	        break;
	}

	// Ghi vào thanh ghi cấu hình Power
	DRV_Lora_WriteRegister(LORA_Instance, REG_PA_CONFIG, pa_config_val);

	// Cấu hình thêm RegPaDac để đảm bảo độ ổn định công suất
	DRV_Lora_WriteRegister(LORA_Instance, 0x4D, pa_dac_val);

    /* Setup for interrupt using pin DIO0 */
    if (LORA_Instance->OperationMode == LORA_MODE_INTERRUPT)
    {
        DRV_Lora_WriteRegister(LORA_Instance,REG_IRQ_FLAGS_MASK, 0xFF & (~(1 <<3 | 1 << 6)));
    }
    else
    {
        DRV_Lora_WriteRegister(LORA_Instance,REG_IRQ_FLAGS_MASK, 0x00 );
    }


    /* Clear IRQ */
	DRV_Lora_WriteRegister(LORA_Instance, REG_IRQ_FLAGS, 0xFF);

    /* Set state to standby */
    DRV_Lora_WriteRegister(LORA_Instance,REG_OP_MODE, LORA_STANDBY_STATE);

    if ( DRV_Lora_GetVersion(LORA_Instance) == 0x12)
    {
    	retVal = STD_E_OK;
    }
    else
    {
    	retVal = STD_E_NOT_OK;
    }
    return retVal;
}
void DRV_LoraDeinit(LORA_HandleTypedef* LORA_Instance)
{
    /* Reset LORA */
    HAL_GPIO_WritePin(LORA_Instance->ResetPort,LORA_Instance->ResetPin,GPIO_PIN_RESET);
    HAL_Delay(2);
    HAL_GPIO_WritePin(LORA_Instance->ResetPort,LORA_Instance->ResetPin,GPIO_PIN_SET);
    HAL_Delay(20);
}

LORA_ReturnTypedef DRV_LoraSwitchMode(LORA_HandleTypedef* LORA_Instance, Lora_State_t LoraState)
{
    uint8_t l_currentState = 0;
    LORA_ReturnTypedef retVal = STD_E_OK;
    /* Read current operating mode */
    DRV_Lora_ReadRegister(LORA_Instance, REG_OP_MODE, &l_currentState);

    /* Check if the requested mode is different from the current mode (masking last 3 bits) */
    if ((l_currentState & 0x07) != LoraState)
    {
        /* * Force the device into Standby mode before switching to the new mode.
         * This ensures the PLL (Phase-Locked Loop) stabilizes.
         * 0x80 is used to maintain Bit 7 (LongRangeMode = 1) for LoRa mode.
         */
        DRV_Lora_WriteRegister(LORA_Instance, REG_OP_MODE, 0x80 | 0x01);
        HAL_Delay(5);
        /* Set the new requested operating mode (keeping LoRa mode bit active) */
        DRV_Lora_WriteRegister(LORA_Instance, REG_OP_MODE, 0x80 | LoraState);
        HAL_Delay(5);
        DRV_Lora_ReadRegister(LORA_Instance, REG_OP_MODE, &l_currentState);
        if ((l_currentState & 0x07) != LoraState )
        {
        	retVal = STD_E_NOT_OK;
        }
    }

    return retVal;
}
LORA_ReturnTypedef DRV_LoraTransmit(LORA_HandleTypedef* LORA_Instance, uint8_t* PtrSourceData, uint16_t length, uint16_t timeOut)
{
	LORA_ReturnTypedef retVal = STD_E_OK ;
	uint8_t l_irqStatus = 0 ;
	uint16_t l_timeout  = timeOut ;
	/* Switch mode of LoRa to standby  */
	DRV_LoraSwitchMode(LORA_Instance, LORA_STANDBY_STATE);

    /* Reset address pointer */
	DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_ADDR_PTR, 0x00); // RegFifoAddrPtr
	DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_TX_BASE_ADDR, 0x00); // RegFifoTxBaseAddr

	/* Write data into fifo */
	DRV_Lora_WriteFifo(LORA_Instance, PtrSourceData, length);

	/* Set number of byte to transmit */
	DRV_Lora_WriteRegister(LORA_Instance, REG_PAYLOAD_LENGTH, length);

	/* Move state to transmit */
	DRV_LoraSwitchMode(LORA_Instance, LORA_TRANSMIT_STATE);

	/* Polling bit TX Transmit done */
	if(LORA_Instance->OperationMode == LORA_MODE_POLLING)
	{
		while( (l_irqStatus & (1 << 3)) == 0 && l_timeout -- > 1 )
		{
			DRV_Lora_ReadRegister(LORA_Instance, REG_IRQ_FLAGS, &l_irqStatus);
			HAL_Delay(1);
		}
		if ( l_timeout == 0 )
		{
			retVal = STD_E_NOT_OK;
		}
		/* Clear IRQ previous state */
		DRV_Lora_WriteRegister(LORA_Instance, REG_IRQ_FLAGS, 0xFF);
	}
	else
	{
		while( (l_irqStatus & (1 << 3)) == 0 && l_timeout -- > 1 )
		{
			DRV_Lora_ReadRegister(LORA_Instance, REG_IRQ_FLAGS, &l_irqStatus);
			HAL_Delay(1);
		}
		if ( l_timeout == 0 )
		{
			retVal = STD_E_NOT_OK;
		}
		/* Clear IRQ previous state */
		DRV_Lora_WriteRegister(LORA_Instance, REG_IRQ_FLAGS, 0xFF);

		/* @brief : advoid compiler warning */
		(void)l_irqStatus;
	}
	return retVal;
}
LORA_ReturnTypedef DRV_LoraReceive(LORA_HandleTypedef* LORA_Instance, uint8_t* PtrDestinationData, uint16_t length, uint16_t timeOut)
{
	LORA_ReturnTypedef retVal = STD_E_OK ;
	uint8_t l_irqStatus = 0 ;
	uint8_t l_rx_address = 0 ;
	uint8_t l_length = 0 ;
	uint16_t l_timeout = timeOut ;
	/* Polling bit TX Transmit done */
	if(LORA_Instance->OperationMode == LORA_MODE_POLLING)
	{
		/* Switch mode of LoRa to RX_Continuos  */
		DRV_LoraSwitchMode(LORA_Instance, LORA_RECEIVE_CONTINUOUS_STATE);

		while( (l_irqStatus & (1 << 6)) == 0 && l_timeout -- > 1)
		{
			DRV_Lora_ReadRegister(LORA_Instance, REG_IRQ_FLAGS, &l_irqStatus);
			HAL_Delay(1);
		}
		if (l_timeout == 0 )
		{
			retVal = STD_E_NOT_OK;
		}
		else
		{
			/* Read address of Rx_Base_Addres_Ptr*/
			DRV_Lora_ReadRegister(LORA_Instance, REG_FIFO_RX_CURRENT_ADDR, &l_rx_address);

			/* Set ptr of Fifo_Address Ptr */
			DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_ADDR_PTR, l_rx_address);

			/* Read payload of current data */
			DRV_Lora_ReadRegister(LORA_Instance, REG_RX_NB_BYTES, &l_length);

			/* Read data from fifo  */
			DRV_Lora_ReadFifo(LORA_Instance, PtrDestinationData, l_length);

			/* Reset fifo rx base address */
			DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_RX_BASE_ADDR, 0x00); // RegFifoRxBaseAddr

			/* Clear IRQ previous state */
			DRV_Lora_WriteRegister(LORA_Instance, REG_IRQ_FLAGS, 0xFF);
		}
	}
	else
	{
		/* Read address of Rx_Base_Addres_Ptr*/
		DRV_Lora_ReadRegister(LORA_Instance, REG_FIFO_RX_CURRENT_ADDR, &l_rx_address);

		/* Set ptr of Fifo_Address Ptr */
		DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_ADDR_PTR, l_rx_address);

		/* Read payload of current data */
		DRV_Lora_ReadRegister(LORA_Instance, REG_RX_NB_BYTES, &l_length);

		/* Read data from fifo  */
		DRV_Lora_ReadFifo(LORA_Instance, PtrDestinationData, l_length);

		/* Reset fifo rx base address */
		DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_RX_BASE_ADDR, 0x00); // RegFifoRxBaseAddr

		/* Clear IRQ previous state */
		DRV_Lora_WriteRegister(LORA_Instance, REG_IRQ_FLAGS, 0xFF);
		/* @brief : advoid compiler warning */
		(void)l_irqStatus;
	}
	return retVal;
}
LORA_ReturnTypedef DRV_LoraConfigFrequency(LORA_HandleTypedef* LORA_Instance)
{
	LORA_ReturnTypedef retVal = 0 ;
    uint32_t frf = (uint32_t)((float)LORA_Instance->Frequency * 0.016384f);
    DRV_Lora_WriteRegister(LORA_Instance, 0x06, (uint8_t)(frf >> 16));
    DRV_Lora_WriteRegister(LORA_Instance, 0x07, (uint8_t)(frf >> 8));
    DRV_Lora_WriteRegister(LORA_Instance, 0x08, (uint8_t)(frf >> 0));
    return retVal;
}
LORA_ReturnTypedef DRV_LoraConfigInterrupt(LORA_HandleTypedef* LORA_Instance)
{
	LORA_ReturnTypedef retVal = 0 ;
	return retVal;
}
LORA_ReturnTypedef DRV_Lora_IRQHandler(LORA_HandleTypedef* LORA_Instance)
{
	LORA_ReturnTypedef retVal = 0 ;
	return retVal;
}
uint8_t DRV_Lora_GetVersion(LORA_HandleTypedef* LORA_Instance)
{
    uint8_t retVal = 0 ;

    /* Read register version in Lora to get version */
    DRV_Lora_ReadRegister(LORA_Instance, REG_VERSION, &retVal);

    /* Return version */
    return retVal;
}
/*
 * Definition static function
 */
static LORA_ReturnTypedef DRV_Lora_ReadRegister(LORA_HandleTypedef *LORA_Instance, const uint8_t reg, uint8_t *data)
{
    HAL_StatusTypeDef TransmitStatus = HAL_OK;
    LORA_ReturnTypedef  retVal = STD_E_OK ;
    uint8_t l_register = LORA_READ_REG(reg);
    /* Pulll the Chip Select pin low */
    HAL_GPIO_WritePin(LORA_Instance->ChipSelectPort,LORA_Instance->ChipSelectPin,GPIO_PIN_RESET);

    /* Transmit register address which want to read from LORA on MOSI Line */
    TransmitStatus = HAL_SPI_Transmit(LORA_Instance->SPI_Instance,&l_register,1,10);

    /* Read data which come from LORA on MISO Line */
    if ( TransmitStatus == HAL_OK)
    {
        TransmitStatus = HAL_SPI_Receive(LORA_Instance->SPI_Instance,data,1,10);
        if(TransmitStatus != HAL_OK)
        {
            retVal = STD_E_NOT_OK;
        }
    }
    else
    {
        retVal = STD_E_NOT_OK;
    }

    /* Driver the Chip Select to a hight logic level */
    HAL_GPIO_WritePin(LORA_Instance->ChipSelectPort,LORA_Instance->ChipSelectPin,GPIO_PIN_SET);

    /* Return result  */
    return retVal;
}
static LORA_ReturnTypedef DRV_Lora_WriteRegister(LORA_HandleTypedef *LORA_Instance,const uint8_t reg, uint8_t data)
{
    HAL_StatusTypeDef TransmitStatus = HAL_OK;
    LORA_ReturnTypedef  retVal = STD_E_OK ;
    uint8_t l_register = LORA_WRITE_REG(reg);
    /* Pull the Chip Select pin low */
    HAL_GPIO_WritePin(LORA_Instance->ChipSelectPort,LORA_Instance->ChipSelectPin,GPIO_PIN_RESET);

    /* Transmit register address which want to read from LORA on MOSI Line */
    TransmitStatus = HAL_SPI_Transmit(LORA_Instance->SPI_Instance,&l_register,1,10);

    /* Transmit data which want to write to LORA on MISO Line */
    if ( TransmitStatus == HAL_OK)
    {
        TransmitStatus = HAL_SPI_Transmit(LORA_Instance->SPI_Instance,&data,1,10);
        if(TransmitStatus != HAL_OK)
        {
            retVal = STD_E_NOT_OK;
        }
    }
    else
    {
        retVal = STD_E_NOT_OK;
    }

    /* Driver the Chip Select to a hight logic level */
    HAL_GPIO_WritePin(LORA_Instance->ChipSelectPort,LORA_Instance->ChipSelectPin,GPIO_PIN_SET);

    /* Return result  */
    return retVal;
}
static LORA_ReturnTypedef DRV_Lora_ReadFifo(LORA_HandleTypedef *LORA_Instance, uint8_t* data, uint16_t length)
{
    HAL_StatusTypeDef TransmitStatus = HAL_OK;
    LORA_ReturnTypedef  retVal = STD_E_OK ;
    uint8_t l_register = LORA_READ_REG(REG_FIFO);

    /* Pull the Chip Select pin low */
    HAL_GPIO_WritePin(LORA_Instance->ChipSelectPort,LORA_Instance->ChipSelectPin,GPIO_PIN_RESET);

    /* Transmit register address which want to read from LORA on MOSI Line */
    TransmitStatus = HAL_SPI_Transmit(LORA_Instance->SPI_Instance,&l_register,1,10);

    /* Transmit data which want to write to LORA on MISO Line */
    if ( TransmitStatus == HAL_OK)
    {
		TransmitStatus = HAL_SPI_Receive(LORA_Instance->SPI_Instance,data,length,10);
		if(TransmitStatus != HAL_OK)
		{
			retVal = STD_E_NOT_OK;
		}
    }
    else
    {
        retVal = STD_E_NOT_OK;
    }

    /* Driver the Chip Select to a hight logic level */
    HAL_GPIO_WritePin(LORA_Instance->ChipSelectPort,LORA_Instance->ChipSelectPin,GPIO_PIN_SET);

    /* Return result  */
    return retVal;
}
static LORA_ReturnTypedef DRV_Lora_WriteFifo(LORA_HandleTypedef *LORA_Instance, uint8_t* data, uint16_t length)
{
    HAL_StatusTypeDef TransmitStatus = HAL_OK;
    LORA_ReturnTypedef  retVal = STD_E_OK ;
    uint8_t l_register = LORA_WRITE_REG(REG_FIFO);

    /* Pull the Chip Select pin low */
    HAL_GPIO_WritePin(LORA_Instance->ChipSelectPort,LORA_Instance->ChipSelectPin,GPIO_PIN_RESET);

    /* Transmit register address which want to read from LORA on MOSI Line */
    TransmitStatus = HAL_SPI_Transmit(LORA_Instance->SPI_Instance,&l_register,1,10);

    /* Transmit data which want to write to LORA on MISO Line */
    if ( TransmitStatus == HAL_OK)
    {
		TransmitStatus = HAL_SPI_Transmit(LORA_Instance->SPI_Instance,data,length,10);
		if(TransmitStatus != HAL_OK)
		{
			retVal = STD_E_NOT_OK;
		}
    }
    else
    {
        retVal = STD_E_NOT_OK;
    }

    /* Driver the Chip Select to a hight logic level */
    HAL_GPIO_WritePin(LORA_Instance->ChipSelectPort,LORA_Instance->ChipSelectPin,GPIO_PIN_SET);

    /* Return result  */
    return retVal;
}
