# 📡 NRF24L01+ Driver for STM32 HAL

## 1️⃣ Introduction

This is a high-performance, lightweight C driver for the **nRF24L01+** 2.4GHz wireless transceiver from Nordic Semiconductor.  

Built specifically for STM32 microcontrollers using the HAL (Hardware Abstraction Layer), this driver provides a clean and easy-to-use API for reliable 2.4GHz communication.

It supports both polling and interrupt-based operation modes.

---

## 🚀 Key Features

- **Multi-Pipe Communication**  
  Supports all 6 hardware data pipes (P0–P5).

- **Auto-ACK & Retransmission**  
  Hardware-level automatic acknowledgment and retry handling.

- **Dynamic Payload**  
  Supports variable packet lengths up to 32 bytes.

- **ACK Payloads**  
  Send data back to transmitters without manual mode switching.

- **Mode Switching**  
  Simple API to toggle between:
  - PTX (Primary Transmitter)
  - PRX (Primary Receiver)

---

# 2️⃣ Wiring Diagram

The nRF24L01+ communicates via SPI.

⚠️ **Important:**  
- Use a stable **3.3V power supply**
- Do NOT connect VCC to 5V
- Add a 10µF capacitor near the module for stability

---

## 🔌 Pin Connections

| NRF24L01 Pin | Function       | STM32 Example Pin | Description |
|--------------|---------------|-------------------|-------------|
| VCC          | Power         | 3.3V              | Do not use 5V |
| GND          | Ground        | GND               | Common ground |
| CE           | Chip Enable   | PB0               | Controls RX/TX active mode |
| CSN          | SPI Select    | PB1               | SPI Slave Select |
| SCK          | SPI Clock     | PA5               | SPI Serial Clock |
| MOSI         | SPI MOSI      | PA7               | Master Out Slave In |
| MISO         | SPI MISO      | PA6               | Master In Slave Out |
| IRQ          | Interrupt     | PA1 (Optional)    | Active Low Interrupt |

---

# 3️⃣ Guide: Implementation

## 📌 Configuration Structure

All settings are handled through:

You must initialize this structure before calling the init function.

### Example Configuration

```c
NRF24L01_HandleTypedef Instance0 =
{
    .SPI_Instance = &hspi1,
    .ChipSelectPin = CHIP_Select_Pin,
    .ChipSelectPort = CHIP_Select_GPIO_Port,
    .InterruptPin = CHIP_Irq_Pin,
    .InterruptPort = CHIP_Irq_GPIO_Port,
    .ChipEnablePin = CHIP_Enable_Pin,
    .ChipEnablePort  = CHIP_Enable_GPIO_Port,
    .InterruptMode = FALSE,
    .NRF24L01_OutputPower = NRF24_MEDIUM_POWER,
    .NRF24L01_AirDataDate = NRF24L01_1MBPS,
    .FrequencyChannel     = 120,
    .AutoRetransmitCount = 10,
    .AutoRetransmitDelay = 10,
	.DynamicPayloadEnable = TRUE,
	.PayloadWithAckEnable = TRUE,
	.State = NRF24_UNINIT,
    // Gán địa chỉ mặc định
    .RxAddressP0 = {0x11, 0x22, 0x33, 0x44, 0x55},
    .RxAddressP1 = {0x12, 0x13, 0x14, 0x15, 0x16},
	.RxAddressP2_5 = {0x15,0x16,0x17,0x18},
    .TxAddress   = {0x11, 0x22, 0x33, 0x44, 0x55}
};
```
## Interrupt vs Polling
### 🟢 Polling Mode

Set:

hNrf.InterruptMode = FALSE;

You must call:

DRV_Nrf24l01Receive()

frequently inside your main loop.

### 🔵 Interrupt Mode

Set:

hNrf.InterruptMode = TRUE;

Then configure the IRQ pin as EXTI and use the callback function to handle incoming data instantly.

## 4️⃣ Code Examples
### Transmitter Example (PTX)

Sends a 32-byte message every second.
```c
uint8_t tx_data[32] = "STM32_DATA_01";
uint8_t target_addr[5] = {0x11, 0x22, 0x33, 0x44, 0x55};

void main(void)
{
    DRV_Nrf24l01Init(&hNrf);

    while (1)
    {
        if (DRV_Nrf24l01Transmit(&hNrf, 0, target_addr, tx_data) == STD_E_OK)
        {
            // Data Sent Successfully
        }
        else
        {
            // Transmission Failed (Max Retries reached)
        }

        HAL_Delay(1000);
    }
}
```
### Receiver Example (PRX)

Listens for data on Pipe 1.
```c
uint8_t rx_buffer[32];
uint8_t pipe_id;

void main(void)
{
    DRV_Nrf24l01Init(&hNrf);
    DRV_Nrf24l01SwitchMode(&hNrf, NRF24_RECEIVE_MODE);

    while (1)
    {
        if (DRV_Nrf24l01Receive(&hNrf, &pipe_id, rx_buffer, 500) == STD_E_OK)
        {
            // rx_buffer contains received data
            // pipe_id indicates which pipe received it
        }
    }
}
```
### Interrupt-Driven Reception (EXTI)

Handle data instantly when the IRQ pin pulls low.
```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == hNrf.InterruptPin)
    {
        uint8_t status = DRV_Nrf24l01GetStatus(&hNrf);

        if (status & (1 << MASK_RX_DR))
        {
            DRV_Nrf24l01ReadPayloadWithAck(&hNrf, global_rx_buf, 10);
            DRV_Nrf24l01ClearIRQ(&hNrf, MASK_RX_DR);
        }
    }
}
```
# 5️⃣ API Reference Summary
## 📚 API Reference

| # | Function | Return Type | Description |
|---|----------|------------|------------|
| 1 | `DRV_Nrf24l01Init` | `NRF24L01_ReturnType` | Initialize module and perform full configuration |
| 2 | `DRV_Nrf24l01Deinit` | `void` | Deinitialize and power down module |
| 3 | `DRV_Nrf24l01GetStatus` | `uint8_t` | Read STATUS register |
| 4 | `DRV_Nrf24l01DataReceiveAvailable` | `uint8_t` | Return status of Receive |
| 5 | `DRV_Nrf24l01ClearIRQ` | `void` | Clear specific interrupt flag |
| 6 | `DRV_Nrf24l01Receive` | `NRF24L01_ReturnType` | Receive data with timeout |
| 7 | `DRV_Nrf24l01WritePayloadWithAck` | `void` | Write ACK payload to TX FIFO |
| 8 | `DRV_Nrf24l01ReadPayloadWithAck` | `NRF24L01_ReturnType` | Read received payload with ACK support |
| 9 | `DRV_Nrf24l01Transmit` | `NRF24L01_ReturnType` | Transmit data and wait for acknowledgment |
| 10 | `DRV_Nrf24l01SwitchMode` | `NRF24L01_ReturnType` | Switch between TX and RX mode |
| 11 | `DRV_Nrf24l01SetDataRate` | `NRF24L01_ReturnType` | Configure RF data rate |
| 12 | `DRV_Nrf24l01SetPALevel` | `NRF24L01_ReturnType` | Configure output power level |
| 13 | `DRV_Nrf24l01OpenWritingPipe` | `NRF24L01_ReturnType` | Configure TX address pipe |
| 14 | `DRV_Nrf24l01OpenReadingPipe` | `NRF24L01_ReturnType` | Configure RX pipe address |
## 📌 Notes

CE must be pulsed for transmission

CE must remain HIGH in receive mode

Always clear IRQ flags after handling events

Flush FIFO after MAX_RT

Ensure transmitter and receiver share:

Same RF channel

Same address

Same data rate

📄 License

This driver is open for educational and embedded development use.

# 👨‍💻 Author
Mai Thanh Khang

NRF24L01+ STM32 HAL Driver Implementation
If you'd like, I can also generate:
- A professional GitHub-ready README with badges  
- A version including timing diagrams  
- Or a minimal clean README for production repositories