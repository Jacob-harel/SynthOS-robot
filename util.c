/**
 * @addtogroup    DFRobot
 * @{
 * @file
 * @author        Igor Serikov
 * @date          07-29-2014
 *
 * @brief         Utility functions
 *
 * @copyright
 * Copyright (c) 2014 Zeidman Technologies, Inc.
 * 15565 Swiss Creek Lane, Cupertino California, 95014 
 * All Rights Reserved
 *
 * @copyright
 * Zeidman Technologies gives an unlimited, nonexclusive license to
 * use this code  as long as this header comment section is kept
 * intact in all distributions and all future versions of this file
 * and the routines within it.
 */
#include "aug-delay.h"

#include "timer.h"
#include "uart.h"
#include "print.h"

/**
 * @brief  powers the system down
 * @param  msg  message to send to UART
 */
void do_power_down (char * msg) {
/* 
 * The whole system is going down. We have to stop all external parts.
 * We cannot use the scheduler because other tasks should be inactive now.
 * We keep interrupts enabled (UART needs them) until the very end.
 */
    left_motor_disable ();
    right_motor_disable ();
    buzzer_enable ();
    while (*msg) {
        if (*msg == '\n')
            uart_put_byte_busy ('\r');
        uart_put_byte_busy (*msg ++);
    }
    _delay_ms (100); /* Let buzzer and UART finish. */
    buzzer_disable ();
    power_down ();
}
