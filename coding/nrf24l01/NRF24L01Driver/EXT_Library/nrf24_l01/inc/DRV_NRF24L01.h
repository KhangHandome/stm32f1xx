#ifndef _DRV_NRF24L01_H_
#define _DRV_NRF24L01_H_

#include "gpio.h"
#include "spi.h"

/* --- NRF24L01 COMMANDS --- 
 * Các lệnh này được gửi trực tiếp qua SPI để điều khiển chip
 */
#define R_REGISTER    0x00  // Đọc thanh ghi (kết hợp với địa chỉ 5 bit)
#define W_REGISTER    0x20  // Ghi thanh ghi (kết hợp với địa chỉ 5 bit)
#define R_RX_PAYLOAD  0x61  // Đọc dữ liệu nhận được từ RX FIFO
#define W_TX_PAYLOAD  0xA0  // Ghi dữ liệu vào TX FIFO
#define FLUSH_TX      0xE1  // Xóa sạch TX FIFO
#define FLUSH_RX      0xE2  // Xóa sạch RX FIFO
#define REUSE_TX_PL   0xE3  // Dùng lại gói tin TX cuối cùng
#define R_RX_PL_WID   0x60  // Đọc độ dài payload của gói tin vừa nhận
#define W_ACK_PAYLOAD 0xA8  // Ghi ACK payload (kèm 3 bit pipe ID)
#define W_TX_PAYLOAD_NOACK 0xB0 // Ghi TX payload nhưng không yêu cầu ACK
#define NOP           0xFF  // Không thực hiện lệnh (dùng để đọc trạng thái STATUS)

/* --- NRF24L01 REGISTER MAP --- 
 * Địa chỉ các thanh ghi cấu hình nội bộ
 */
#define CONFIG      0x00  // Cấu hình (Interrupt, CRC, PWR, RX/TX mode)
#define EN_AA       0x01  // Bật/tắt tính năng Auto Acknowledgment
#define EN_RXADDR   0x02  // Kích hoạt các địa chỉ RX (Pipes 0-5)
#define SETUP_AW    0x03  // Thiết lập độ rộng địa chỉ (3, 4 hoặc 5 bytes)
#define SETUP_RETR  0x04  // Cấu hình tự động gửi lại (Delay & Count)
#define RF_CH       0x05  // Kênh tần số RF (từ 2400MHz đến 2525MHz)
#define RF_SETUP    0x06  // Tốc độ truyền dữ liệu và Công suất phát
#define STATUS      0x07  // Trạng thái (Ngắt, TX Full, RX Pipe ID)
#define OBSERVE_TX  0x08  // Quan sát lỗi truyền (Gói bị mất/gửi lại)
#define RPD         0x09  // Phát hiện công suất nhận (Carrier Detect)
#define RX_ADDR_P0  0x0A  // Địa chỉ nhận Pipe 0 (5 bytes)
#define RX_ADDR_P1  0x0B  // Địa chỉ nhận Pipe 1 (5 bytes)
#define RX_ADDR_P2  0x0C  // Địa chỉ nhận Pipe 2 (1 byte)
#define RX_ADDR_P3  0x0D  // Địa chỉ nhận Pipe 3 (1 byte)
#define RX_ADDR_P4  0x0E  // Địa chỉ nhận Pipe 4 (1 byte)
#define RX_ADDR_P5  0x0F  // Địa chỉ nhận Pipe 5 (1 byte)
#define TX_ADDR     0x10  // Địa chỉ đích để truyền (5 bytes)
#define RX_PW_P0    0x11  // Độ rộng payload Pipe 0 (1-32 bytes)
#define RX_PW_P1    0x12  // Độ rộng payload Pipe 1 (1-32 bytes)
#define RX_PW_P2    0x13  // Độ rộng payload Pipe 2 (1-32 bytes)
#define RX_PW_P3    0x14  // Độ rộng payload Pipe 3 (1-32 bytes)
#define RX_PW_P4    0x15  // Độ rộng payload Pipe 4 (1-32 bytes)
#define RX_PW_P5    0x16  // Độ rộng payload Pipe 5 (1-32 bytes)
#define FIFO_STATUS 0x17  // Trạng thái các hàng đợi FIFO (TX/RX Full/Empty)
#define DYNPD       0x1C  // Kích hoạt Dynamic Payload Length
#define FEATURE     0x1D  // Các tính năng mở rộng (Dynamic PL, ACK Payload)

/* --- BIT DEFINITIONS --- 
 * Một số bit quan trọng thường dùng
 */
#define MASK_RX_DR  6     // Bit ngắt khi nhận dữ liệu
#define MASK_TX_DS  5     // Bit ngắt khi gửi dữ liệu xong
#define MASK_MAX_RT 4     // Bit ngắt khi gửi lại quá số lần quy định
#define EN_CRC      3     // Bật CRC
#define CRCO        2     // Độ dài CRC (0: 1 byte, 1: 2 bytes)
#define PWR_UP      1     // Bật nguồn (1: Power Up, 0: Power Down)
#define PRIM_RX     0     // Chọn chế độ (1: PRX, 0: PTX)

/*
 * Typedef and structure
 */
typedef enum
{
    NRF24_MODE_POLLING,
    NRF24_MODE_INTERRUPT
} NRF24L01_OperationModeTypedef;
typedef enum 
{
    NRF24_LOW_POWER,
    NRF24_MEDIUM_POWER,
    NRF24_MAX_POWER,
    NRF24_ULTRA_POWER
} NRF24L01_OutputPowerTypedef ; 

typedef struct
{
    /* Hardware Interface */
    SPI_HandleTypeDef*        SPI_Instance;      /**< SPI peripheral instance for communication */
    uint16_t                  ChipSelectPin;     /**< NSS/CS pin number */
    GPIO_TypeDef*             ChipSelectPort;    /**< NSS/CS GPIO port */
    uint16_t                  InterruptPin;      /**< Hardware interrupt pin number */
    GPIO_TypeDef*             InterruptPort;     /**< Hardware interrupt port  */
    uint16_t                  ChipEnablePin;     /**< Chip Enable Activate RX or TX mode  */
    GPIO_TypeDef*             ChipEnbalePort;    /**< Chip Enable Activate RX or TX mode  */
    /* Operation Mode */
    NRF24L01_OperationModeTypedef OperationMode;     /**< Polling or interrupt mode */
    NRF24L01_OutputPowerTypedef   NRF24L01_OutputPower;      
    /* NRF24 Configuration */
    uint8_t                   FrequencyChannel; 
    

    
    /* Callback Functions */
    void                      (*RxCallback)(uint8_t* data, uint8_t size);  /**< Receive complete callback */
    void                      (*TxCallback)(void);                         /**< Transmit complete callback */
} NRF24L01_HandleTypedef;

/*
 * Prototype function 
*/
void DRV_Nrf24l01Init();
void DRV_Nrf24l01Deinit();
void DRV_Nrf24l01Receive();
void DRV_Nrf24l01Transmit();
void DRV_Nrf24l01SetChannel();
void DRV_Nrf24l01SetDataRate();
void DRV_Nrf24l01SetPALevel();
void DRV_Nrf24l01OpenWritingPipe();
void DRV_Nrf24l01OpenReadingPiple();
void DRV_Nrf24l01SwitchMode();
#endif /* _DRV_NRF24L01_H_ */