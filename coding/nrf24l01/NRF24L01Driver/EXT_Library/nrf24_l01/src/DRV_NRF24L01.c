#include "DRV_NRF24L01.h"
#include "stdint.h"

/*
 * Bitmask Definitions
 */
#define READ_REGISTER(x) ((x) & 0x1F)
#define WRITE_REGISTER(x) ((x) | 0x20 )

/*
 * Local prototype functions (Private helpers)
 */
static uint8_t DRV_Nrf_ReadRegister(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t reg);
static void DRV_Nrf_WriteRegister(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t reg, uint8_t data);
static void DRV_Nrf24L01ReadPayload(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t *data, uint8_t length);
static void DRV_Nrf24L01WritePayload(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t *data, uint8_t length);

/**
 * @brief Initializes the NRF24L01 module with provided configuration
 */
NRF24L01_ReturnType DRV_Nrf24l01Init(NRF24L01_HandleTypedef* NRF24L01Instance)
{
    uint8_t retval = 0;
    uint8_t counter = 0;
    uint8_t cmd    = 0;

    HAL_Delay(5); /* Wait for Power-On Reset (POR) stability */

    /* Power up and enable CRC (2-byte CRC) */
    DRV_Nrf_WriteRegister(NRF24L01Instance, CONFIG, (1 << EN_CRC) | (1 << CRCO) | (1 << PWR_UP) );
    cmd = DRV_Nrf_ReadRegister(NRF24L01Instance, CONFIG);

    /* --- Activation Sequence --- */
    /* 1. Pull CSN Low to start SPI session */
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);

    /* 2. Send ACTIVATE command followed by 0x73 to enable special features */
    cmd = ACTIVATE;
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
    cmd = 0x73;
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);

    /* 3. Pull CSN High to terminate session */
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);

    /* Setting up Payload Features */
    cmd = 0x00;
    if (NRF24L01Instance->DynamicPayloadEnable == TRUE)
    {
        cmd |= 1 << 2; /* EN_DPL bit */
        DRV_Nrf_WriteRegister(NRF24L01Instance, DYNPD, 0x3F); /* Enable DPL on all pipes */
    }
    if (NRF24L01Instance->PayloadWithAckEnable == TRUE)
    {
        cmd |= 1 << 1; /* EN_ACK_PAY bit */
    }
    /* Write enabled features to FEATURE register */
    DRV_Nrf_WriteRegister(NRF24L01Instance, FEATURE, cmd);

    /* 4. Enable Auto-ACK for all Pipes (P0-P5) */
    DRV_Nrf_WriteRegister(NRF24L01Instance, EN_AA, 0x3F);

    /* 5. Enable all RX Addresses to be ready for reception */
    DRV_Nrf_WriteRegister(NRF24L01Instance, EN_RXADDR, 0x3F);

    /* 6. Configure Automatic Retransmission (Delay and Retry Count) */
    DRV_Nrf_WriteRegister(NRF24L01Instance, SETUP_RETR, (NRF24L01Instance->AutoRetransmitDelay << 4) | (NRF24L01Instance->AutoRetransmitCount));

    /* 7. Set RF Frequency Channel */
    DRV_Nrf_WriteRegister(NRF24L01Instance, RF_CH, NRF24L01Instance->FrequencyChannel);

    /* 8. Setup Data Rate and Power Level */
    DRV_Nrf24l01SetDataRate(NRF24L01Instance, NRF24L01Instance->NRF24L01_AirDataDate);
    DRV_Nrf24l01SetPALevel(NRF24L01Instance, NRF24L01Instance->NRF24L01_OutputPower);
    retval = DRV_Nrf_ReadRegister(NRF24L01Instance, RF_CH);

    /* 9. Load default addresses for Pipes and set Payload Width = 32 */
    DRV_Nrf24l01OpenReadingPipe(NRF24L01Instance, 0, NRF24L01Instance->RxAddressP0);
    DRV_Nrf24l01OpenReadingPipe(NRF24L01Instance, 1, NRF24L01Instance->RxAddressP1);

    /* Addresses for Pipes 2 to 5 only require 1 LSB byte */
    for(counter = 2; counter <= 5; counter++) {
        DRV_Nrf24l01OpenReadingPipe(NRF24L01Instance, counter, &NRF24L01Instance->RxAddressP2_5[counter-2]);
    }

    /* Flush TX FIFO */
    cmd = FLUSH_TX;
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, 0);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, 1);

    /* Flush RX FIFO */
    cmd = FLUSH_RX;
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, 0);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, 1);

    /* Clear status flags (RX_DR, TX_DS, MAX_RT) */
    DRV_Nrf_WriteRegister(NRF24L01Instance, STATUS, 0x70);

    /* Avoid compiler warning for unused variable */
    (void) retval;
    return STD_E_OK;
}

void DRV_Nrf24l01Deinit(NRF24L01_HandleTypedef* NRF24L01Instance)
{
    /* Reset configuration to power-down state */
    DRV_Nrf_WriteRegister(NRF24L01Instance, CONFIG, 0x00);
}
/*
 * @brief : This function used to get status of NRF24l01
 * @param : [in] : The instance of NRF24L01
 * @return : The status of NRF24L01
 */
uint8_t DRV_Nrf24l01GetStatus(NRF24L01_HandleTypedef* NRF24L01Instance)
{
	uint8_t retval = 0 ;
	uint8_t cmd    = 0xFFU;
    /* Start SPI transmission */
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);

    /* Send address and read status simultaneously, then read data */
    HAL_SPI_TransmitReceive(NRF24L01Instance->SPI_Instance, &cmd, &retval, 1, 10);

    /* End SPI transmission */
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);

	return retval;
}
void DRV_Nrf24l01ClearIRQ(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t InterruptID)
{
	DRV_Nrf_WriteRegister(NRF24L01Instance, STATUS, (1 << InterruptID));
}
NRF24L01_ReturnType DRV_Nrf24l01SwitchMode(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t mode)
{
    uint8_t config = 0 ;
    NRF24L01_ReturnType retval = STD_E_OK ;
    if ( mode ==  NRF24_RECEIVE_MODE)
    {
        /* Switch to receive mode by setting PRIM_RX bit */
        config = DRV_Nrf_ReadRegister(NRF24L01Instance, CONFIG);
        DRV_Nrf_WriteRegister(NRF24L01Instance, CONFIG, config | (1 << 0));

        /* Enable CE Pin to enter RX mode */
        HAL_GPIO_WritePin(NRF24L01Instance->ChipEnablePort, NRF24L01Instance->ChipEnablePin, GPIO_PIN_SET);
    }
    else if ( mode == NRF24_TRANSMIT_MODE)
    {
        /* Switch to transmit mode by clearing PRIM_RX bit */
        config = DRV_Nrf_ReadRegister(NRF24L01Instance, CONFIG);
        DRV_Nrf_WriteRegister(NRF24L01Instance, CONFIG, config & (~(1 << 0)));

        /* CE Pin should be handled during the actual transmit pulse */
        HAL_GPIO_WritePin(NRF24L01Instance->ChipEnablePort, NRF24L01Instance->ChipEnablePin, GPIO_PIN_RESET);
    }
    else
    {
        retval = STD_E_NOT_OK;
    }
    return retval;
}

void DRV_Nrf24l01WritePayloadWithAck(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t pipeID, uint8_t *sourcePtr)
{
    uint8_t cmd = 0 ;
    /* Move to Standby-I mode */
    HAL_GPIO_WritePin(NRF24L01Instance->ChipEnablePort, NRF24L01Instance->ChipEnablePin, GPIO_PIN_RESET);

    /* Clear data in TX FIFO */
    cmd = FLUSH_TX;
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10 );
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);

    /* 1. Pull CSN low to start SPI session */
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);

    /* 2. Send Write Payload with ACK command */
    cmd = W_ACK_PAYLOAD | pipeID ;
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, sourcePtr, LENGTH, 10);

    /* 3. Pull CSN High and stay in Standby */
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);
}

NRF24L01_ReturnType DRV_Nrf24l01Receive(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t* pipeID, uint8_t *destinationPtr, uint32_t timeout)
{
    uint8_t config      = 0;
    uint8_t status      = 0;
    uint8_t fifo_status = 0;
    uint8_t cmd         = 0 ;
    uint32_t timeoutCount = timeout;
    NRF24L01_ReturnType retval = STD_E_OK;
    uint8_t length = 0;

    NRF24L01Instance->State = NRF24_BUSY;

    /* 1. Switch to RX mode (PRIM_RX = 1) */
    config = DRV_Nrf_ReadRegister(NRF24L01Instance, CONFIG);
    DRV_Nrf_WriteRegister(NRF24L01Instance, CONFIG, config | (1 << 0));

    /* 2. Activate CE to start listening */
    HAL_GPIO_WritePin(NRF24L01Instance->ChipEnablePort, NRF24L01Instance->ChipEnablePin, GPIO_PIN_SET);

    /* 3. Wait for data or timeout */
    if ( FALSE == NRF24L01Instance->InterruptMode)
    {
        do {
            status = DRV_Nrf_ReadRegister(NRF24L01Instance, STATUS);
            timeoutCount--;
        } while (!(status & (1 << 6)) && (timeoutCount > 0)); /* Bit 6 is RX_DR */
    }
    else
    {
        status = DRV_Nrf_ReadRegister(NRF24L01Instance, STATUS);
    }
    DRV_Nrf24l01GetStatus(NRF24L01Instance);

    /* 4. If data received (RX_DR = 1) */
    if (status & (1 << 6)) {
        /* Identify source Pipe ID */
        (*pipeID) = (status >> 1) & 0x07;

        /* Empty the FIFO */
        do {
            if ( NRF24L01Instance->DynamicPayloadEnable == TRUE)
            {
                HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);
                cmd = R_RX_PL_WID;
                HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
                HAL_SPI_Receive(NRF24L01Instance->SPI_Instance, &length, 1, 100);
                HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);
            }

            /* Read Payload from the top of the FIFO */
            DRV_Nrf24L01ReadPayload(NRF24L01Instance, destinationPtr, length);

            /* Clear RX_DR flag to allow next packet processing */
            DRV_Nrf_WriteRegister(NRF24L01Instance, STATUS, (1 << 6));

            /* Check if FIFO still contains data (RX_EMPTY bit) */
            fifo_status = DRV_Nrf_ReadRegister(NRF24L01Instance, FIFO_STATUS);

        } while (!(fifo_status & (1 << 0)));

        /* Pull CE low (Standby-I) after processing */
        HAL_GPIO_WritePin(NRF24L01Instance->ChipEnablePort, NRF24L01Instance->ChipEnablePin, GPIO_PIN_RESET);
        retval = STD_E_OK;
    }
    else
    {
        /* Timeout or no data */
        HAL_GPIO_WritePin(NRF24L01Instance->ChipEnablePort, NRF24L01Instance->ChipEnablePin, GPIO_PIN_RESET);
        retval = STD_E_NOT_OK;
    }

    NRF24L01Instance->State = NRF24_IDEL;
    return retval;
}

NRF24L01_ReturnType DRV_Nrf24l01Transmit(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t channelID, uint8_t *targetAddr, uint8_t *sourcePtr)
{
    uint8_t cmd      = 0 ;
    uint8_t status   = 0 ;
    uint32_t timeout = 500;
    NRF24L01_ReturnType retval = STD_E_OK;

    /* Step 1: Ensure PRIM_RX = 0 (TX Mode) */
    uint8_t config = DRV_Nrf_ReadRegister(NRF24L01Instance, CONFIG);
    config &= ~(1 << 0);
    DRV_Nrf_WriteRegister(NRF24L01Instance, CONFIG, config);

    /* Step 2: Configure address and load payload */
    DRV_Nrf24l01OpenWritingPipe(NRF24L01Instance, channelID, targetAddr);
    DRV_Nrf24L01WritePayload(NRF24L01Instance, sourcePtr, LENGTH);

    /* Step 3: Trigger transmission pulse */
    HAL_GPIO_WritePin(NRF24L01Instance->ChipEnablePort, NRF24L01Instance->ChipEnablePin, GPIO_PIN_SET);
    HAL_Delay(1); /* Pulse must be > 10us */
    HAL_GPIO_WritePin(NRF24L01Instance->ChipEnablePort, NRF24L01Instance->ChipEnablePin, GPIO_PIN_RESET);

    if ( FALSE == NRF24L01Instance->InterruptMode)
    {
        /* Step 4: Wait for TX_DS (Success) or MAX_RT (Failed) */
        do
        {
            status = DRV_Nrf_ReadRegister(NRF24L01Instance, STATUS);
            timeout--;
        } while (!(status & ((1 << 5) | (1 << 4))) && (timeout > 0));

        /* Step 5: Handle transmission result */
        if ((status & (1 << 4)) || (timeout == 0)) /* MAX_RT error or SPI Timeout */
        {
            DRV_Nrf_WriteRegister(NRF24L01Instance, STATUS, (1 << 4)); /* Clear MAX_RT flag */

            cmd = FLUSH_TX;
            HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, 0);
            HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
            HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, 1);

            retval =  STD_E_NOT_OK;
        }
    }

    return retval;
}

NRF24L01_ReturnType DRV_Nrf24l01ReadPayloadWithAck(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t* destinationPtr, uint32_t timeout)
{
    uint8_t status      = 0;
    uint8_t fifo_status = 0;
    uint8_t cmd         = 0 ;
    NRF24L01_ReturnType retval = STD_E_OK;
    uint8_t length       = 0;

    status = DRV_Nrf_ReadRegister(NRF24L01Instance, STATUS);

    /* Check if data is available (RX_DR = 1) */
    if (status & (1 << 6))
    {
        do
        {
            if ( NRF24L01Instance->DynamicPayloadEnable == TRUE)
            {
                HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);
                cmd = R_RX_PL_WID;
                HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
                HAL_SPI_Receive(NRF24L01Instance->SPI_Instance, &length, 1, 100);
                HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);

                DRV_Nrf24L01ReadPayload(NRF24L01Instance, destinationPtr, length);
            }
            else
            {
                DRV_Nrf24L01ReadPayload(NRF24L01Instance, destinationPtr, LENGTH);
            }

            /* Clear RX_DR flag */
            DRV_Nrf_WriteRegister(NRF24L01Instance, STATUS, (1 << 6));
            fifo_status = DRV_Nrf_ReadRegister(NRF24L01Instance, FIFO_STATUS);

        } while (!(fifo_status & (1 << 0)));

        HAL_GPIO_WritePin(NRF24L01Instance->ChipEnablePort, NRF24L01Instance->ChipEnablePin, GPIO_PIN_RESET);
        retval = STD_E_OK;
    }
    else
    {
        HAL_GPIO_WritePin(NRF24L01Instance->ChipEnablePort, NRF24L01Instance->ChipEnablePin, GPIO_PIN_RESET);
        retval = STD_E_NOT_OK;
    }
    return retval;
}

NRF24L01_ReturnType DRV_Nrf24l01SetDataRate(NRF24L01_HandleTypedef* NRF24L01Instance, NRF24L01_AirDataDateTypedef speed)
{
    uint8_t rf_setup = DRV_Nrf_ReadRegister(NRF24L01Instance, RF_SETUP);

    /* Data rate is controlled by bit 3 (RF_DR_HIGH) */
    if (speed == NRF24L01_2MBPS)
    {
        rf_setup |= (1 << 3);
    }
    else
    {
        rf_setup &= ~(1 << 3);
    }

    DRV_Nrf_WriteRegister(NRF24L01Instance, RF_SETUP, rf_setup);

    /* Verify if write was successful */
    if (DRV_Nrf_ReadRegister(NRF24L01Instance, RF_SETUP) == rf_setup) return STD_E_OK;
    return STD_E_NOT_OK;
}

NRF24L01_ReturnType DRV_Nrf24l01SetPALevel(NRF24L01_HandleTypedef* NRF24L01Instance, NRF24L01_OutputPowerTypedef level)
{
    uint8_t rf_setup = DRV_Nrf_ReadRegister(NRF24L01Instance, RF_SETUP);

    /* Clear existing bits 2:1 */
    rf_setup &= ~(0x06);

    /* Write new level into bits 2:1 */
    rf_setup |= (level << 1);

    DRV_Nrf_WriteRegister(NRF24L01Instance, RF_SETUP, rf_setup);
    return STD_E_OK;
}

NRF24L01_ReturnType DRV_Nrf24l01OpenWritingPipe(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t channelID, uint8_t *address)
{
    uint8_t fullAddress[5];
    uint8_t counter = 0 ;

    if (channelID < 2)
    {
        /* For Pipe 0 and 1, use the full 5-byte address provided */
        for(counter = 0; counter < 5; counter++)
        {
            fullAddress[counter] = address[counter];
        }
    }
    else
    {
        /* For Pipes 2-5, LSB is unique, but MSBs must match Pipe 1 */
        for(counter = 1; counter < 5; counter++)
        {
            fullAddress[counter] = NRF24L01Instance->RxAddressP1[counter];
        }
        fullAddress[0] = address[0];
    }

    /* 1. Write FULL address to TX_ADDR register */
    uint8_t cmd = W_REGISTER | TX_ADDR;
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, fullAddress, 5, 50);
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);

    /* 2. Write same address to RX_ADDR_P0 to enable Auto-ACK reception */
    cmd = W_REGISTER | RX_ADDR_P0;
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, fullAddress, 5, 50);
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);

    return STD_E_OK;
}

NRF24L01_ReturnType DRV_Nrf24l01OpenReadingPipe(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t pipeNum, uint8_t *address)
{
    NRF24L01_ReturnType retval = STD_E_OK ;
    uint8_t reg = 0 ;
    uint8_t cmd = 0 ;
    uint8_t en_rx = 0 ;
    uint8_t addrSize = 0 ;

    if (pipeNum > 5)
    {
        retval = STD_E_NOT_OK;
        (void) en_rx; (void) cmd ; (void) reg ;
    }
    else
    {
        /* 1. Write address to RX_ADDR_Px register */
        reg = RX_ADDR_P0 + pipeNum;
        cmd = W_REGISTER | reg;

        HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);
        HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);

        /* P0 and P1 use 5 bytes, P2-P5 only use 1 byte (LSB) */
        addrSize = (pipeNum < 2) ? 5 : 1;
        HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, address, addrSize, 10);
        HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);

        /* 2. Set static Payload Width (Defaulting to 32 bytes) */
        DRV_Nrf_WriteRegister(NRF24L01Instance, RX_PW_P0 + pipeNum, 32);

        /* 3. Activate the specific Pipe in EN_RXADDR register */
        en_rx = DRV_Nrf_ReadRegister(NRF24L01Instance, EN_RXADDR);
        en_rx |= (1 << pipeNum);
        DRV_Nrf_WriteRegister(NRF24L01Instance, EN_RXADDR, en_rx);
        retval = STD_E_OK;
    }

    return retval;
}

/*
 * Local helper functions
 */
static uint8_t DRV_Nrf_ReadRegister(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t reg)
{
    uint8_t cmd = READ_REGISTER(reg);
    uint8_t status = 0;
    uint8_t data = 0;

    /* Start SPI transmission */
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);

    /* Send address and read status simultaneously, then read data */
    HAL_SPI_TransmitReceive(NRF24L01Instance->SPI_Instance, &cmd, &status, 1, 10);
    HAL_SPI_Receive(NRF24L01Instance->SPI_Instance, &data, 1, 10);

    /* End SPI transmission */
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);

    return data;
}

static void DRV_Nrf_WriteRegister(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t reg, uint8_t data)
{
    uint8_t l_reg = WRITE_REGISTER(reg);

    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);

    /* Send register address and then the data byte */
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &l_reg , 1, 10);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &data, 1, 10);

    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);
}

/**
 * @brief Reads a payload from the RX FIFO
 */
static void DRV_Nrf24L01ReadPayload(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t *data, uint8_t length)
{
    uint8_t cmd = R_RX_PAYLOAD;

    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);

    /* Send command and receive the requested number of bytes */
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
    HAL_SPI_Receive(NRF24L01Instance->SPI_Instance, data, length, 100);

    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);
}

/**
 * @brief Writes a payload into the TX FIFO
 */
static void DRV_Nrf24L01WritePayload(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t *data, uint8_t length )
{
    uint8_t cmd = W_TX_PAYLOAD;

    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);

    /* Send command and then the payload buffer */
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, data, length, 100);

    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);

    /* Note: CE must be pulsed HIGH for at least 10us after this to trigger transmission */
}
