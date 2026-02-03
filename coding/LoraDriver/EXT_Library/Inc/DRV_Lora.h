/**
 * @file drv_lora.h
 * @brief LoRa/LoRaWAN Driver Header File for SX1278 Module
 * @details This driver provides an interface for SX1278 LoRa transceiver module
 *          supporting configuration, transmission, and reception in LoRa mode.
 */

#ifndef _DRV_LORA_H
#define _DRV_LORA_H

/* Include */
#include "stdint.h"
#include "spi.h"
#include "gpio.h"

/* 
 * SX1278 / LoRa R01 Register Map (LoRa mode)
 */

/* ============================================================================
 * COMMON REGISTERS
 * ============================================================================ */

/** @brief FIFO read/write access register */
#define REG_FIFO                       0x00

/** @brief Operating mode and LoRa/FSK selection register */
#define REG_OP_MODE                    0x01

/** @brief RF carrier frequency MSB register */
#define REG_FRF_MSB                    0x06

/** @brief RF carrier frequency MID register */
#define REG_FRF_MID                    0x07

/** @brief RF carrier frequency LSB register */
#define REG_FRF_LSB                    0x08

/** @brief Power amplifier configuration register */
#define REG_PA_CONFIG                  0x09

/** @brief PA ramp time configuration register */
#define REG_PA_RAMP                    0x0A

/** @brief Over current protection register */
#define REG_OCP                        0x0B

/** @brief Low noise amplifier settings register */
#define REG_LNA                        0x0C

/** @brief FIFO SPI pointer register */
#define REG_FIFO_ADDR_PTR              0x0D

/** @brief FIFO TX base address register */
#define REG_FIFO_TX_BASE_ADDR          0x0E

/** @brief FIFO RX base address register */
#define REG_FIFO_RX_BASE_ADDR          0x0F

/** @brief Start address of last packet received in FIFO */
#define REG_FIFO_RX_CURRENT_ADDR       0x10

/** @brief Interrupt flags mask register */
#define REG_IRQ_FLAGS_MASK             0x11

/** @brief Interrupt flags register */
#define REG_IRQ_FLAGS                  0x12

/** @brief Number of payload bytes of latest packet received */
#define REG_RX_NB_BYTES                0x13

/** @brief Number of valid headers received MSB */
#define REG_RX_HEADER_CNT_VALUE_MSB    0x14

/** @brief Number of valid headers received LSB */
#define REG_RX_HEADER_CNT_VALUE_LSB    0x15

/** @brief Number of valid packets received MSB */
#define REG_RX_PACKET_CNT_VALUE_MSB    0x16

/** @brief Number of valid packets received LSB */
#define REG_RX_PACKET_CNT_VALUE_LSB    0x17

/** @brief Live LoRa modem status register */
#define REG_MODEM_STAT                 0x18

/** @brief SNR estimation of last packet received */
#define REG_PKT_SNR_VALUE              0x19

/** @brief RSSI of last packet received */
#define REG_PKT_RSSI_VALUE             0x1A

/** @brief Current RSSI value */
#define REG_RSSI_VALUE                 0x1B

/** @brief Frequency hopping channel register */
#define REG_HOP_CHANNEL                0x1C

/** @brief Modem PHY configuration 1 (BW, CR, Header mode) */
#define REG_MODEM_CONFIG_1             0x1D

/** @brief Modem PHY configuration 2 (SF, CRC, RX timeout MSB) */
#define REG_MODEM_CONFIG_2             0x1E

/** @brief RX timeout LSB register */
#define REG_SYMB_TIMEOUT_LSB           0x1F

/** @brief Preamble length MSB register */
#define REG_PREAMBLE_MSB               0x20

/** @brief Preamble length LSB register */
#define REG_PREAMBLE_LSB               0x21

/** @brief Payload length register */
#define REG_PAYLOAD_LENGTH             0x22

/** @brief Maximum payload length register */
#define REG_MAX_PAYLOAD_LENGTH         0x23

/** @brief Frequency hopping period register */
#define REG_HOP_PERIOD                 0x24

/** @brief Address of last byte written in FIFO */
#define REG_FIFO_RX_BYTE_ADDR          0x25

/** @brief Modem PHY configuration 3 (AGC, LowDataRateOptimize) */
#define REG_MODEM_CONFIG_3             0x26

/* ============================================================================
 * FREQUENCY HOPPING / CHANNEL REGISTERS
 * ============================================================================ */

/** @brief Frequency error MSB register */
#define REG_FEI_MSB                    0x28

/** @brief Frequency error MID register */
#define REG_FEI_MID                    0x29

/** @brief Frequency error LSB register */
#define REG_FEI_LSB                    0x2A

/** @brief Wideband RSSI measurement register */
#define REG_RSSI_WIDEBAND              0x2C

/* ============================================================================
 * DETECTION / SYNC REGISTERS
 * ============================================================================ */

/** @brief LoRa detection optimize register */
#define REG_DETECT_OPTIMIZE            0x31

/** @brief IQ inversion register */
#define REG_INVERT_IQ                  0x33

/** @brief LoRa detection threshold register */
#define REG_DETECTION_THRESHOLD        0x37

/** @brief LoRa sync word register (default: 0x12 for LoRaWAN) */
#define REG_SYNC_WORD                  0x39

/* ============================================================================
 * DIO MAPPING REGISTERS
 * ============================================================================ */

/** @brief DIO0-DIO3 mapping register */
#define REG_DIO_MAPPING_1              0x40

/** @brief DIO4-DIO5 mapping register */
#define REG_DIO_MAPPING_2              0x41

/* ============================================================================
 * VERSION & CONFIGURATION REGISTERS
 * ============================================================================ */

/** @brief Chip version register */
#define REG_VERSION                    0x42

/** @brief TCXO or crystal oscillator input selection */
#define REG_TCXO                       0x4B

/** @brief High power settings for PA_BOOST */
#define REG_PA_DAC                     0x4D

/** @brief Temperature sensor value at last calibration */
#define REG_FORMER_TEMP                0x5B

/** @brief AGC reference level */
#define REG_AGC_REF                    0x61

/** @brief AGC threshold 1 */
#define REG_AGC_THRESH1                0x62

/** @brief AGC threshold 2 */
#define REG_AGC_THRESH2                0x63

/** @brief AGC threshold 3 */
#define REG_AGC_THRESH3                0x64

/** @brief PLL frequency hopping settings */
#define REG_PLL_HOP                    0x44

/** @brief PLL lock status */
#define REG_PLL_LOCK                   0x70

/* ============================================================================
 * OPERATION STATE DEFINITIONS
 * ============================================================================ */

/** @brief Sleep mode (lowest power consumption) */
#define LORA_SLEEP_STATE                     0x00

/** @brief Standby mode (crystal oscillator running) */
#define LORA_STANDBY_STATE                   0x01

/** @brief Transmit mode */
#define LORA_TRANSMIT_STATE                  0x03

/** @brief Continuous receive mode */
#define LORA_RECEIVE_CONTINUOUS_STATE        0x05

/** @brief Single receive mode */
#define LORA_RECEIVE_SINGLE_STATE            0x06

/** @brief Channel activity detection mode */
#define LORA_CHANNEL_ACTIVITY_DETECTION      0x07

/* ============================================================================
 * DRIVER MACROS
 * ============================================================================ */

/** @brief Maximum data length that can be transmitted in one packet */
#define MAX_DATA_SENT                       0xFF

/** @brief Macro to generate read register address (clear MSB) */
#define LORA_READ_REG(x)                    ((uint8_t)((x) & 0x7F))

/** @brief Macro to generate write register address (set MSB) */
#define LORA_WRITE_REG(x)                   ((uint8_t)((x) | 0x80))

/* ============================================================================
 * TYPEDEFS AND STRUCTURES
 * ============================================================================ */

/**
 * @brief Return type enumeration for LoRa driver functions
 */
typedef enum
{
    STD_E_OK,       /**< Operation completed successfully */
    STD_E_NOT_OK    /**< Operation failed */
} LORA_ReturnTypedef; 

/**
 * @brief Operation mode type for LoRa driver (polling vs interrupt)
 */
typedef enum
{
    LORA_MODE_POLLING,      /**< Polling mode (blocking) */
    LORA_MODE_INTERRUPT     /**< Interrupt mode (non-blocking with callbacks) */
} LORA_OperationModeTypedef; 

/**
 * @brief LoRa module operating states
 */
typedef enum {
    LORA_MODE_SLEEP      = 0x00,    /**< Sleep mode */
    LORA_MODE_STANDBY    = 0x01,    /**< Standby mode */
    LORA_MODE_TX         = 0x03,    /**< Transmit mode */
    LORA_MODE_RX_CONT    = 0x05,    /**< Continuous receive mode */
    LORA_MODE_RX_SINGLE  = 0x06,    /**< Single receive mode */
} Lora_State_t;

/**
 * @brief LoRa bandwidth configuration enumeration
 * @details Lower bandwidth = better sensitivity but lower data rate
 *          Higher bandwidth = higher data rate but worse sensitivity
 */
typedef enum {
    LORA_BW_7_8_KHZ   = 0,  /**< 7.8 kHz bandwidth */
    LORA_BW_10_4_KHZ  = 1,  /**< 10.4 kHz bandwidth */
    LORA_BW_15_6_KHZ  = 2,  /**< 15.6 kHz bandwidth */
    LORA_BW_20_8_KHZ  = 3,  /**< 20.8 kHz bandwidth */
    LORA_BW_31_25_KHZ = 4,  /**< 31.25 kHz bandwidth */
    LORA_BW_41_7_KHZ  = 5,  /**< 41.7 kHz bandwidth */
    LORA_BW_62_5_KHZ  = 6,  /**< 62.5 kHz bandwidth */
    LORA_BW_125_KHZ   = 7,  /**< 125 kHz bandwidth (common for LoRaWAN) */
    LORA_BW_250_KHZ   = 8,  /**< 250 kHz bandwidth */
    LORA_BW_500_KHZ   = 9   /**< 500 kHz bandwidth */
} LORA_Bandwidth_t;

/**
 * @brief LoRa forward error correction coding rate
 * @details Higher coding rate = better error correction but lower effective data rate
 *          Format: 4/x where x is the denominator
 */
typedef enum {
    LORA_CR_4_5 = 1,    /**< Coding rate 4/5 (register value 1) */
    LORA_CR_4_6 = 2,    /**< Coding rate 4/6 (register value 2) */
    LORA_CR_4_7 = 3,    /**< Coding rate 4/7 (register value 3) */
    LORA_CR_4_8 = 4     /**< Coding rate 4/8 (register value 4) */
} LORA_CodingRate_t;

/**
 * @brief LoRa spreading factor configuration
 * @details Higher SF = longer range but lower data rate and higher air time
 *          Lower SF = shorter range but higher data rate and lower air time
 *          LoRaWAN typically uses SF7-SF12
 */
typedef enum {
    LORA_SF_7  = 7,     /**< Spreading factor 7 (fastest, shortest range) */
    LORA_SF_8  = 8,     /**< Spreading factor 8 */
    LORA_SF_9  = 9,     /**< Spreading factor 9 */
    LORA_SF_10 = 10,    /**< Spreading factor 10 (common for LoRaWAN) */
    LORA_SF_11 = 11,    /**< Spreading factor 11 */
    LORA_SF_12 = 12     /**< Spreading factor 12 (slowest, longest range) */
} LORA_SpreadingFactor_t;

/**
 * @brief LoRa transmission power modes
 * @details Power levels affect transmission range and power consumption
 */
typedef enum {
    LORA_POWER_LOW     = 0,     /**< Low power mode: ~2 dBm (indoor, battery saving) */
    LORA_POWER_BALANCE = 1,     /**< Balanced mode: ~14 dBm (medium range) */
    LORA_POWER_MAX     = 2      /**< Maximum power: 17-20 dBm (long range, wall penetration) */
} LORA_PowerMode_t;

/**
 * @brief LoRa driver handle structure
 * @details Contains all configuration parameters and hardware interface settings
 *          for the LoRa module operation
 */
typedef struct
{
    /* Hardware Interface */
    SPI_HandleTypeDef*        SPI_Instance;      /**< SPI peripheral instance for communication */
    uint16_t                  ChipSelectPin;     /**< NSS/CS pin number */
    GPIO_TypeDef*             ChipSelectPort;    /**< NSS/CS GPIO port */
    uint16_t                  ResetPin;          /**< Hardware reset pin number */
    GPIO_TypeDef*             ResetPort;         /**< Hardware reset GPIO port */
    uint16_t                  Dio_0_Pin;         /**< DIO0 interrupt pin number */
    GPIO_TypeDef*             Dio_0_Port;        /**< DIO0 GPIO port */
    
    /* Operation Mode */
    LORA_OperationModeTypedef OperationMode;     /**< Polling or interrupt mode */
    
    /* RF Configuration */
    uint32_t                  Frequency;         /**< Operating frequency in Hz (e.g., 433000000 for 433 MHz) */
    LORA_SpreadingFactor_t    SpreadingFactor;   /**< LoRa spreading factor (SF7-SF12) */
    LORA_Bandwidth_t          Bandwidth;         /**< Signal bandwidth */
    LORA_CodingRate_t         CodingRate;        /**< Forward error correction coding rate */
    LORA_PowerMode_t          Power;             /**< Transmission power mode */
    
    /* Callback Functions */
    void                      (*RxCallback)(uint8_t* data, uint8_t size);  /**< Receive complete callback */
    void                      (*TxCallback)(void);                         /**< Transmit complete callback */
} LORA_HandleTypedef;

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */

/**
 * @brief Initialize the LoRa module
 * @details Configures the SX1278 module with the specified parameters,
 *          sets up SPI communication, and puts the module in standby mode
 * @in LORA_Instance Pointer to LoRa handle structure with configuration
 * @output STD_E_OK if initialization successful, STD_E_NOT_OK otherwise
 */
LORA_ReturnTypedef DRV_LoraInit(LORA_HandleTypedef* LORA_Instance);

/**
 * @brief Deinitialize the LoRa module
 * @details Puts the module in sleep mode and releases resources
 * @in LORA_Instance Pointer to LoRa handle structure
 * @output None
 */
void DRV_LoraDeinit(LORA_HandleTypedef* LORA_Instance);

/**
 * @brief Switch LoRa module operating mode
 * @details Changes the module state (sleep, standby, TX, RX, etc.)
 * @in LORA_Instance Pointer to LoRa handle structure
 * @in LoraState Desired operating state (Lora_State_t enumeration)
 * @output STD_E_OK if mode switch successful, STD_E_NOT_OK otherwise
 */
LORA_ReturnTypedef DRV_LoraSwitchMode(LORA_HandleTypedef* LORA_Instance, Lora_State_t LoraState);

/**
 * @brief Transmit data via LoRa
 * @details Sends data packet through the LoRa module
 * @in LORA_Instance Pointer to LoRa handle structure
 * @in PtrSourceData Pointer to data buffer to transmit
 * @in length Number of bytes to transmit (max 255)
 * @in timeOut Timeout value in milliseconds
 * @output STD_E_OK if transmission successful, STD_E_NOT_OK otherwise
 */
LORA_ReturnTypedef DRV_LoraTransmit(LORA_HandleTypedef* LORA_Instance, uint8_t* PtrSourceData, uint16_t length, uint16_t timeOut);

/**
 * @brief Receive data via LoRa
 * @details Receives data packet from the LoRa module
 * @in LORA_Instance Pointer to LoRa handle structure
 * @in PtrDestinationData Pointer to buffer for received data
 * @in length Maximum number of bytes to receive
 * @in timeOut Timeout value in milliseconds
 * @output STD_E_OK if reception successful, STD_E_NOT_OK otherwise
 */
LORA_ReturnTypedef DRV_LoraReceive(LORA_HandleTypedef* LORA_Instance, uint8_t* PtrDestinationData, uint16_t length, uint16_t timeOut);

/**
 * @brief Configure LoRa operating frequency
 * @details Sets the RF carrier frequency based on LORA_Instance->Frequency
 * @in LORA_Instance Pointer to LoRa handle structure containing frequency setting
 * @output STD_E_OK if configuration successful, STD_E_NOT_OK otherwise
 */
LORA_ReturnTypedef DRV_LoraConfigFrequency(LORA_HandleTypedef* LORA_Instance);

/**
 * @brief Configure LoRa interrupt settings
 * @details Sets up interrupt flags and DIO pin mappings for interrupt-driven operation
 * @in LORA_Instance Pointer to LoRa handle structure
 * @output STD_E_OK if configuration successful, STD_E_NOT_OK otherwise
 */
LORA_ReturnTypedef DRV_LoraConfigInterrupt(LORA_HandleTypedef* LORA_Instance);

/**
 * @brief LoRa interrupt handler
 * @details Processes interrupt flags and calls appropriate callbacks
 *          Should be called from the DIO0 external interrupt handler
 * @in LORA_Instance Pointer to LoRa handle structure
 * @output STD_E_OK if interrupt handled successfully, STD_E_NOT_OK otherwise
 */
LORA_ReturnTypedef DRV_Lora_IRQHandler(LORA_HandleTypedef* LORA_Instance);

/**
 * @brief Get SX1278 chip version
 * @details Reads the version register to verify chip communication
 * @in LORA_Instance Pointer to LoRa handle structure
 * @output Chip version number (typically 0x12 for SX1278)
 */
uint8_t DRV_Lora_GetVersion(LORA_HandleTypedef* LORA_Instance);

#endif /* _DRV_LORA_H */