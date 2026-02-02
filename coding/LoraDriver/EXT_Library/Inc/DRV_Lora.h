#ifndef _DRV_LORA_H
#define _DRV_LORA_H

/* Include */
#include "stdint.h"
#include "spi.h"
#include "gpio.h"
/* 
 * SX1278 / LoRa R01 Register Map (LoRa mode)
 */
/* Common registers */
#define REG_FIFO                       0x00
#define REG_OP_MODE                    0x01
#define REG_FRF_MSB                    0x06
#define REG_FRF_MID                    0x07
#define REG_FRF_LSB                    0x08
#define REG_PA_CONFIG                  0x09
#define REG_PA_RAMP                    0x0A
#define REG_OCP                        0x0B
#define REG_LNA                        0x0C
#define REG_FIFO_ADDR_PTR              0x0D
#define REG_FIFO_TX_BASE_ADDR          0x0E
#define REG_FIFO_RX_BASE_ADDR          0x0F
#define REG_FIFO_RX_CURRENT_ADDR       0x10
#define REG_IRQ_FLAGS_MASK             0x11
#define REG_IRQ_FLAGS                  0x12
#define REG_RX_NB_BYTES                0x13
#define REG_RX_HEADER_CNT_VALUE_MSB    0x14
#define REG_RX_HEADER_CNT_VALUE_LSB    0x15
#define REG_RX_PACKET_CNT_VALUE_MSB    0x16
#define REG_RX_PACKET_CNT_VALUE_LSB    0x17
#define REG_MODEM_STAT                 0x18
#define REG_PKT_SNR_VALUE              0x19
#define REG_PKT_RSSI_VALUE             0x1A
#define REG_RSSI_VALUE                 0x1B
#define REG_HOP_CHANNEL                0x1C
#define REG_MODEM_CONFIG_1             0x1D
#define REG_MODEM_CONFIG_2             0x1E
#define REG_SYMB_TIMEOUT_LSB           0x1F
#define REG_PREAMBLE_MSB               0x20
#define REG_PREAMBLE_LSB               0x21
#define REG_PAYLOAD_LENGTH             0x22
#define REG_MAX_PAYLOAD_LENGTH         0x23
#define REG_HOP_PERIOD                 0x24
#define REG_FIFO_RX_BYTE_ADDR          0x25
#define REG_MODEM_CONFIG_3             0x26

/* Frequency hopping / channel registers */
#define REG_FEI_MSB                    0x28
#define REG_FEI_MID                    0x29
#define REG_FEI_LSB                    0x2A
#define REG_RSSI_WIDEBAND              0x2C

/* Detection / sync */
#define REG_DETECT_OPTIMIZE            0x31
#define REG_INVERT_IQ                  0x33
#define REG_DETECTION_THRESHOLD        0x37
#define REG_SYNC_WORD                  0x39

/* DIO mapping */
#define REG_DIO_MAPPING_1              0x40
#define REG_DIO_MAPPING_2              0x41

/* Version */
#define REG_VERSION                    0x42

/* PA / TCXO / temperature */
#define REG_TCXO                       0x4B
#define REG_PA_DAC                     0x4D
#define REG_FORMER_TEMP                0x5B
#define REG_AGC_REF                    0x61
#define REG_AGC_THRESH1                0x62
#define REG_AGC_THRESH2                0x63
#define REG_AGC_THRESH3                0x64

/* PLL */
#define REG_PLL_HOP                    0x44
#define REG_PLL_LOCK                   0x70

/* Operation State */
#define LORA_SLEEP_STATE                     0x00
#define LORA_STANDBY_STATE                   0x01
#define LORA_TRANSMIT_STATE                  0x03
#define LORA_RECEIVE_CONTINUOUS_STATE        0x05
#define LORA_RECEIVE_SINGLE_STATE            0x06
#define LORA_CHANNEL_ACTIVITY_DETECTION      0x07

/* Max data sent disposable  */
#define MAX_DATA_SENT                       0xFF
#define LORA_READ_REG(x)                    ((uint8_t)((x) & 0x7F))
#define LORA_WRITE_REG(x)                   ((uint8_t)((x) | 0x80))
/*
 * Typdef and structure 
*/
/* Define return type for driver lora */
typedef enum
{
    STD_E_OK,
    STD_E_NOT_OK
} LORA_ReturnTypedef; 

/* Define type of lora async or sync  */
typedef enum
{
    LORA_MODE_POLLING,
    LORA_MODE_INTERRUPT
} LORA_OperationModeTypedef ; 

/* Define mode of lora */
typedef enum {
    LORA_MODE_SLEEP      = 0x00,
    LORA_MODE_STANDBY    = 0x01,
    LORA_MODE_TX         = 0x03,
    LORA_MODE_RX_CONT    = 0x05,
    LORA_MODE_RX_SINGLE  = 0x06,
} Lora_State_t;

/* Define bandwidth */
typedef enum {
    LORA_BW_7_8_KHZ   = 0,
    LORA_BW_10_4_KHZ  = 1,
    LORA_BW_15_6_KHZ  = 2,
    LORA_BW_20_8_KHZ  = 3,
    LORA_BW_31_25_KHZ = 4,
    LORA_BW_41_7_KHZ  = 5,
    LORA_BW_62_5_KHZ  = 6,
    LORA_BW_125_KHZ   = 7,
    LORA_BW_250_KHZ   = 8,
    LORA_BW_500_KHZ   = 9
} LORA_Bandwidth_t;

/* Define coding rate structure */
typedef enum {
    LORA_CR_4_5 = 1, // Ghi vào thanh ghi giá trị 1
    LORA_CR_4_6 = 2,
    LORA_CR_4_7 = 3,
    LORA_CR_4_8 = 4
} LORA_CodingRate_t;

/* Define spreading factor structure */
typedef enum {
    LORA_SF_7  = 7,
    LORA_SF_8  = 8,
    LORA_SF_9  = 9,
    LORA_SF_10 = 10,
    LORA_SF_11 = 11,
    LORA_SF_12 = 12
} LORA_SpreadingFactor_t;

/* Define structure for power mode */
typedef enum {
    LORA_POWER_LOW     = 0, // Tiết kiệm pin, dùng trong phòng (2 dBm)
    LORA_POWER_BALANCE = 1, // Cân bằng, đi xa vừa phải (14 dBm)
    LORA_POWER_MAX     = 2  // Mạnh nhất, xuyên tường (17-20 dBm)
} LORA_PowerMode_t;


/* Define structure for driver lora */
typedef struct
{
    /* data */
    SPI_HandleTypeDef*        SPI_Instance ; 
    uint16_t                  ChipSelectPin;
    GPIO_TypeDef*             ChipSelectPort;
    uint16_t                  ResetPin     ;
    GPIO_TypeDef*             ResetPort;
    uint16_t                  Dio_0_Pin    ;
    GPIO_TypeDef*             Dio_0_Port;
    LORA_OperationModeTypedef OperationMode;
    uint32_t                  Frequency;         // Example: 433000000
    LORA_SpreadingFactor_t    SpreadingFactor;   // LORA_SF7 to LORA_SF12
    LORA_Bandwidth_t          Bandwidth;         // LORA_BW_125KHZ, v.v.
    LORA_CodingRate_t         CodingRate;        // LORA_CR_4_5, v.v.
    LORA_PowerMode_t          Power;             // Power transmit (dBm)
    void                      (*RxCallback)(uint8_t* data, uint8_t size);
    void                      (*TxCallback);                           
} LORA_HandleTypedef ;

/* Prototype function */
LORA_ReturnTypedef DRV_LoraInit(LORA_HandleTypedef* LORA_Instance);
void               DRV_LoraDeinit(LORA_HandleTypedef* LORA_Instance);
LORA_ReturnTypedef DRV_LoraSwitchMode(LORA_HandleTypedef* LORA_Instance, Lora_State_t LoraState);
LORA_ReturnTypedef DRV_LoraTransmit(LORA_HandleTypedef* LORA_Instance, uint8_t* PtrSourceData, uint16_t length, uint16_t timeOut);
LORA_ReturnTypedef DRV_LoraReceive(LORA_HandleTypedef* LORA_Instance, uint8_t* PtrDestinationData, uint16_t length, uint16_t timeOut);
LORA_ReturnTypedef DRV_LoraConfigFrequency(LORA_HandleTypedef* LORA_Instance);
LORA_ReturnTypedef DRV_LoraConfigInterrupt(LORA_HandleTypedef* LORA_Instance);
LORA_ReturnTypedef DRV_Lora_IRQHandler(LORA_HandleTypedef* LORA_Instance); 
uint8_t DRV_Lora_GetVersion(LORA_HandleTypedef* LORA_Instance);
#endif
