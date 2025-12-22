#include "main.h"

#define FIRMWARE_A 0x08004000
#define FIRMWARE_B 0x0800A000

static void switch_firmware(uint32_t address_firmware);

int main(void)
{
    switch_firmware(FIRMWARE_B);

    while (1); // never return
}

static void switch_firmware(uint32_t address_firmware)
{
    uint32_t msp;
    uint32_t reset;

    __disable_irq();

    /* Set vector table */
    SCB->VTOR = address_firmware;

    /* Set MSP */
    msp = *(uint32_t *)address_firmware;
    __asm volatile ("msr msp, %0" : : "r" (msp) : );

    /* Jump to Reset_Handler */
    reset = *(uint32_t *)(address_firmware + 4);
    void (*reset_handler)(void) = (void (*)(void))reset;

    reset_handler();
}
