/*
    Include 
*/
#include "hal_usart.h"
#include "stm32f1xx.h"

/*
    Variable 
*/
static HAL_UART_CallBack_t UART1_RX_Callback = NULL_PTR;
static HAL_UART_CallBack_t UART1_TX_Callback = NULL_PTR;
static HAL_UART_CallBack_t DMA_Callback      = NULL_PTR;

/*
    Prototype 
*/
static void HAL_UART_Transfer_Char(const HAL_UART_t* uart, uint8_t c);
static void HAL_UART_Calculation_BRR(uint32_t ClockInput, uint32_t Baudrate, uint16_t* Mantissa, uint16_t* Fraction);

/*
    Initialize UART with configuration: clock, GPIO, BRR calculation, DMA/Interrupt/Pin swap if enabled
*/
void HAL_UART_Init(const HAL_UART_t* config)
{
    USART_TypeDef* usart = NULL;

    // Select UART port and enable corresponding clocks and configure GPIO
    if (config->uart_port == USART1) {
        usart = USART1;
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
        RCC->AHBENR  |= RCC_AHBENR_DMA1EN;

        // PA9: TX (AF Push-Pull), PA10: RX (Default input floating)
        GPIOA->CRH |= (0x3 << GPIO_CRH_MODE9_Pos);
        GPIOA->CRH |= (0x2 << GPIO_CRH_CNF9_Pos);

    } else if (config->uart_port == USART2) {
        usart = USART2;
        RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
        RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

        // PA2: TX (AF Push-Pull), PA3: RX (Input floating)
        GPIOA->CRL &= ~GPIO_CRL_CNF2;
        GPIOA->CRL |= GPIO_CRL_CNF2_1;
        GPIOA->CRL |= GPIO_CRL_MODE2;

        GPIOA->CRL &= ~GPIO_CRL_CNF3;
        GPIOA->CRL |= GPIO_CRL_CNF3_0;
        GPIOA->CRL &= ~GPIO_CRL_MODE3;
    }

    // Calculate and set BRR register for baudrate
    uint16_t mantissa = 0, fraction = 0;
    HAL_UART_Calculation_BRR(config->clock_input, config->baudrate, &mantissa, &fraction);
    usart->BRR = (mantissa << 4) | (fraction & 0x0F);

    // Enable UART: Transmit, Receive, UART Enable
    usart->CR1 |= USART_CR1_TE | USART_CR1_RE;
    usart->CR1 |= USART_CR1_UE;

    // Enable DMA if required
    if (config->use_dma == HAL_UART_DMA_ENABLE) {
        usart->CR3 |= USART_CR3_DMAR;

        // Cấu hình DMA1 Channel5 cho USART1_RX (theo Reference Manual)
        DMA1_Channel5->CCR &= ~DMA_CCR_EN; // Tắt DMA trước khi cấu hình

        DMA1_Channel5->CPAR = (uint32_t)&(USART1->DR);          // Địa chỉ ngoại vi: thanh ghi dữ liệu USART1
        DMA1_Channel5->CMAR = (uint32_t)(config->dma_rx_buffer); // Địa chỉ bộ nhớ đích
        DMA1_Channel5->CNDTR = config->dma_rx_size;             // Số lượng byte cần nhận

        // Cấu hình:
        // - Đọc từ ngoại vi
        // - Tăng địa chỉ bộ nhớ
        // - Kích hoạt ngắt hoàn thành
        // - Ưu tiên cao
        // - Chế độ circular nếu cần
        DMA1_Channel5->CCR =
              DMA_CCR_MINC      // Memory increment
            | DMA_CCR_TCIE      // Transfer complete interrupt enable
            | DMA_CCR_PL_1      // Priority level high
            | DMA_CCR_CIRC      // Optional: circular mode (nếu muốn lặp vòng)
            | DMA_CCR_EN;       // Enable DMA

        // Bật ngắt DMA trong NVIC nếu cần
        NVIC_EnableIRQ(DMA1_Channel5_IRQn);
    }

    // Enable interrupt and NVIC if required
    if (config->rx_interrupt == HAL_UART_INTERRUPT_ENABLE || config->tx_interrupt == HAL_UART_INTERRUPT_ENABLE) {
    	if(config->rx_interrupt == HAL_UART_INTERRUPT_ENABLE)
    	{
            usart->CR1 |= USART_CR1_RXNEIE;  // Enable receive interrupt
    	}
    	if(config->tx_interrupt == HAL_UART_INTERRUPT_ENABLE)
    	{
            usart->CR1 |= USART_CR1_TXEIE;   // Enable transmit interrupt (if needed)
    	}
        if (config->uart_port == USART1)
            NVIC->ISER[USART1_IRQn / 32] |= (1 << (USART1_IRQn % 32));
        else if (config->uart_port == USART2)
            NVIC->ISER[USART2_IRQn / 32] |= (1 << (USART2_IRQn % 32));
    }

    // Remap UART pins if required
    if (config->swap_pins == HAL_UART_SWAP_ENABLE) {
        AFIO->MAPR |= AFIO_MAPR_USART1_REMAP;
    }
}

/*
    Transfer a null-terminated string over UART
*/
void HAL_UART_Transfer(const HAL_UART_t* uart, const uint8_t* data)
{
    uint16_t index = 0;
    while (data[index] != '\0') {
        HAL_UART_Transfer_Char(uart, data[index]);
        index++;
    }
}

/*
    Blocking read one byte from UART
*/
uint8_t HAL_UART_Read(const HAL_UART_t* uart)
{
	uint8_t retVal = 0 ;
    while (!(uart->uart_port->SR & USART_SR_RXNE)) {
        // Wait until data is received
    }
    retVal = (uint8_t)(uart->uart_port->DR & 0xFF);
    return retVal;
}

/*
    Internal: Transfer a single character over UART
*/
static void HAL_UART_Transfer_Char(const HAL_UART_t* uart, uint8_t c)
{
    while (!(uart->uart_port->SR & USART_SR_TXE)) {
        // Wait until transmit buffer is empty
    }
    uart->uart_port->DR = c;
}

/*
    Internal: Calculate BRR value from clock and baudrate
*/
static void HAL_UART_Calculation_BRR(uint32_t ClockInput, uint32_t Baudrate, uint16_t* Mantissa, uint16_t* Fraction)
{
    float usartdiv = (float)ClockInput / (16 * Baudrate);
    *Mantissa = (uint16_t)usartdiv;
    *Fraction = (uint16_t)((usartdiv - *Mantissa) * 16);
}

/*
    Set callback function for DMA transfer complete (currently only for USART1)
*/
void HAL_UART_SetDMACallback(const HAL_UART_t* uart, HAL_UART_CallBack_t cb)
{
    if (uart->uart_port == USART1) {
    	DMA_Callback = cb;
    }
}
void HAL_UART_SetDMABuffer(const HAL_UART_t* uart)
{
    if (uart->uart_port == USART1) {
        DMA1_Channel5->CCR &= ~DMA_CCR_EN;       // Tắt DMA trước
        DMA1_Channel5->CMAR = (uint32_t)uart->dma_rx_buffer;  // Cập nhật địa chỉ bộ nhớ
        DMA1_Channel5->CNDTR = (uint32_t)uart->dma_rx_size;             // Cập nhật số byte
        DMA1_Channel5->CCR |= DMA_CCR_EN;        // Bật lại DMA
    }
}
/*
    Set callback function for interrupt receive complete (currently only for USART1)
*/
void HAL_UART_SetTxCallBack(const HAL_UART_t* uart,HAL_UART_CallBack_t cb_tx)
{
    if (uart->uart_port == USART1)
    {
    	UART1_TX_Callback = cb_tx;
    }
}
void HAL_UART_SetRxCallBack(const HAL_UART_t* uart, HAL_UART_CallBack_t cb_rx)
{
    if (uart->uart_port == USART1)
    {
    	UART1_RX_Callback = cb_rx;
    }
}

/*
    USART1 interrupt handler: distinguish RXNE vs TXE and call callback
*/
void USART1_IRQHandler()
{
    if (USART1->SR & USART_SR_RXNE) {
        // Data received
        if (UART1_RX_Callback != NULL_PTR) {
        	UART1_RX_Callback();  // You may define a more specific RX callback
        }
    }

    if (USART1->SR & USART_SR_TXE) {
        // Transmit buffer empty
        // Implement TX callback if needed or disable TXE interrupt when done
    	if(UART1_TX_Callback != NULL_PTR)
    	{
        	UART1_TX_Callback();
            USART1->CR1 &= ~USART_CR1_TXEIE;  // Optional: disable to prevent repeated interrupts
    	}
    }
}
