#ifndef _HAL_USART_H_
#define _HAL_USART_H_
/*
 * Inlude
*/
#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>
/*
 * define 
*/
#ifndef NULL_PTR
#define NULL_PTR (void*)(0x00)
#endif
/*
 * typedef and struct 
*/

// Define callback function 
typedef void (*HAL_UART_CallBack_t)(void);

// Cấu hình parity
typedef enum {
    HAL_UART_PARITY_NONE,
    HAL_UART_PARITY_EVEN,
    HAL_UART_PARITY_ODD
} HAL_UART_Parity_t;

// Số bit dữ liệu
typedef enum {
    HAL_UART_DATA_8BITS,
    HAL_UART_DATA_9BITS
} HAL_UART_DataBits_t;

// Ngắt
typedef enum {
    HAL_UART_INTERRUPT_DISABLE,
    HAL_UART_INTERRUPT_ENABLE
} HAL_UART_Interrupt_t;

// DMA
typedef enum {
    HAL_UART_DMA_DISABLE,
    HAL_UART_DMA_ENABLE
} HAL_UART_DMA_t;

// SWAP chân TX/RX
typedef enum {
    HAL_UART_SWAP_DISABLE,
    HAL_UART_SWAP_ENABLE
} HAL_UART_SwapPins_t;

// Cấu hình UART
typedef struct {
    USART_TypeDef*        uart_port;       // UART1, UART2, ...
    uint32_t              baudrate;        // ex: 9600
    uint32_t              clock_input;     // ex: 8000000 (8MHz)
    HAL_UART_Parity_t     parity;          // none, even, odd
    HAL_UART_DataBits_t   data_bits;       // 8 or 9 bits
    HAL_UART_Interrupt_t  tx_interrupt;   // enable/disable interrupt
    HAL_UART_Interrupt_t  rx_interrupt;   // enable/disable interrupt
    HAL_UART_DMA_t        use_dma;         // enable/disable DMA
    HAL_UART_SwapPins_t   swap_pins;       // enable/disable pin swap

    // Thêm cho DMA
    uint8_t* dma_rx_buffer;
    uint16_t dma_rx_size;
} HAL_UART_t;

// === API PROTOTYPES ===

// Khởi tạo UART
extern void HAL_UART_Init(const HAL_UART_t* config);

// Gửi dữ liệu (polling, interrupt hoặc DMA tuỳ config)
extern void HAL_UART_Transfer(const HAL_UART_t* uart, const uint8_t* data);

// Đọc 1 byte (blocking hoặc non-blocking tuỳ config)
extern uint8_t HAL_UART_Read(const HAL_UART_t* uart);
extern void HAL_UART_SetDMABuffer(const HAL_UART_t* uart);
// Đăng ký callback cho DMA hoàn tất
extern void HAL_UART_SetDMACallback(const HAL_UART_t* uart, HAL_UART_CallBack_t cb);

// Đăng ký callback cho ngắt RX hoặc TX
extern void HAL_UART_SetTxCallBack(const HAL_UART_t* uart, HAL_UART_CallBack_t cb_tx);
extern void HAL_UART_SetRxCallBack(const HAL_UART_t* uart, HAL_UART_CallBack_t cb_rx);

#endif
