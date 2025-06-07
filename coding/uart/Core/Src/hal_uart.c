/*
 * hal_uart.c
 *
 *  Created on: Jun 3, 2025
 *      Author: KhangMT
 */
#include "hal_uart.h"
#include "stm32f1xx.h"
#include "stdint.h"

static void HAL_UART_Send_Char(USART_TypeDef* usart, uint8_t c);
typedef enum {

};
uint8_t data[32];
// Gửi 1 ký tự
static void HAL_UART_Send_Char(USART_TypeDef* usart, uint8_t c)
{
    while (!(usart->SR & USART_SR_TXE)) {
        // chờ tới khi TXE (Transmit data register empty) được set
    }
    usart->DR = c;
}

// Khởi tạo UART
void HAL_UART_Init(USART_TypeDef* usart)
{
    if (usart == USART1) {
        // Bật clock cho GPIOA và USART1
    	RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
    	RCC->AHBENR |= RCC_AHBENR_DMA1EN;

        /*PA9: TX
         * PA10 : RX
         * */
        GPIOA->CRH |= (0x3 << GPIO_CRH_MODE9_Pos); // Set pa9 to output max speed 50Mhz
        GPIOA->CRH |= (0x2 << GPIO_CRH_CNF9_Pos ); // Set pa9 to output alternative function push pull
        /* */

    } else if (usart == USART2) {
        RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
        RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

        // PA2 = TX, PA3 = RX
        GPIOA->CRL &= ~GPIO_CRL_CNF2;
        GPIOA->CRL |= GPIO_CRL_CNF2_1;
        GPIOA->CRL |= GPIO_CRL_MODE2;

        GPIOA->CRL &= ~GPIO_CRL_CNF3;
        GPIOA->CRL |= GPIO_CRL_CNF3_0;
        GPIOA->CRL &= ~GPIO_CRL_MODE3;
    }

    // Cấu hình Baudrate = 9600 với PCLK = 72MHz → USARTDIV ≈ 468.75
    // BRR = mantissa << 4 | fraction = 468 << 4 | 12 = 0x1D2C
    usart->BRR = (52 << 4) | 1; // Mới, dành cho 8MHz

    // Enable Transmit, Receive, and USART
    usart->CR1 |= USART_CR1_TE | USART_CR1_RE;
    usart->CR3 |= USART_CR3_DMAR;
    usart->CR1 |= USART_CR1_UE;

//    /*Enable interrupt receive */
//    usart->CR1 |= USART_CR1_RXNEIE;
//    NVIC->ISER[USART1_IRQn/32] |= (1 << (USART1_IRQn % 32)) ;

    /*Using dma to receive data */
    /*Enable bit DMA in usart */

    DMA1_Channel5->CCR |= DMA_CCR_MINC | DMA_CCR_CIRC | (0x02 << DMA_CCR_PL_Pos);

    DMA1_Channel5->CNDTR = 4;
    DMA1_Channel5->CPAR  = (uint32_t)&(usart->DR);
    DMA1_Channel5->CMAR  = (uint32_t)data ;


    DMA1_Channel5->CCR |= DMA_CCR_TCIE;
    DMA1_Channel5->CCR |= DMA_CCR_EN;
    NVIC->ISER[DMA1_Channel5_IRQn/32] |= (1 << (DMA1_Channel5_IRQn % 32)) ;

}
void DMA1_Channel5_IRQHandler()
{
    if (DMA1->ISR & DMA_ISR_TCIF5) {
        DMA1->IFCR |= DMA_IFCR_CTCIF5; // Clear interrupt flag

        // Xử lý dữ liệu ở đây nếu cần
        // Ví dụ: parse `data[]` hoặc đặt cờ cho ứng dụng biết dữ liệu đã có
    }
}
void USART1_IRQHandler()
{
	uint8_t a = 0 ;
	a = (USART1->DR & 0xFF);

	//Interrupt is here
}
// Gửi chuỗi ký tự (kết thúc bằng ký tự '\n')
void HAL_UART_Send(USART_TypeDef* usart, uint8_t* str)
{
    uint16_t index = 0;
    while (str[index] != '\n') {
        HAL_UART_Send_Char(usart, str[index]);
        index++;
    }
}

// Nhận 1 ký tự
void HAL_UART_ReadChar(USART_TypeDef* usart, uint8_t* str)
{
    while (!(usart->SR & USART_SR_RXNE)) {
        // Chờ cho đến khi có dữ liệu nhận
    }
    *str = (uint8_t)(usart->DR & 0xFF);
}

// Hủy khởi tạo UART
void HAL_UART_Deinit(USART_TypeDef* usart)
{
    usart->CR1 &= ~USART_CR1_UE;
}
