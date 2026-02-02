/*
 * Lora_Reg.h
 *
 *  Created on: Jan 29, 2026
 *      Author: maith
 */

#ifndef INC_LORA_REG_H_
#define INC_LORA_REG_H_
/*
 * SX1278 / LoRa R01 Register Map (LoRa mode)
 */

/* Common registers */
#define REG_FIFO                       0x00
#define REG_OP_MODE                    0x01
#define REG_FRF_MSB                    0x06
#define REG_FRF_MID                    0x07
#define REG_FRF_LSB                    0x08
#define REG_PA_CONFIG                  0x09
#define REG_PA_RAMP                    0x0A
#define REG_OCP                        0x0B
#define REG_LNA                        0x0C
#define REG_FIFO_ADDR_PTR              0x0D
#define REG_FIFO_TX_BASE_ADDR          0x0E
#define REG_FIFO_RX_BASE_ADDR          0x0F
#define REG_FIFO_RX_CURRENT_ADDR       0x10
#define REG_IRQ_FLAGS_MASK             0x11
#define REG_IRQ_FLAGS                  0x12
#define REG_RX_NB_BYTES                0x13
#define REG_RX_HEADER_CNT_VALUE_MSB    0x14
#define REG_RX_HEADER_CNT_VALUE_LSB    0x15
#define REG_RX_PACKET_CNT_VALUE_MSB    0x16
#define REG_RX_PACKET_CNT_VALUE_LSB    0x17
#define REG_MODEM_STAT                 0x18
#define REG_PKT_SNR_VALUE              0x19
#define REG_PKT_RSSI_VALUE             0x1A
#define REG_RSSI_VALUE                 0x1B
#define REG_HOP_CHANNEL                0x1C
#define REG_MODEM_CONFIG_1             0x1D
#define REG_MODEM_CONFIG_2             0x1E
#define REG_SYMB_TIMEOUT_LSB           0x1F
#define REG_PREAMBLE_MSB               0x20
#define REG_PREAMBLE_LSB               0x21
#define REG_PAYLOAD_LENGTH             0x22
#define REG_MAX_PAYLOAD_LENGTH         0x23
#define REG_HOP_PERIOD                 0x24
#define REG_FIFO_RX_BYTE_ADDR          0x25
#define REG_MODEM_CONFIG_3             0x26

/* Frequency hopping / channel registers */
#define REG_FEI_MSB                    0x28
#define REG_FEI_MID                    0x29
#define REG_FEI_LSB                    0x2A
#define REG_RSSI_WIDEBAND              0x2C

/* Detection / sync */
#define REG_DETECT_OPTIMIZE            0x31
#define REG_INVERT_IQ                  0x33
#define REG_DETECTION_THRESHOLD        0x37
#define REG_SYNC_WORD                  0x39

/* DIO mapping */
#define REG_DIO_MAPPING_1              0x40
#define REG_DIO_MAPPING_2              0x41

/* Version */
#define REG_VERSION                    0x42

/* PA / TCXO / temperature */
#define REG_TCXO                       0x4B
#define REG_PA_DAC                     0x4D
#define REG_FORMER_TEMP                0x5B
#define REG_AGC_REF                    0x61
#define REG_AGC_THRESH1                0x62
#define REG_AGC_THRESH2                0x63
#define REG_AGC_THRESH3                0x64

/* PLL */
#define REG_PLL_HOP                    0x44
#define REG_PLL_LOCK                   0x70

/* Operation Mode */
#define SLEEP_MODE                     0x00
#define STANDBY_MODE                   0x01
#define TRANSMIT_MODE                  0x03
#define RECEIVE_CONTINUOUS_MODE        0x05
#define RECEIVE_SINGLE_MODE            0x06
#define CHANNEL_ACTIVITY_DETECTION     0x07


#endif /* INC_LORA_REG_H_ */
