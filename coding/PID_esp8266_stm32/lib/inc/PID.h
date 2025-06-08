/*
 * PID.h
 *
 *  Created on: Jun 7, 2025
 *      Author: KhangMT
 */

#ifndef _PID_H_
#define _PID_H_
#include "stdio.h"
#include "stdint.h"

/*
 * description :
 * Một bộ điều khiển vi tích phân tỉ lệ (PID- Proportional Integral Derivative)
 * là một cơ chế phản hồi vòng điều khiển tổng quát được sử dụng rộng rãi trong các hệ thống điều khiển công nghiệp
 * Giải thuật tính toán bộ điều khiển PID bao gồm 3 thông số riêng biệt, do đó đôi khi nó còn được gọi là điều khiển ba khâu:
 * các giá trị tỉ lệ, tích phân và đạo hàm, viết tắt là P, I, và D.
 */

typedef struct
{
	float Kp;
	float Ki;
	float Kd;
	float output;
	float previos_error;
	float integral;
	float setpoint;
}PID_t;

extern void CalculatorPID(PID_t *pid, float* current_value);
#endif /* _PID_H_ */
