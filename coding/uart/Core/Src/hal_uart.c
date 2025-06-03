/*
 * hal_uart.c
 *
 *  Created on: Jun 3, 2025
 *      Author: KhangMT
 */
#include "hal_uart.h"
#include "stm32f1xx.h"
#include "stdint.h"
static void HAL_UART_Send_Char(USART_TypeDef* usart,uint8_t c);
static void HAL_UART_Send_Char(USART_TypeDef* usart,uint8_t c)
{
	while( (usart->SR) >> USART_SR_TXE_Pos == 0 )
	{

	}
	usart->DR = c;
}
void HAL_UART_Init(USART_TypeDef* usart)
{
	/*Enable uart */
	usart->CR1 |= (USART_CR1_UE_Msk & ( 0 << USART_CR1_UE_Pos));
	/*Setup word length for usart */
	usart->CR1 |= (USART_CR1_M_Msk & (0 << USART_CR1_M_Pos));
	/*Select the number of stop bits uin usart 2 */
	usart->CR2 |= ((0 << USART_CR2_STOP_Pos) & USART_CR2_STOP_Msk);
	/*Enable or disable dma for receive and transmiter */
	usart->CR3 |= ((0 << USART_CR3_DMAT_Pos) & USART_CR3_DMAT_Msk);
	usart->CR3 |= ((0 << USART_CR3_DMAR_Pos) & USART_CR3_DMAR_Msk);
	/*Select DMA enable (DMAT) in USART_CR3 if Multi buffer Communication is to take
	place. Configure the DMA register as explained in multibuffer communication.*/
	/*Setup baurate */
	/*Baud = f(CK) / ( 16 * USARTDIV)*/
	//	USARDIV = 52.083
	usart->BRR |= (((52 << USART_BRR_DIV_Mantissa_Pos) & USART_BRR_DIV_Mantissa_Msk) | (((13 << USART_BRR_DIV_Fraction_Pos) & USART_BRR_DIV_Fraction_Msk)));
	/*Enable transmit and enable receive */
	usart->CR1 |= (USART_CR1_TE | USART_CR1_RE);

	RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
	RCC->APB2ENR  |= RCC_APB2ENR_USART1EN;
	AFIO->MAPR    |= AFIO_MAPR_USART1_REMAP | AFIO_MAPR_USART2_REMAP | AFIO_MAPR_USART3_REMAP;
}
void HAL_UART_Send(USART_TypeDef* usart,uint8_t* str)
{
	uint16_t index = 0 ;
	while ( str[index] != (uint8_t)'\n')
	{
		HAL_UART_Send_Char(usart, *(str+index));
		index += 1 ;
	}
}
void HAL_UART_ReadChar(USART_TypeDef* usart, uint8_t* str)
{
	while( (usart->SR) >> USART_SR_RXNE_Pos == 0 )
	{

	}
	(*str) = usart->DR;
}
extern void HAL_UART_Deinit(USART_TypeDef* usart);

