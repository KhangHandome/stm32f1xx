LoRa SX127x (Ra-02) STM32 HAL Driver

Thư viện driver giao tiếp LoRa SX127x (Ra-02) sử dụng giao thức SPI trên nền tảng STM32 HAL.
Hỗ trợ cả Polling mode và Interrupt mode, phù hợp cho các ứng dụng LoRa point-to-point hoặc gateway đơn giản.

1. Sơ đồ đấu nối (Wiring Diagram)

Kết nối module LoRa Ra-02 với STM32 theo bảng dưới đây.
⚠️ Lưu ý quan trọng: Module LoRa cực kỳ nhạy cảm với sụt áp, cần nguồn 3.3V ổn định, dòng có thể lên tới ~120mA khi phát.
+---------+------------+-------------------+------------------------------+
| Pin     | Chức năng  | Kết nối STM32     | Ghi chú                      |
+---------+------------+-------------------+------------------------------+
| VCC     | Nguồn cấp  | 3.3V              | Không dùng 5V                |
| GND     | Ground     | GND               | Chung mass với MCU           |
| SCK     | SPI Clock  | SPI_SCK           | Khuyên dùng < 1 MHz          |
| MISO    | SPI MISO   | SPI_MISO          | Dữ liệu từ LoRa về MCU       |
| MOSI    | SPI MOSI   | SPI_MOSI          | Dữ liệu từ MCU đi LoRa       |
| CS      | Chip Select| GPIO Output       | Kéo LOW khi giao tiếp        |
| RST     | Reset      | GPIO Output       | Kéo LOW để reset cứng        |
| DIO0    | Interrupt  | GPIO EXTI / Input | Báo TxDone / RxDone          |
+---------+------------+-------------------+------------------------------+

2. Hướng dẫn cấu hình (Customization)

Các thông số hoạt động của LoRa được quản lý thông qua cấu trúc
LORA_HandleTypedef.

Bạn có thể tùy chỉnh các tham số RF để đánh đổi giữa tốc độ truyền và khoảng cách.

Các thông số chính

Frequency: Tần số hoạt động (ví dụ: 433000000)

Spreading Factor (SF): Từ SF7 → SF12

SF càng cao → khoảng cách xa hơn, tốc độ chậm hơn

Bandwidth (BW): Thông thường 125 kHz

Power: Công suất phát (Low, Balance, Max)

OperationMode: Polling hoặc Interrupt

Ví dụ cấu hình
LORA_HandleTypedef MyLora = {
    .SPI_Instance    = &hspi1,
    .ChipSelectPort  = GPIOA,
    .ChipSelectPin   = GPIO_PIN_4,
    .ResetPort       = GPIOA,
    .ResetPin        = GPIO_PIN_0,

    // RF Configuration
    .Frequency       = 433000000,          // 433 MHz
    .SpreadingFactor = LORA_SF_7,           // SF7 cho tốc độ nhanh
    .Bandwidth       = LORA_BW_125_KHZ,     // Băng thông chuẩn
    .Power           = LORA_POWER_BALANCE,  // Công suất trung bình
    .OperationMode   = LORA_MODE_POLLING    // Polling hoặc Interrupt
};

3. Cách sử dụng thư viện (API Usage)
Bước 1: Khởi tạo (Initialization)

Hàm khởi tạo sẽ:

Reset chip

Kiểm tra thanh ghi Version (phải là 0x12)

Nạp toàn bộ cấu hình RF

if (DRV_LoraInit(&MyLora) == STD_E_OK) {
    // Khởi tạo thành công
}

Bước 2: Truyền dữ liệu (Transmit)

Hàm đóng gói dữ liệu vào FIFO và phát đi.

uint8_t msg[] = "Hello LoRa";
DRV_LoraTransmit(&MyLora, msg, strlen((char*)msg), 100);

Bước 3: Nhận dữ liệu (Receive)

Tùy theo OperationMode đã chọn:

🔹 Polling Mode

Hàm sẽ chờ dữ liệu đến hoặc timeout.

uint8_t rx_buffer[64];

if (DRV_LoraReceive(&MyLora, rx_buffer, 64, 500) == STD_E_OK) {
    // Đã nhận được dữ liệu
}

🔹 Interrupt Mode

Sử dụng ngắt ngoài (EXTI) trên chân DIO0.

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == MyLora.Dio_0_Pin) {
        DRV_LoraReceive(&MyLora, rx_buffer, 64, 0);
    }
}


📌 Khuyến nghị:
Không thực hiện giao tiếp SPI trực tiếp trong ISR nếu hệ thống phức tạp.
ISR chỉ nên dùng để set flag, xử lý SPI ở main loop hoặc task.

4. Lưu ý quan trọng khi triển khai

Chuyển Mode:
Hàm DRV_LoraSwitchMode() luôn đưa chip về Standby trước khi chuyển mode mới để đảm bảo PLL ổn định.

SPI Mode:
STM32 SPI nên cấu hình Mode 0
(CPOL = 0, CPHA = 0).

Anten:
⚠️ Tuyệt đối không phát khi chưa gắn anten, có thể làm cháy PA của module LoRa.

CRC:
Driver đã bật sẵn Payload CRC để tránh nhận dữ liệu rác.