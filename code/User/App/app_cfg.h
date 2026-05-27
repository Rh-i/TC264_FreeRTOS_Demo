#ifndef __APP_CFG_H__
#define __APP_CFG_H__

#include "bsp_io.h"

#include "bsp_uart.h"
#include "uart_protocol.h"

#include "bsp_encoder.h"
#include "bsp_pwm.h"

#include "device_motor.h"
#include "device_servo.h"
#include "device_r9ds.h"

#include "auto_ctrl.h"
#include "r9ds_ctrl.h"

/**
 * @brief 用户所有的初始化
 */
void user_init(void);

#endif // __APP_CFG_H__
