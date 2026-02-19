#ifndef _DRV_NRF24L01_H_
#define _DRV_NRF24L01_H_

#include "gpio.h"
#include "spi.h"

/* --- NRF24L01 COMMANDS --- 
 * Use these commands to control NRF24 via SPI
 */
#define R_REGISTER          0x00  // Read register (mask with 5-bit address)
#define W_REGISTER          0x20  // Write register (mask with 5-bit address)
#define R_RX_PAYLOAD        0x61  // Read RX payload from FIFO
#define W_TX_PAYLOAD        0xA0  // Write TX payload to FIFO
#define FLUSH_TX            0xE1  // Flush TX FIFO
#define FLUSH_RX            0xE2  // Flush RX FIFO
#define REUSE_TX_PL         0xE3  // Reuse last transmitted payload
#define R_RX_PL_WID         0x60  // Read RX payload width for the top R_RX_PAYLOAD
#define W_ACK_PAYLOAD       0xA8  // Write Payload to be transmitted with ACK packet
#define W_TX_PAYLOAD_NOACK  0xB0  // Write TX payload and disable AUTOACK
#define ACTIVATE            0x50  // Followed by 0x73 to activate R_RX_PL_WID, W_ACK_PAYLOAD, etc.
#define NOP                 0xFF  // No Operation (used to read STATUS register)
#define LENGTH              0x20U // Maximum payload length (32 bytes)

/* --- NRF24L01 REGISTER MAP --- 
 * Internal register addresses
 */
#define CONFIG      0x00  // Configuration (Interrupts, CRC, Power, RX/TX mode)
#define EN_AA       0x01  // Enable 'Auto Acknowledgment' Function
#define EN_RXADDR   0x02  // Enabled RX Addresses (Pipes 0-5)
#define SETUP_AW    0x03  // Setup of Address Widths (3, 4, or 5 bytes)
#define SETUP_RETR  0x04  // Setup of Automatic Retransmission (Delay & Count)
#define RF_CH       0x05  // RF Channel (Frequency 2400MHz to 2525MHz)
#define RF_SETUP    0x06  // RF Setup (Data Rate and Output Power)
#define STATUS      0x07  // Status Register (Interrupt flags, TX Full, RX Pipe ID)
#define OBSERVE_TX  0x08  // Transmit observe register (Lost & Retransmitted packets)
#define RPD         0x09  // Received Power Detector (Carrier Detect)
#define RX_ADDR_P0  0x0A  // Receive address data pipe 0 (5 bytes)
#define RX_ADDR_P1  0x0B  // Receive address data pipe 1 (5 bytes)
#define RX_ADDR_P2  0x0C  // Receive address data pipe 2 (LSB byte)
#define RX_ADDR_P3  0x0D  // Receive address data pipe 3 (LSB byte)
#define RX_ADDR_P4  0x0E  // Receive address data pipe 4 (LSB byte)
#define RX_ADDR_P5  0x0F  // Receive address data pipe 5 (LSB byte)
#define TX_ADDR     0x10  // Transmit address (5 bytes)
#define RX_PW_P0    0x11  // Number of bytes in RX payload in data pipe 0
#define RX_PW_P1    0x12  // Number of bytes in RX payload in data pipe 1
#define RX_PW_P2    0x13  // Number of bytes in RX payload in data pipe 2
#define RX_PW_P3    0x14  // Number of bytes in RX payload in data pipe 3
#define RX_PW_P4    0x15  // Number of bytes in RX payload in data pipe 4
#define RX_PW_P5    0x16  // Number of bytes in RX payload in data pipe 5
#define FIFO_STATUS 0x17  // FIFO Status Register (TX/RX Full/Empty flags)
#define DYNPD       0x1C  // Enable dynamic payload length per pipe
#define FEATURE     0x1D  // Feature Register (Dynamic PL, ACK Payload, etc.)

/* --- BIT DEFINITIONS --- 
 * Important bit positions in registers
 */
#define MASK_RX_DR  6     // Mask interrupt caused by RX_DR (Data Ready RX)
#define MASK_TX_DS  5     // Mask interrupt caused by TX_DS (Data Sent TX)
#define MASK_MAX_RT 4     // Mask interrupt caused by MAX_RT (Max Retransmits)
#define EN_CRC      3     // Enable CRC
#define CRCO        2     // CRC encoding scheme (0: 1 byte, 1: 2 bytes)
#define PWR_UP      1     // Power up (1: Power Up, 0: Power Down)
#define PRIM_RX     0     // RX/TX control (1: PRX, 0: PTX)

#ifndef NULL_PTR
    #define NULL_PTR (void*)(0x00)
#endif
#ifndef FALSE
	#define FALSE 0
#endif
#ifndef TRUE
	#define TRUE  1
#endif

#define NRF24_TRANSMIT_MODE 0
#define NRF24_RECEIVE_MODE  1

/* --- Typedefs and Structures --- */

typedef enum 
{
    NRF24_LOW_POWER = 0,    // -18 dBm
    NRF24_MEDIUM_POWER = 1, // -12 dBm
    NRF24_MAX_POWER = 2,    // -6 dBm
    NRF24_ULTRA_POWER = 3   // 0 dBm
} NRF24L01_OutputPowerTypedef;

typedef enum
{
    NRF24L01_1MBPS,
    NRF24L01_2MBPS
} NRF24L01_AirDataDateTypedef;

typedef enum
{
	NRF24_UNINIT,
	NRF24_INIT,
	NRF24_IDEL,
	NRF24_BUSY,
} NRF24L01_StateTypedef;

typedef struct
{
    /* Hardware Interface */
    SPI_HandleTypeDef* SPI_Instance;      /**< SPI peripheral instance */
    uint16_t                  ChipSelectPin;     /**< CSN pin number */
    GPIO_TypeDef* ChipSelectPort;    /**< CSN GPIO port */
    uint16_t                  InterruptPin;      /**< IRQ pin number */
    GPIO_TypeDef* InterruptPort;     /**< IRQ GPIO port */
    uint16_t                  ChipEnablePin;     /**< CE pin to activate RX/TX mode */
    GPIO_TypeDef* ChipEnablePort;    /**< CE GPIO port */

    /* Operation Mode */
    uint8_t                   InterruptMode;     /**< Operating mode: Polling or Interrupt */
    NRF24L01_OutputPowerTypedef   NRF24L01_OutputPower;     
    NRF24L01_AirDataDateTypedef   NRF24L01_AirDataDate;  
    uint8_t                   DynamicPayloadEnable;
    uint8_t                   PayloadWithAckEnable;

    /* NRF24 Configuration */
	uint32_t FrequencyChannel;     /**< RF Channel frequency (0-125) */
	uint8_t  AutoRetransmitCount;  /**< Number of retries (0-15) */
	uint8_t  AutoRetransmitDelay;  /**< Retransmit delay (0-15, multiple of 250us) */

	/* Addresses for 6 Pipes (5 bytes each) */
	uint8_t  RxAddressP0[5];
	uint8_t  RxAddressP1[5];
	uint8_t  RxAddressP2_5[4];     /**< LSB for Pipes 2 through 5 */
	uint8_t  TxAddress[5];
	NRF24L01_StateTypedef State;

    /* Callback Functions */
    void (*RxCallback)(uint8_t* data, uint8_t size);  /**< Data received callback */
    void (*TxCallback)(void);                         /**< Data transmitted callback */
} NRF24L01_HandleTypedef;

typedef enum 
{
    STD_E_OK,
    STD_E_NOT_OK,
    STD_E_BUSY
} NRF24L01_ReturnType;

/* --- Function Prototypes --- */

NRF24L01_ReturnType DRV_Nrf24l01Init(NRF24L01_HandleTypedef* NRF24L01Instance);
void DRV_Nrf24l01Deinit(NRF24L01_HandleTypedef* NRF24L01Instance);

NRF24L01_ReturnType DRV_Nrf24l01Receive(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t* pipeID, uint8_t *destinationPtr, uint32_t timeout);
void DRV_Nrf24l01WritePayloadWithAck(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t pipeID, uint8_t *sourcePtr);
NRF24L01_ReturnType DRV_Nrf24l01ReadPayloadWithAck(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t *destinationPtr, uint32_t timeout);

NRF24L01_ReturnType DRV_Nrf24l01Transmit(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t channelID, uint8_t *targetAddr, uint8_t *sourcePtr);
NRF24L01_ReturnType DRV_Nrf24l01SwitchMode(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t mode);

NRF24L01_ReturnType DRV_Nrf24l01SetDataRate(NRF24L01_HandleTypedef* NRF24L01Instance, NRF24L01_AirDataDateTypedef speed);
NRF24L01_ReturnType DRV_Nrf24l01SetPALevel(NRF24L01_HandleTypedef* NRF24L01Instance, NRF24L01_OutputPowerTypedef level);

NRF24L01_ReturnType DRV_Nrf24l01OpenWritingPipe(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t channelID, uint8_t *address);
NRF24L01_ReturnType DRV_Nrf24l01OpenReadingPipe(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t pipeNum, uint8_t *address);

#endif /* _DRV_NRF24L01_H_ */
