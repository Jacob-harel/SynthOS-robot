/**
 * @addtogroup    DFRobot
 * @{
 * @file
 * @author        Igor Serikov
 * @date          07-29-2014
 *
 * @brief         Motors control module interface
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
typedef enum {
    motors_action_stop = 0,
    motors_action_forward,
    motors_action_backward,
    motors_action_left,
    motors_action_right
} motors_action_t;

void motors_stop (void);
void motors_left (void);
void motors_right (void);
void motors_forward (void);
void motors_backward (void);

extern volatile motors_action_t motors_action;
extern volatile unsigned motors_left_count, motors_right_count;
