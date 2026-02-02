#ifndef _LORA_H_
#define _LORA_H_

#include "spi.h"
#include "gpio.h"
#include "stdint.h"
#include "Lora_Reg.h"
/*
 * Define write or read data from Lora
 */
#define READ_FROM_LORA 0x7F
#define WRITE_TO_LORA  0x80

typedef enum {
	E_OK,
	E_NOT_OK
} Std_return_type;
/*
 * Prototype function
 */
void DRV_Lora_Init();
void DRV_Lora_Deinit();
void DRV_Lora_Reset();
void DRV_Lora_Transmit(uint8_t *data, uint8_t size);
Std_return_type DRV_Lora_Receive(uint8_t* data, uint8_t size);
uint8_t DRV_Lora_GetVersionInfor();

#endif /*_LORA_H_*/
