#include "main.h"
#define FIRMWARE_ADDRESS 0x0800A000
static void Enable_Timer(void);
static void Enable_GPIO(void);
static void VTOR_Init(uint32_t address_firmware);
int main(void)
{
	VTOR_Init(FIRMWARE_ADDRESS);
    Enable_GPIO();
    Enable_Timer();
	__enable_irq();

    while (1)
    {
    }
}
static void Enable_Timer(void)
{

    /* Enable TIM2 clock */
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* Prescaler: 8MHz / 8000 = 1kHz (1 ms) */
    TIM2->PSC = 8000 - 1;

    /* Auto reload: 200 ms */
    TIM2->ARR = 200 - 1;

    /* Enable update interrupt */
    TIM2->DIER |= TIM_DIER_UIE;

    /* Enable TIM2 interrupt in NVIC */
    NVIC_EnableIRQ(TIM2_IRQn);

    /* Start timer */
    TIM2->CR1 |= TIM_CR1_CEN;
}
static void Enable_GPIO(void)
{
    /* Enable clock GPIOC */
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    /* PC13 output push-pull, max speed 2 MHz
     * CNF13 = 00
     * MODE13 = 10
     */
    GPIOC->CRH &= ~(0xF << 20);     // clear PC13
    GPIOC->CRH |=  (0x2 << 20);     // output 2 MHz push-pull

    /* LED OFF initially (PC13 active low) */
    GPIOC->BSRR = (1 << 13);
}
static void VTOR_Init(uint32_t address_firmware)
{
	/* Setup for register VTOR
	 *
	 */
	SCB->VTOR = address_firmware;
	/*
	 * Setup for stack pointer
	 */
	__asm volatile(
	"mov sp, %0\n"
	:
	:"r" (*(uint32_t*)( address_firmware))
	);
}
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF)
    {
        TIM2->SR &= ~TIM_SR_UIF;   // clear flag

        /* Toggle PC13 */
        GPIOC->ODR ^= (1 << 13);
    }
}
