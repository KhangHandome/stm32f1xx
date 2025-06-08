/*
 * PID.c
 *
 *  Created on: Jun 7, 2025
 *      Author: KhangMT
 */
#include "PID.h"

/**
 * @brief Hàm tính toán giá trị điều khiển PID.
 *
 * @param pid Con trỏ đến cấu trúc PID_t chứa các thông số PID và trạng thái trước đó.
 * @param current_value Giá trị hiện tại của hệ thống (giá trị đo được).
 */
void CalculatorPID(PID_t *pid, float* current_value)
{
	float tpm = *current_value;
    float error = pid->setpoint - tpm;

    // Tích lũy sai số để tính thành phần tích phân
    pid->integral += error;

    // Tính đạo hàm của sai số
    float derivative = error - pid->previos_error;

    // Tính đầu ra PID
    pid->output = pid->Kp * error + pid->Ki * pid->integral + pid->Kd * derivative;
    *current_value = pid->output;
    // Cập nhật sai số trước đó
    pid->previos_error = error;
}
