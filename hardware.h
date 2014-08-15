/**
 * @addtogroup    DFRobot
 * @{
 * @file
 * @author        Igor Serikov
 * @date          07-29-2014
 *
 * @brief         Generic hardware control module interface
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

#include <stdint.h>

void pan_pulse (unsigned duration);
void tilt_pulse (unsigned duration);
void left_motor_enable (void);
void left_motor_disable (void);
void left_motor_forward (void);
void left_motor_backward (void);
void right_motor_enable (void);
void right_motor_disable (void);
void right_motor_forward (void);
void right_motor_backward (void);
void left_motor_set (uint8_t torque);
void right_motor_set (uint8_t torque);
uint16_t left_encoder (void);
uint16_t right_encoder (void);
uint16_t temperature (void);
uint16_t left_eye (void);
uint16_t top_eye (void);
uint16_t right_eye (void);
uint16_t bottom_eye (void);

void ir_leds_enable (void);
void ir_leds_disable (void);
void led_enable (void);
void led_disable (void);
void buzzer_enable (void);
void buzzer_disable (void);
void power_down (void);
