/**
 * @file DRV_Lora.c
 * @brief LoRa/LoRaWAN Driver Implementation for SX1278 Module
 * @details This file implements the LoRa driver functions for SX1278 transceiver
 *          including initialization, mode switching, transmission, and reception
 */

#include "DRV_Lora.h"

/* ============================================================================
 * STATIC FUNCTION PROTOTYPES
 * ============================================================================ */

/**
 * @brief Read a single register from SX1278
 * @in LORA_Instance Pointer to LoRa handle structure
 * @in reg Register address to read from
 * @in data Pointer to store the read data
 * @output STD_E_OK if read successful, STD_E_NOT_OK otherwise
 */
static LORA_ReturnTypedef DRV_Lora_ReadRegister(LORA_HandleTypedef *LORA_Instance, const uint8_t reg, uint8_t *data);

/**
 * @brief Write a single byte to SX1278 register
 * @in LORA_Instance Pointer to LoRa handle structure
 * @in reg Register address to write to
 * @in data Data byte to write
 * @output STD_E_OK if write successful, STD_E_NOT_OK otherwise
 */
static LORA_ReturnTypedef DRV_Lora_WriteRegister(LORA_HandleTypedef *LORA_Instance, const uint8_t reg, uint8_t data);

/**
 * @brief Read multiple bytes from FIFO buffer
 * @in LORA_Instance Pointer to LoRa handle structure
 * @in data Pointer to buffer for received data
 * @in length Number of bytes to read from FIFO
 * @output STD_E_OK if read successful, STD_E_NOT_OK otherwise
 */
static LORA_ReturnTypedef DRV_Lora_ReadFifo(LORA_HandleTypedef *LORA_Instance, uint8_t* data, uint16_t length);

/**
 * @brief Write multiple bytes to FIFO buffer
 * @in LORA_Instance Pointer to LoRa handle structure
 * @in data Pointer to data to write
 * @in length Number of bytes to write to FIFO
 * @output STD_E_OK if write successful, STD_E_NOT_OK otherwise
 */
static LORA_ReturnTypedef DRV_Lora_WriteFifo(LORA_HandleTypedef *LORA_Instance, uint8_t* data, uint16_t length);

/* ============================================================================
 * PUBLIC FUNCTION DEFINITIONS
 * ============================================================================ */

/**
 * @brief Initialize the LoRa module
 * @details Performs complete initialization sequence:
 *          1. Hardware reset via GPIO
 *          2. Set LoRa mode (vs FSK mode)
 *          3. Configure modem parameters (BW, CR, SF)
 *          4. Set operating frequency
 *          5. Configure power amplifier
 *          6. Setup interrupts (if enabled)
 *          7. Verify chip version
 * @in LORA_Instance Pointer to LoRa handle structure with configuration
 * @output STD_E_OK if initialization successful, STD_E_NOT_OK otherwise
 */
LORA_ReturnTypedef DRV_LoraInit(LORA_HandleTypedef* LORA_Instance)
{
	uint8_t Config = 0;
	LORA_ReturnTypedef retVal = STD_E_OK;
	
	/* ========================================================================
	 * HARDWARE RESET SEQUENCE
	 * ======================================================================== */
	/* Reset LORA module using hardware reset pin */
	HAL_GPIO_WritePin(LORA_Instance->ResetPort, LORA_Instance->ResetPin, GPIO_PIN_RESET);
	HAL_Delay(2);  /* Hold reset for 2ms */
	HAL_GPIO_WritePin(LORA_Instance->ResetPort, LORA_Instance->ResetPin, GPIO_PIN_SET);
	HAL_Delay(20); /* Wait for module to stabilize */

	/* ========================================================================
	 * MODE CONFIGURATION
	 * ======================================================================== */
	/* Move to sleep state (required before changing between LoRa/FSK modes) */
	DRV_Lora_WriteRegister(LORA_Instance, REG_OP_MODE, LORA_SLEEP_STATE);

	/* Switch to LoRa mode (bit 7 = 1 for LoRa, 0 for FSK/OOK) */
	DRV_Lora_WriteRegister(LORA_Instance, REG_OP_MODE, LORA_MODE_SLEEP | (1 << 7));

	/* ========================================================================
	 * MODEM CONFIGURATION
	 * ======================================================================== */
	/* Configure LoRa modem parameters */
	
	/**
	 * RegModemConfig1 (0x1D):
	 * - Bits 7-4: Bandwidth
	 * - Bits 3-1: Coding Rate
	 * - Bit 0: Implicit Header Mode (0 = explicit header ON)
	 */
	DRV_Lora_WriteRegister(LORA_Instance, REG_MODEM_CONFIG_1, 
	                       (uint8_t)(LORA_Instance->Bandwidth << 4) | 
	                       (LORA_Instance->CodingRate << 1));

	/**
	 * RegModemConfig2 (0x1E):
	 * - Bits 7-4: Spreading Factor
	 * - Bit 2: CRC On (0x04 enables CRC for packet integrity)
	 */
	DRV_Lora_WriteRegister(LORA_Instance, REG_MODEM_CONFIG_2, 
	                       (LORA_Instance->SpreadingFactor << 4) | 0x04);

	/**
	 * RegModemConfig3 (0x26):
	 * Low Data Rate Optimization should be enabled when:
	 * - Bandwidth < 125 kHz, OR
	 * - Spreading Factor > 10
	 * This improves sensitivity at the cost of slightly increased air time
	 */
	if ((LORA_Instance->Bandwidth < LORA_BW_125_KHZ) || 
	    (LORA_Instance->SpreadingFactor > LORA_SF_10))
	{
		/* Enable both AGC (bit 3) and LowDataRateOptimize (bit 2) */
		Config = (1 << 3) | 0x04;
	}
	else
	{
		/* Enable only AGC (Automatic Gain Control) */
		Config = (1 << 3);
	}
	
	/* Write modem config 3 register */
	DRV_Lora_WriteRegister(LORA_Instance, REG_MODEM_CONFIG_3, Config);

	/* ========================================================================
	 * FREQUENCY CONFIGURATION
	 * ======================================================================== */
	/* Set RF carrier frequency based on LORA_Instance->Frequency */
	DRV_LoraConfigFrequency(LORA_Instance);

	/* ========================================================================
	 * SYNC WORD CONFIGURATION
	 * ======================================================================== */
	/**
	 * Set LoRa sync word to 0x12 (default for LoRaWAN public networks)
	 * Note: Private networks typically use 0x34
	 * This ensures only devices with matching sync words can communicate
	 */
	DRV_Lora_WriteRegister(LORA_Instance, 0x39, 0x12);

	/* ========================================================================
	 * FIFO CONFIGURATION
	 * ======================================================================== */
	/* Reset FIFO address pointers to start of buffer */
	DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_ADDR_PTR, 0x00);      /* Current FIFO pointer */
	DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_TX_BASE_ADDR, 0x00);  /* TX base address */
	DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_RX_BASE_ADDR, 0x00);  /* RX base address */

	/* ========================================================================
	 * POWER AMPLIFIER CONFIGURATION
	 * ======================================================================== */
	/**
	 * Configure PA (Power Amplifier) using PA_BOOST mode
	 * PA_BOOST uses pin PA_BOOST instead of RFO pins for higher output power
	 */
	uint8_t pa_config_val = 0;
	uint8_t pa_dac_val = 0x84; /* Default value for RegPaDac (0x4D) */
	
	switch (LORA_Instance->Power)
	{
		case LORA_POWER_LOW:
			/**
			 * Low power mode: 2 dBm
			 * Best for indoor testing and battery conservation
			 * Formula: Pout = 2 + OutputPower
			 * For 2 dBm: OutputPower = 0
			 */
			pa_config_val = 0x80 | 0x00;
			break;

		case LORA_POWER_BALANCE:
			/**
			 * Balanced power mode: 14 dBm
			 * Safe for all power sources, medium range
			 * Formula: Pout = 2 + OutputPower
			 * For 14 dBm: OutputPower = 12 (0x0C)
			 */
			pa_config_val = 0x80 | 0x0C;
			break;

		case LORA_POWER_MAX:
			/**
			 * Maximum power mode: 17 dBm
			 * Highest power for maximum range and wall penetration
			 * Formula: Pout = 2 + OutputPower
			 * For 17 dBm: OutputPower = 15 (0x0F)
			 * 
			 * Note: 20 dBm is possible by setting pa_dac_val = 0x87,
			 * but 17 dBm is the safest maximum for Ra-02 modules
			 */
			pa_config_val = 0x80 | 0x0F;
			break;

		default:
			/* Default to balanced mode if not specified */
			pa_config_val = 0x80 | 0x0C;
			break;
	}

	/* Write power configuration to PA config register */
	DRV_Lora_WriteRegister(LORA_Instance, REG_PA_CONFIG, pa_config_val);

	/* Configure PA DAC for power stability */
	DRV_Lora_WriteRegister(LORA_Instance, 0x4D, pa_dac_val);

	/* ========================================================================
	 * INTERRUPT CONFIGURATION
	 * ======================================================================== */
	/**
	 * Setup interrupt mask based on operation mode
	 * In interrupt mode, only unmask TxDone (bit 3) and RxDone (bit 6)
	 * In polling mode, unmask all interrupts
	 */
	if (LORA_Instance->OperationMode == LORA_MODE_INTERRUPT)
	{
		/* Mask all except TxDone and RxDone interrupts */
		DRV_Lora_WriteRegister(LORA_Instance, REG_IRQ_FLAGS_MASK, 
		                       0xFF & (~(1 << 3 | 1 << 6)));
	}
	else
	{
		/* Unmask all interrupts for polling mode */
		DRV_Lora_WriteRegister(LORA_Instance, REG_IRQ_FLAGS_MASK, 0x00);
	}

	/* ========================================================================
	 * CLEAR INTERRUPTS AND SET STANDBY MODE
	 * ======================================================================== */
	/* Clear all interrupt flags from initialization */
	DRV_Lora_WriteRegister(LORA_Instance, REG_IRQ_FLAGS, 0xFF);

	/* Set module to standby state (ready for TX/RX operations) */
	DRV_Lora_WriteRegister(LORA_Instance, REG_OP_MODE, LORA_STANDBY_STATE);

	/* ========================================================================
	 * VERIFY CHIP VERSION
	 * ======================================================================== */
	/**
	 * Read chip version to verify SPI communication is working
	 * SX1278 should return 0x12
	 */
	if (DRV_Lora_GetVersion(LORA_Instance) == 0x12)
	{
		retVal = STD_E_OK;
	}
	else
	{
		retVal = STD_E_NOT_OK;
	}
	
	return retVal;
}

/**
 * @brief Deinitialize the LoRa module
 * @details Performs hardware reset to put module in default state
 * @in LORA_Instance Pointer to LoRa handle structure
 * @output None
 */
void DRV_LoraDeinit(LORA_HandleTypedef* LORA_Instance)
{
	/* Hardware reset sequence */
	HAL_GPIO_WritePin(LORA_Instance->ResetPort, LORA_Instance->ResetPin, GPIO_PIN_RESET);
	HAL_Delay(2);
	HAL_GPIO_WritePin(LORA_Instance->ResetPort, LORA_Instance->ResetPin, GPIO_PIN_SET);
	HAL_Delay(20);
}

/**
 * @brief Switch LoRa module operating mode
 * @details Changes the module state with proper PLL stabilization
 *          Always transitions through standby mode to ensure stability
 * @in LORA_Instance Pointer to LoRa handle structure
 * @in LoraState Desired operating state (sleep, standby, TX, RX, etc.)
 * @output STD_E_OK if mode switch successful, STD_E_NOT_OK otherwise
 */
LORA_ReturnTypedef DRV_LoraSwitchMode(LORA_HandleTypedef* LORA_Instance, Lora_State_t LoraState)
{
	uint8_t l_currentState = 0;
	LORA_ReturnTypedef retVal = STD_E_OK;
	
	/* Read current operating mode from RegOpMode */
	DRV_Lora_ReadRegister(LORA_Instance, REG_OP_MODE, &l_currentState);

	/**
	 * Check if the requested mode is different from current mode
	 * Mask with 0x07 to check only the mode bits (bits 2-0)
	 */
	if ((l_currentState & 0x07) != LoraState)
	{
		/**
		 * Force device into Standby mode before switching to new mode
		 * This ensures the PLL (Phase-Locked Loop) stabilizes properly
		 * 0x80 maintains Bit 7 (LongRangeMode = 1) for LoRa mode
		 * 0x01 sets Standby state
		 */
		DRV_Lora_WriteRegister(LORA_Instance, REG_OP_MODE, 0x80 | 0x01);
		HAL_Delay(5); /* Wait for PLL to stabilize */
		
		/* Set the new requested operating mode (keeping LoRa mode bit active) */
		DRV_Lora_WriteRegister(LORA_Instance, REG_OP_MODE, 0x80 | LoraState);
		HAL_Delay(5); /* Wait for mode transition */
		
		/* Verify the mode change was successful */
		DRV_Lora_ReadRegister(LORA_Instance, REG_OP_MODE, &l_currentState);
		if ((l_currentState & 0x07) != LoraState)
		{
			retVal = STD_E_NOT_OK;
		}
	}

	return retVal;
}

/**
 * @brief Transmit data via LoRa
 * @details Performs complete transmission sequence:
 *          1. Switch to standby mode
 *          2. Reset FIFO pointers
 *          3. Write data to FIFO
 *          4. Set payload length
 *          5. Switch to TX mode
 *          6. Wait for TxDone interrupt/flag
 * @in LORA_Instance Pointer to LoRa handle structure
 * @in PtrSourceData Pointer to data buffer to transmit
 * @in length Number of bytes to transmit (max 255)
 * @in timeOut Timeout in milliseconds
 * @output STD_E_OK if transmission successful, STD_E_NOT_OK on timeout
 */
LORA_ReturnTypedef DRV_LoraTransmit(LORA_HandleTypedef* LORA_Instance, uint8_t* PtrSourceData, uint16_t length, uint16_t timeOut)
{
	LORA_ReturnTypedef retVal = STD_E_OK;
	uint8_t l_irqStatus = 0;
	uint16_t l_timeout = timeOut;
	
	/* ========================================================================
	 * PREPARE FOR TRANSMISSION
	 * ======================================================================== */
	/* Switch to standby mode (required before TX) */
	DRV_LoraSwitchMode(LORA_Instance, LORA_STANDBY_STATE);

	/* Reset FIFO address pointers to start of TX buffer */
	DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_ADDR_PTR, 0x00);
	DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_TX_BASE_ADDR, 0x00);

	/* ========================================================================
	 * LOAD DATA INTO FIFO
	 * ======================================================================== */
	/* Write payload data to FIFO buffer */
	DRV_Lora_WriteFifo(LORA_Instance, PtrSourceData, length);

	/* Set the payload length register */
	DRV_Lora_WriteRegister(LORA_Instance, REG_PAYLOAD_LENGTH, length);

	/* ========================================================================
	 * START TRANSMISSION
	 * ======================================================================== */
	/* Switch to transmit mode (starts transmission automatically) */
	DRV_LoraSwitchMode(LORA_Instance, LORA_TRANSMIT_STATE);

	/* ========================================================================
	 * WAIT FOR TRANSMISSION COMPLETE
	 * ======================================================================== */
	if (LORA_Instance->OperationMode == LORA_MODE_POLLING)
	{
		/**
		 * Polling mode: Wait for TxDone flag (bit 3)
		 * Keep checking IRQ flags register until transmission complete or timeout
		 */
		while ((l_irqStatus & (1 << 3)) == 0 && l_timeout-- > 1)
		{
			DRV_Lora_ReadRegister(LORA_Instance, REG_IRQ_FLAGS, &l_irqStatus);
			HAL_Delay(1);
		}
		
		/* Check if timeout occurred */
		if (l_timeout == 0)
		{
			retVal = STD_E_NOT_OK;
		}
		
		/* Clear all interrupt flags */
		DRV_Lora_WriteRegister(LORA_Instance, REG_IRQ_FLAGS, 0xFF);
	}
	else
	{
		/* Avoid compiler warning for unused variable */
		(void)l_irqStatus;
	}
	
	return retVal;
}

/**
 * @brief Receive data via LoRa
 * @details Performs complete reception sequence:
 *          1. Switch to RX continuous mode (polling) or use IRQ (interrupt)
 *          2. Wait for RxDone interrupt/flag
 *          3. Read RX FIFO current address
 *          4. Read payload length
 *          5. Read data from FIFO
 * @in LORA_Instance Pointer to LoRa handle structure
 * @in PtrDestinationData Pointer to buffer for received data
 * @in length Maximum buffer size (not used, actual length read from register)
 * @in timeOut Timeout in milliseconds
 * @output STD_E_OK if reception successful, STD_E_NOT_OK on timeout
 */
LORA_ReturnTypedef DRV_LoraReceive(LORA_HandleTypedef* LORA_Instance, uint8_t* PtrDestinationData, uint16_t length, uint16_t timeOut)
{
	LORA_ReturnTypedef retVal = STD_E_OK;
	uint8_t l_irqStatus = 0;
	uint8_t l_rx_address = 0;
	uint8_t l_length = 0;
	uint16_t l_timeout = timeOut;
	
	/* ========================================================================
	 * RECEIVE DATA BASED ON OPERATION MODE
	 * ======================================================================== */
	if (LORA_Instance->OperationMode == LORA_MODE_POLLING)
	{
		/* Switch to continuous receive mode */
		DRV_LoraSwitchMode(LORA_Instance, LORA_RECEIVE_CONTINUOUS_STATE);

		/**
		 * Wait for RxDone flag (bit 6)
		 * Poll IRQ flags register until packet received or timeout
		 */
		while ((l_irqStatus & (1 << 6)) == 0 && l_timeout-- > 1)
		{
			DRV_Lora_ReadRegister(LORA_Instance, REG_IRQ_FLAGS, &l_irqStatus);
			HAL_Delay(1);
		}
		
		/* Check if timeout occurred */
		if (l_timeout == 0)
		{
			retVal = STD_E_NOT_OK;
		}
		else
		{
			/* ================================================================
			 * READ RECEIVED DATA FROM FIFO
			 * ================================================================ */
			/**
			 * Read the address where received packet starts in FIFO
			 * This is set by the modem automatically
			 */
			DRV_Lora_ReadRegister(LORA_Instance, REG_FIFO_RX_CURRENT_ADDR, &l_rx_address);

			/* Point FIFO address pointer to start of received packet */
			DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_ADDR_PTR, l_rx_address);

			/* Read actual payload length of received packet */
			DRV_Lora_ReadRegister(LORA_Instance, REG_RX_NB_BYTES, &l_length);

			/* Read data from FIFO buffer */
			DRV_Lora_ReadFifo(LORA_Instance, PtrDestinationData, l_length);

			/* Reset RX FIFO base address for next reception */
			DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_RX_BASE_ADDR, 0x00);

			/* Clear all interrupt flags */
			DRV_Lora_WriteRegister(LORA_Instance, REG_IRQ_FLAGS, 0xFF);
		}
	}
	else
	{
		/**
		 * Interrupt mode: Data already received (triggered by IRQ)
		 * Just read the data from FIFO
		 */
		
		/* Read FIFO address where packet starts */
		DRV_Lora_ReadRegister(LORA_Instance, REG_FIFO_RX_CURRENT_ADDR, &l_rx_address);

		/* Set FIFO pointer to received packet location */
		DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_ADDR_PTR, l_rx_address);

		/* Read payload length */
		DRV_Lora_ReadRegister(LORA_Instance, REG_RX_NB_BYTES, &l_length);

		/* Read data from FIFO */
		DRV_Lora_ReadFifo(LORA_Instance, PtrDestinationData, l_length);

		/* Reset RX FIFO base address */
		DRV_Lora_WriteRegister(LORA_Instance, REG_FIFO_RX_BASE_ADDR, 0x00);

		/* Clear interrupt flags */
		DRV_Lora_WriteRegister(LORA_Instance, REG_IRQ_FLAGS, 0xFF);
		
		/* Avoid compiler warning */
		(void)l_irqStatus;
	}
	
	return retVal;
}

/**
 * @brief Configure LoRa operating frequency
 * @details Calculates and sets the RF carrier frequency using the formula:
 *          FRF = (Frequency * 2^19) / F_XOSC
 *          where F_XOSC = 32 MHz for SX1278
 *          Simplified: FRF = Frequency * 0.016384
 * @in LORA_Instance Pointer to LoRa handle structure (contains Frequency field)
 * @output STD_E_OK (always successful in current implementation)
 */
LORA_ReturnTypedef DRV_LoraConfigFrequency(LORA_HandleTypedef* LORA_Instance)
{
	LORA_ReturnTypedef retVal = STD_E_OK;
	
	/**
	 * Calculate frequency register value
	 * Example: For 433 MHz:
	 * FRF = 433000000 * 0.016384 = 7094272 = 0x6C4000
	 */
	uint32_t frf = (uint32_t)((float)LORA_Instance->Frequency * 0.016384f);
	
	/* Write frequency to 3 separate registers (MSB, MID, LSB) */
	DRV_Lora_WriteRegister(LORA_Instance, 0x06, (uint8_t)(frf >> 16));  /* FRF_MSB */
	DRV_Lora_WriteRegister(LORA_Instance, 0x07, (uint8_t)(frf >> 8));   /* FRF_MID */
	DRV_Lora_WriteRegister(LORA_Instance, 0x08, (uint8_t)(frf >> 0));   /* FRF_LSB */
	
	return retVal;
}

/**
 * @brief Configure LoRa interrupt settings
 * @details Placeholder function for additional interrupt configuration
 *          Currently not implemented - interrupts configured in Init function
 * @in LORA_Instance Pointer to LoRa handle structure
 * @output STD_E_OK (placeholder)
 */
LORA_ReturnTypedef DRV_LoraConfigInterrupt(LORA_HandleTypedef* LORA_Instance)
{
	LORA_ReturnTypedef retVal = STD_E_OK;
	/* Function not yet implemented */
	return retVal;
}

/**
 * @brief LoRa interrupt handler
 * @details Placeholder for interrupt service routine
 *          Should be called from EXTI callback when DIO0 triggers
 *          Implementation should:
 *          1. Read IRQ flags
 *          2. Check TxDone/RxDone flags
 *          3. Call appropriate callbacks
 *          4. Clear IRQ flags
 * @in LORA_Instance Pointer to LoRa handle structure
 * @output STD_E_OK (placeholder)
 */
LORA_ReturnTypedef DRV_Lora_IRQHandler(LORA_HandleTypedef* LORA_Instance)
{
	LORA_ReturnTypedef retVal = STD_E_OK;
	/* Function not yet implemented */
	/* TODO: Implement interrupt handling logic */
	return retVal;
}

/**
 * @brief Get SX1278 chip version
 * @details Reads the version register to verify chip communication
 *          SX1278 should return 0x12
 *          Useful for debugging SPI communication issues
 * @in LORA_Instance Pointer to LoRa handle structure
 * @output Chip version number (0x12 for SX1278)
 */
uint8_t DRV_Lora_GetVersion(LORA_HandleTypedef* LORA_Instance)
{
	uint8_t retVal = 0;

	/* Read version register (should return 0x12 for SX1278) */
	DRV_Lora_ReadRegister(LORA_Instance, REG_VERSION, &retVal);

	return retVal;
}

/* ============================================================================
 * STATIC FUNCTION DEFINITIONS (LOW-LEVEL SPI COMMUNICATION)
 * ============================================================================ */

/**
 * @brief Read a single register from SX1278 via SPI
 * @details SPI transaction sequence:
 *          1. Pull CS low
 *          2. Send register address with MSB=0 (read operation)
 *          3. Read one byte from MISO
 *          4. Pull CS high
 * @in LORA_Instance Pointer to LoRa handle structure
 * @in reg Register address to read (0x00-0x7F)
 * @in data Pointer to store the read byte
 * @output STD_E_OK if successful, STD_E_NOT_OK on SPI error
 */
static LORA_ReturnTypedef DRV_Lora_ReadRegister(LORA_HandleTypedef *LORA_Instance, const uint8_t reg, uint8_t *data)
{
	HAL_StatusTypeDef TransmitStatus = HAL_OK;
	LORA_ReturnTypedef retVal = STD_E_OK;
	
	/* Prepare register address for read (MSB = 0) */
	uint8_t l_register = LORA_READ_REG(reg);
	
	/* Assert chip select (active low) */
	HAL_GPIO_WritePin(LORA_Instance->ChipSelectPort, LORA_Instance->ChipSelectPin, GPIO_PIN_RESET);

	/* Send register address on MOSI */
	TransmitStatus = HAL_SPI_Transmit(LORA_Instance->SPI_Instance, &l_register, 1, 10);

	/* Read data byte from MISO */
	if (TransmitStatus == HAL_OK)
	{
		TransmitStatus = HAL_SPI_Receive(LORA_Instance->SPI_Instance, data, 1, 10);
		if (TransmitStatus != HAL_OK)
		{
			retVal = STD_E_NOT_OK;
		}
	}
	else
	{
		retVal = STD_E_NOT_OK;
	}

	/* Deassert chip select */
	HAL_GPIO_WritePin(LORA_Instance->ChipSelectPort, LORA_Instance->ChipSelectPin, GPIO_PIN_SET);

	return retVal;
}

/**
 * @brief Write a single byte to SX1278 register via SPI
 * @details SPI transaction sequence:
 *          1. Pull CS low
 *          2. Send register address with MSB=1 (write operation)
 *          3. Send data byte on MOSI
 *          4. Pull CS high
 * @in LORA_Instance Pointer to LoRa handle structure
 * @in reg Register address to write (0x00-0x7F)
 * @in data Data byte to write
 * @output STD_E_OK if successful, STD_E_NOT_OK on SPI error
 */
static LORA_ReturnTypedef DRV_Lora_WriteRegister(LORA_HandleTypedef *LORA_Instance, const uint8_t reg, uint8_t data)
{
	HAL_StatusTypeDef TransmitStatus = HAL_OK;
	LORA_ReturnTypedef retVal = STD_E_OK;
	
	/* Prepare register address for write (MSB = 1) */
	uint8_t l_register = LORA_WRITE_REG(reg);
	
	/* Assert chip select (active low) */
	HAL_GPIO_WritePin(LORA_Instance->ChipSelectPort, LORA_Instance->ChipSelectPin, GPIO_PIN_RESET);

	/* Send register address */
	TransmitStatus = HAL_SPI_Transmit(LORA_Instance->SPI_Instance, &l_register, 1, 10);

	/* Send data byte */
	if (TransmitStatus == HAL_OK)
	{
		TransmitStatus = HAL_SPI_Transmit(LORA_Instance->SPI_Instance, &data, 1, 10);
		if (TransmitStatus != HAL_OK)
		{
			retVal = STD_E_NOT_OK;
		}
	}
	else
	{
		retVal = STD_E_NOT_OK;
	}

	/* Deassert chip select */
	HAL_GPIO_WritePin(LORA_Instance->ChipSelectPort, LORA_Instance->ChipSelectPin, GPIO_PIN_SET);

	return retVal;
}

/**
 * @brief Read multiple bytes from FIFO buffer via SPI
 * @details FIFO is accessed through register 0x00
 *          Auto-increments internal pointer after each byte read
 *          Used for reading received packet data
 * @in LORA_Instance Pointer to LoRa handle structure
 * @in data Pointer to buffer for storing read data
 * @in length Number of bytes to read from FIFO
 * @output STD_E_OK if successful, STD_E_NOT_OK on SPI error
 */
static LORA_ReturnTypedef DRV_Lora_ReadFifo(LORA_HandleTypedef *LORA_Instance, uint8_t* data, uint16_t length)
{
	HAL_StatusTypeDef TransmitStatus = HAL_OK;
	LORA_ReturnTypedef retVal = STD_E_OK;
	
	/* FIFO register address for read operation */
	uint8_t l_register = LORA_READ_REG(REG_FIFO);

	/* Assert chip select */
	HAL_GPIO_WritePin(LORA_Instance->ChipSelectPort, LORA_Instance->ChipSelectPin, GPIO_PIN_RESET);

	/* Send FIFO register address */
	TransmitStatus = HAL_SPI_Transmit(LORA_Instance->SPI_Instance, &l_register, 1, 10);

	/* Read multiple bytes from FIFO */
	if (TransmitStatus == HAL_OK)
	{
		TransmitStatus = HAL_SPI_Receive(LORA_Instance->SPI_Instance, data, length, 10);
		if (TransmitStatus != HAL_OK)
		{
			retVal = STD_E_NOT_OK;
		}
	}
	else
	{
		retVal = STD_E_NOT_OK;
	}

	/* Deassert chip select */
	HAL_GPIO_WritePin(LORA_Instance->ChipSelectPort, LORA_Instance->ChipSelectPin, GPIO_PIN_SET);

	return retVal;
}

/**
 * @brief Write multiple bytes to FIFO buffer via SPI
 * @details FIFO is accessed through register 0x00
 *          Auto-increments internal pointer after each byte written
 *          Used for loading packet data before transmission
 * @in LORA_Instance Pointer to LoRa handle structure
 * @in data Pointer to data buffer to write
 * @in length Number of bytes to write to FIFO
 * @output STD_E_OK if successful, STD_E_NOT_OK on SPI error
 */
static LORA_ReturnTypedef DRV_Lora_WriteFifo(LORA_HandleTypedef *LORA_Instance, uint8_t* data, uint16_t length)
{
	HAL_StatusTypeDef TransmitStatus = HAL_OK;
	LORA_ReturnTypedef retVal = STD_E_OK;
	
	/* FIFO register address for write operation */
	uint8_t l_register = LORA_WRITE_REG(REG_FIFO);

	/* Assert chip select */
	HAL_GPIO_WritePin(LORA_Instance->ChipSelectPort, LORA_Instance->ChipSelectPin, GPIO_PIN_RESET);

	/* Send FIFO register address */
	TransmitStatus = HAL_SPI_Transmit(LORA_Instance->SPI_Instance, &l_register, 1, 10);

	/* Write multiple bytes to FIFO */
	if (TransmitStatus == HAL_OK)
	{
		TransmitStatus = HAL_SPI_Transmit(LORA_Instance->SPI_Instance, data, length, 10);
		if (TransmitStatus != HAL_OK)
		{
			retVal = STD_E_NOT_OK;
		}
	}
	else
	{
		retVal = STD_E_NOT_OK;
	}

	/* Deassert chip select */
	HAL_GPIO_WritePin(LORA_Instance->ChipSelectPort, LORA_Instance->ChipSelectPin, GPIO_PIN_SET);

	return retVal;
}
/**
 * @brief Read the current interrupt flags from the SX1278
 * @details This function retrieves the status of all 8 interrupt sources
 * available in LoRa mode from the REG_IRQ_FLAGS (0x12) register.
 * Common flags include RxDone (Bit 6), TxDone (Bit 3), and PayloadCrcError (Bit 5).
 * * @in LORA_Instance Pointer to LoRa handle structure
 * @output uint8_t Current value of the IRQ flags register
 */
uint8_t DRV_Lora_GetIrqStatus(LORA_HandleTypedef *LORA_Instance)
{
    uint8_t irqFlags = 0;

    /* Read the IRQ flags register (Address 0x12) */
    DRV_Lora_ReadRegister(LORA_Instance, REG_IRQ_FLAGS, &irqFlags);

    return irqFlags;
}

/**
 * @brief Clear a specific interrupt flag by its ID/Mask
 * @details In the SX127x series, interrupt flags are cleared by writing a
 * logical '1' to the corresponding bit position in REG_IRQ_FLAGS.
 * Gently clearing individual flags prevents losing other pending interrupts.
 * * @in LORA_Instance Pointer to LoRa handle structure
 * @in interruptMask Bit mask representing the flag to be cleared:
 * - 0x40: RX_DONE
 * - 0x08: TX_DONE
 * - 0x20: PAYLOAD_CRC_ERROR
 * - 0x80: RX_TIMEOUT
 * * @output LORA_ReturnTypedef STD_E_OK if write was successful
 */
LORA_ReturnTypedef DRV_Lora_ClearIrq(LORA_HandleTypedef *LORA_Instance, uint8_t interruptMask)
{
    /**
     * Note: Writing '1' clears the bit.
     * If you want to clear only RxDone, pass (1 << 6).
     */
    return DRV_Lora_WriteRegister(LORA_Instance, REG_IRQ_FLAGS, interruptMask);
}
