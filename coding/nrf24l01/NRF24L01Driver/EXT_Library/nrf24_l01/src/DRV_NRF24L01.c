#include "DRV_NRF24L01.h"
#include "stdint.h"
#define READ_REGISTER(x) ((x) & 0x1F)
#define WRITE_REGISTER(x) ((x) | 0x20 )
static uint8_t DRV_Nrf_ReadRegister(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t reg);
static void DRV_Nrf_WriteRegister(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t reg, uint8_t data);
static void DRV_Nrf24L01ReadPayload(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t *data, uint8_t length);
static void DRV_Nrf24L01WritePayload(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t *data, uint8_t length);
static uint8_t DRV_Nrf_ReadRegister(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t reg)
{
    uint8_t cmd = READ_REGISTER(reg);
    uint8_t status = 0;
    uint8_t data = 0;

    /* Down CS Pin to low lever to start a new transmision */
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);

    /* Select address to read and read status return */
    HAL_SPI_TransmitReceive(NRF24L01Instance->SPI_Instance, &cmd, &status, 1, 10);
    HAL_SPI_Receive(NRF24L01Instance->SPI_Instance, &data, 1, 10);
    /* Up CS Pin to high lever to end current transmision */
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);
    return data;
}
static void DRV_Nrf_WriteRegister(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t reg, uint8_t data)
{
	uint8_t l_reg = WRITE_REGISTER(reg);
	/* Set CS Pin to low level to start a new transmission */
	HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort,NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &l_reg , 1, 10);
	HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &data, 1, 10);

	/* Set CS Pin to high level to stop transmit data */
	HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort,NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);
}
NRF24L01_ReturnType DRV_Nrf24l01Init(NRF24L01_HandleTypedef* NRF24L01Instance)
{
	uint8_t retval = 0 ;
	uint8_t counter = 0;
	uint8_t cmd    = 0 ;
    HAL_Delay(5); // Chờ chip ổn định nguồn

    // 1. Cơ bản: Power Up và bật CRC (2 bytes)
    DRV_Nrf_WriteRegister(NRF24L01Instance, CONFIG, (1 << EN_CRC) | (1 << CRCO) | (1 << PWR_UP));

    /* Enable active mode */
	/* 1. Kéo CSN xuống thấp để bắt đầu phiên SPI */
	HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);
	/* 2. Gửi lệnh enable activate  */
	cmd = ACTIVATE; // Lệnh R_RX_PAYLOAD
	HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
	cmd = 0x73 ; // Lệnh R_RX_PAYLOAD
	HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);

	/* 4. Kéo CSN lên cao để kết thúc */
	HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);

	/* Enable payload length */
	DRV_Nrf_WriteRegister(NRF24L01Instance, FEATURE, (1 << 2 ));
	DRV_Nrf_WriteRegister(NRF24L01Instance, DYNPD, 0x3F);


    // 2. Bật Auto-ACK cho tất cả Pipes (P0-P5)
    DRV_Nrf_WriteRegister(NRF24L01Instance, EN_AA, 0x3F);

    // 3. Bật tất cả các Pipes để sẵn sàng nhận
    DRV_Nrf_WriteRegister(NRF24L01Instance, EN_RXADDR, 0x3F);

    // 4. Cấu hình cho việc tự động gửi lại dữ liệu
    DRV_Nrf_WriteRegister(NRF24L01Instance, SETUP_RETR, (NRF24L01Instance->AutoRetransmitDelay << 4) | (NRF24L01Instance->AutoRetransmitCount));

    // 5. Cấu hình tần số
    DRV_Nrf_WriteRegister(NRF24L01Instance, RF_CH, NRF24L01Instance->FrequencyChannel);

    // 6. Setup speed
    DRV_Nrf24l01SetDataRate(NRF24L01Instance, NRF24L01Instance->NRF24L01_AirDataDate);
    DRV_Nrf24l01SetPALevel(NRF24L01Instance, NRF24L01Instance->NRF24L01_OutputPower);
    retval = DRV_Nrf_ReadRegister(NRF24L01Instance, RF_CH);

    // 7. Nạp địa chỉ mặc định cho các Pipes và đặt Payload Width = 32
    DRV_Nrf24l01OpenWritingPipe(NRF24L01Instance, NRF24L01Instance->TxAddress);
    DRV_Nrf24l01OpenReadingPipe(NRF24L01Instance, 0, NRF24L01Instance->RxAddressP0);
    DRV_Nrf24l01OpenReadingPipe(NRF24L01Instance, 1, NRF24L01Instance->RxAddressP1);

    /* Địa chỉ cho pipe từ 2 tới 5 chỉ cần 1 byte */
    for(counter = 2; counter <= 5; counter++) {
        DRV_Nrf24l01OpenReadingPipe(NRF24L01Instance, counter, &NRF24L01Instance->RxAddressP2_5[counter-2]);
    }
    cmd = FLUSH_TX;
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, 0);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, 1);

    cmd = FLUSH_RX;
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, 0);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, 1);

    // Xóa cờ trạng thái rác nếu có
    DRV_Nrf_WriteRegister(NRF24L01Instance, STATUS, 0x70);
    /* Advoid compiler warning */
    (void) retval;
    return STD_E_OK;
}
void DRV_Nrf24l01Deinit(NRF24L01_HandleTypedef* NRF24L01Instance)
{
	/* Reset all state */
	DRV_Nrf_WriteRegister(NRF24L01Instance, CONFIG, 0x00);
}
/*
 * @brief : This function use to read payload from rx_buffer
 */
static void DRV_Nrf24L01ReadPayload(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t *data, uint8_t length)
{
	uint8_t cmd = R_RX_PAYLOAD; // Lệnh R_RX_PAYLOAD

	/* 1. Kéo CSN xuống thấp để bắt đầu phiên SPI */
	HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);

	/* 2. Gửi lệnh đọc Payload */
	HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);

	/* 3. Nhận dữ liệu (độ dài 'length' byte) từ module về mảng 'data' */
	HAL_SPI_Receive(NRF24L01Instance->SPI_Instance, data, length, 100);

	/* 4. Kéo CSN lên cao để kết thúc */
	HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);
}
/*
 * @brief : This function use to write payload from rx_buffer
 */
static void DRV_Nrf24L01WritePayload(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t *data, uint8_t length )
{
	uint8_t cmd = W_TX_PAYLOAD; // Lệnh W_TX_PAYLOAD

	/* 1. Kéo CSN xuống thấp */
	HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);

	/* 2. Gửi lệnh ghi Payload */
	HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);

	/* 3. Truyền mảng 'data' vào module */
	HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, data, length, 100);

	/* 4. Kéo CSN lên cao */
	HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);

	/* Lưu ý: Sau khi gọi hàm này, bạn cần kích chân CE lên HIGH ít nhất 10us để chip bắt đầu phát */
}
int8_t DRV_Nrf24l01Receive(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t *destinationPtr, uint8_t length)
{
    uint8_t config = 0;
    uint8_t status = 0;
    uint8_t fifo_status = 0;
    uint8_t pipeId = -1;
    uint8_t cmd    = 0 ;
    uint32_t timeoutCount = 100000; // Tăng giá trị này tùy theo tốc độ clock của MCU

    // 1. Chuyển sang chế độ RX (PRIM_RX = 1)
    config = DRV_Nrf_ReadRegister(NRF24L01Instance, CONFIG);
    DRV_Nrf_WriteRegister(NRF24L01Instance, CONFIG, config | (1 << 0)); // Bit 0 là PRIM_RX

    // 2. Kích hoạt CE để bắt đầu nghe
    HAL_GPIO_WritePin(NRF24L01Instance->ChipEnablePort, NRF24L01Instance->ChipEnablePin, GPIO_PIN_SET);

    // 3. Chờ cho đến khi có dữ liệu (RX_DR) hoặc hết Timeout
    do {
        status = DRV_Nrf_ReadRegister(NRF24L01Instance, STATUS);
        timeoutCount--;
    } while (!(status & (1 << 6)) && (timeoutCount > 0)); // Bit 6 là RX_DR

    // 4. Nếu thoát vòng lặp do có dữ liệu (RX_DR = 1)
    if (status & (1 << 6)) {
        // Lấy Pipe ID của gói tin đầu tiên
        pipeId = (status >> 1) & 0x07;

        // Vòng lặp "Hút sạch" FIFO
        do {
        	/* 1. Kéo CSN xuống thấp để bắt đầu phiên SPI */
        	HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);
        	/* 2. Gửi lệnh đọc Payload */
        	cmd = R_RX_PL_WID;
        	HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
        	/* 3. Nhận dữ liệu (độ dài 'length' byte) từ module về mảng 'data' */
        	length = 0 ;
        	HAL_SPI_Receive(NRF24L01Instance->SPI_Instance, &length, 1, 100);
        	HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);

        	// Đọc Payload từ tầng hiện tại của FIFO
            DRV_Nrf24L01ReadPayload(NRF24L01Instance, destinationPtr, length);

            // Xóa cờ RX_DR để nRF có thể đẩy gói tin tiếp theo (nếu có) lên đầu FIFO
            DRV_Nrf_WriteRegister(NRF24L01Instance, STATUS, (1 << 6));

            // Kiểm tra xem FIFO còn dữ liệu không (Bit 0 của FIFO_STATUS là RX_EMPTY)
            fifo_status = DRV_Nrf_ReadRegister(NRF24L01Instance, FIFO_STATUS);

        } while (!(fifo_status & (1 << 0))); // Tiếp tục nếu RX_EMPTY == 0

        // Đưa CE về thấp (Standby-I) sau khi đã lấy hết dữ liệu
        HAL_GPIO_WritePin(NRF24L01Instance->ChipEnablePort, NRF24L01Instance->ChipEnablePin, GPIO_PIN_RESET);
        return pipeId;
    }

    // Nếu hết timeout mà không có dữ liệu
    HAL_GPIO_WritePin(NRF24L01Instance->ChipEnablePort, NRF24L01Instance->ChipEnablePin, GPIO_PIN_RESET);
    return -1;
}
NRF24L01_ReturnType DRV_Nrf24l01Transmit(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t *targetAddr, uint8_t *sourcePtr, uint8_t length)
{
	// Bước 1: Đảm bảo PRIM_RX = 0 (Chế độ TX)
	    uint8_t config = DRV_Nrf_ReadRegister(NRF24L01Instance, CONFIG);
	    config &= ~(1 << PRIM_RX);
	    DRV_Nrf_WriteRegister(NRF24L01Instance, CONFIG, config);

	    // Bước 2: Cấu hình địa chỉ (Chỉ ghi nếu cần, nhưng để an toàn ta ghi mỗi lần)
	    // Cài đặt TX_ADDR và RX_ADDR_P0 giống nhau để nhận ACK
	    DRV_Nrf24l01OpenWritingPipe(NRF24L01Instance, targetAddr);

	    // Bước 2.1: Clock dữ liệu vào TX FIFO
	    DRV_Nrf24L01WritePayload(NRF24L01Instance, sourcePtr, length);

	    // Bước 3: Kích xung CE tối thiểu 10us để bắt đầu phát
	    HAL_GPIO_WritePin(NRF24L01Instance->ChipEnablePort, NRF24L01Instance->ChipEnablePin, GPIO_PIN_SET);
	    // Dùng delay nhỏ hoặc loop để đảm bảo > 10us
	    HAL_Delay(5);
	    HAL_GPIO_WritePin(NRF24L01Instance->ChipEnablePort, NRF24L01Instance->ChipEnablePin, GPIO_PIN_RESET);

	    // Bước 5: Đợi cờ TX_DS hoặc MAX_RT (Đợi phản hồi từ Radio)
	    uint8_t status;
	    uint32_t timeout = 50000; // Tránh treo máy
	    do {
	        status = DRV_Nrf_ReadRegister(NRF24L01Instance, STATUS);
	        timeout--;
	    } while (!(status & ((1 << 5) | (1 << 4))) && (timeout > 0));

	    // Bước 5.1: Xóa cờ ngắt trong STATUS (ghi 1 vào bit đó)
	    DRV_Nrf_WriteRegister(NRF24L01Instance, STATUS, (1 << 5) | (1 << 4));

	    // Bước 5.2: Xử lý lỗi MAX_RT
	    if (status & (1 << 4))
	    {
	        // Gói tin vẫn ở trong FIFO, phải Flush nếu muốn gửi gói mới
	        uint8_t cmd = FLUSH_TX;
	        HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, 0);
	        HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
	        HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, 1);

	        return STD_E_NOT_OK; // Truyền thất bại
	    }

	    return STD_E_OK; // Truyền thành công và nhận được ACK
}
NRF24L01_ReturnType DRV_Nrf24l01SetChannel(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t channel)
{
	return STD_E_OK;
}

NRF24L01_ReturnType DRV_Nrf24l01SetDataRate(NRF24L01_HandleTypedef* NRF24L01Instance, NRF24L01_AirDataDateTypedef speed)
{
    uint8_t rf_setup = DRV_Nrf_ReadRegister(NRF24L01Instance, RF_SETUP);

    // Tốc độ nằm ở bit 3 (RF_DR_HIGH).
    // Theo header của bạn: 0 = 1MBPS, 1 = 2MBPS
    if (speed == NRF24L01_2MBPS)
    {
        rf_setup |= (1 << 3);
    }
    else
    {
        rf_setup &= ~(1 << 3);
    }

    DRV_Nrf_WriteRegister(NRF24L01Instance, RF_SETUP, rf_setup);

    // Kiểm tra lại xem đã ghi thành công chưa
    if (DRV_Nrf_ReadRegister(NRF24L01Instance, RF_SETUP) == rf_setup) return STD_E_OK;
    return STD_E_NOT_OK;
}
NRF24L01_ReturnType DRV_Nrf24l01SetPALevel(NRF24L01_HandleTypedef* NRF24L01Instance, NRF24L01_OutputPowerTypedef level)
{
    uint8_t rf_setup = DRV_Nrf_ReadRegister(NRF24L01Instance, RF_SETUP);

    // Xóa bit 2:1 hiện tại
    rf_setup &= ~(0x06);

    // Ghi level mới vào (Dịch sang trái 1 bit để khớp vị trí 2:1)
    rf_setup |= (level << 1);

    DRV_Nrf_WriteRegister(NRF24L01Instance, RF_SETUP, rf_setup);
    return STD_E_OK;
}
NRF24L01_ReturnType DRV_Nrf24l01OpenWritingPipe(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t *address)
{
    // Bước 1: Ghi địa chỉ vào TX_ADDR (5 byte mặc định)
    uint8_t cmd = W_REGISTER | TX_ADDR;
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, address, 5, 50);
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);

    // Bước 2: Ghi trùng địa chỉ vào RX_ADDR_P0 để hứng gói tin ACK
    cmd = W_REGISTER | RX_ADDR_P0;
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, address, 5, 50);
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);

    return STD_E_OK;
}
NRF24L01_ReturnType DRV_Nrf24l01OpenReadingPipe(NRF24L01_HandleTypedef* NRF24L01Instance, uint8_t pipeNum, uint8_t *address)
{
    if (pipeNum > 5)
    {
    	return STD_E_NOT_OK;
    }

    // 1. Ghi địa chỉ vào thanh ghi RX_ADDR_Px
    uint8_t reg = RX_ADDR_P0 + pipeNum;
    uint8_t cmd = W_REGISTER | reg;

    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, &cmd, 1, 10);

    // P0 và P1 dùng 5 byte, P2-P5 chỉ dùng 1 byte cuối (LSB)
    uint8_t addrSize = (pipeNum < 2) ? 5 : 1;
    HAL_SPI_Transmit(NRF24L01Instance->SPI_Instance, address, addrSize, 10);
    HAL_GPIO_WritePin(NRF24L01Instance->ChipSelectPort, NRF24L01Instance->ChipSelectPin, GPIO_PIN_SET);

//    // 2. Thiết lập độ rộng Payload (mặc định 32 byte để dễ dùng)
//    DRV_Nrf_WriteRegister(NRF24L01Instance, RX_PW_P0 + pipeNum, 32);

    // 3. Kích hoạt Pipe này trong thanh ghi EN_RXADDR
    uint8_t en_rx = DRV_Nrf_ReadRegister(NRF24L01Instance, EN_RXADDR);
    en_rx |= (1 << pipeNum);
    DRV_Nrf_WriteRegister(NRF24L01Instance, EN_RXADDR, en_rx);

    return STD_E_OK;
}

