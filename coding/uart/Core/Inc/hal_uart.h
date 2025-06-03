/*
 * hal_uart.h
 *
 *  Created on: Jun 3, 2025
 *      Author: KhangMT
 */

#ifndef INC_HAL_UART_H_
#define INC_HAL_UART_H_
#include "stm32f1xx.h"
extern void HAL_UART_Init(USART_TypeDef* usart);
extern void HAL_UART_Send(USART_TypeDef* usart,uint8_t* str);
extern void HAL_UART_ReadChar(USART_TypeDef* usart, uint8_t* str);
extern void HAL_UART_Deinit(USART_TypeDef* usart);


#endif /* INC_HAL_UART_H_ */
