/**
 * @addtogroup    DFRobot
 * @{
 * @file
 * @author        Igor Serikov
 * @date          07-29-2014
 *
 * @brief         Motors control module
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
 *
 * Notes
 * --------------------------------------------------------
 * All the routines beside motors_stop reset the left and 
 * right counters.
 */
#include "timer.h"
#include "print.h"
#include "motors.h"
#include "util.h"

/* 
 * We need to apply a higher torque when turn
 * The speed of the robot greatly depends on the
 * battery charge. DO NOT use power supply with
 * these xxx_speed_yyy values.
 */
typedef enum {
    encoders_middle            = 840, /* Analog readout middle value */
    margin                     =  10, /* Uncertainty radius around the middle encoder value */
    low_speed_normal           =  80, /* in a part of 255 */
    high_speed_normal          = 100, /* in a part of 255 */
    low_speed_turn             = 120, /* in a part of 255 */
    high_speed_turn            = 140, /* in a part of 255 */
    time_top                   =  34, /* in ticks */
    time_bottom                =  22, /* in ticks */
    sector_maximum_delay       = 300, /* in ticks */
    acceleration_count         =   2, /* in wheel sectors */
    initial_acceleration_count =   8, /* in wheel sectors */
} motors_values_type;

/**
 * @brief  Last request made to the module via motors_xxx
 * 
 * Do not write to this variable directly.
 */
volatile motors_action_t motors_action;

/** @brief Wheel sector counters */
volatile unsigned motors_left_count, motors_right_count;

/** @brief Module initialization routine */
static void motors_init (void) __attribute__ ((constructor));
static void motors_init (void) {
    motors_action = motors_action_stop;
    motors_left_count = motors_right_count = 0;
}

/** @brief Stops the motors */
void motors_stop (void) {
    motors_action = motors_action_stop;
}

/** @brief Starts left turn */
void motors_left (void) {
    motors_action = motors_action_left;
    motors_left_count = motors_right_count = 0;
}

/** @brief Starts right turn */
void motors_right (void) {
    motors_action = motors_action_right;
    motors_left_count = motors_right_count = 0;
}

/** @brief Starts forward motion */
void motors_forward (void) {
    motors_action = motors_action_forward;
    motors_left_count = motors_right_count = 0;
}

/** @brief Starts backward motion */
void motors_backward (void) {
    motors_action = motors_action_forward;
    motors_left_count = motors_right_count = 0;
}

/**
 * @brief Gets a middle value out of three consecutive values
 *
 * Example: 6 2 5 -> 5
 *
 * @param  a  circular array containing 4 readings
 * @param  p  starting position
 * @return  middle value
 */
static unsigned get_middle (unsigned a [3], unsigned p) {
    unsigned v1 = a [p], v2 = a [(p + 1) % 3], v3 = a [(p + 2) % 3];
    if (v1 <= v2) {
        if (v2 <= v3)
            return v2; /* v1 <= v2 <= v3 */
        if (v1 <= v3)
            return v3; /* v1 <= v3 < v2 */
        return v1; /* v3 < v1 <= v2 */
    }
    if (v1 <= v3)
        return v1; /* v2 < v1 <= v3 */
    if (v2 <= v3)
        return v3; /* v2 <= v3 < v1 */
    return v2; /* v3 < v2 < v1 */
}

/**
 * @brief Analyzes an encoder value
 * @param  v  encoder value
 * @return  1 - low, 0 - not sure, 1 - high.
 */
static int qualify (unsigned v) {
    if (v <= encoders_middle - margin)
        return -1;
    if (v >= encoders_middle + margin)
        return 1;
    return  0;
}

/**
 * @brief Interval normalization
 *
 * In the encoder wheel, openings and bridges are
 * not equal in angle. This function pefrorm the
 * necessary timing normalization.
 * @param  t  timing
 * @return  q  opening/bridge indicator 
 *          (-1/1, we did not check what corresponds to what)
 * @return  normalized value
 */
static unsigned normalize (unsigned t, int q) {
    return q > 0 ? t * 5 / 4 : t * 5 / 6;
}

/* Everything below the cut line would be duplicated with "left" <-> "right"
   substitution. */
/* --- cut --- */

unsigned motors_left_timer;
motors_action_t left_action;

/**
 * @brief motor control function.
 *
 * Duplicating this function we get two functions:
 * one for the left motor and one for the right. The function controlled by setting
 * "motors_action". After the movement started, it constantly read the encoder value
 * and adjust speed accordingly. Also, it beeps and poweres the robot down when the
 * track gets stuck. To avoid oscillation, we get the middle value of every three
 * consecutive readings and allow some tolerance range. Also, let the robot 
 * accelerate/decelerate before making another decision about the speed.
 */
void left_motor () {
    unsigned index, mark, acc_start, acc_count, speed, middle, high_speed, last_clock;
    int value, new_value;
    unsigned clocks [3]; /* circular buffer containing last 3 encoder readings
                            (addressed by "index") */

 stop_motor:
    left_motor_disable ();

 handle:
    left_action = motors_action;

    switch (left_action) {
      case motors_action_forward:
        speed = low_speed_normal;
        high_speed = high_speed_normal;
        left_motor_forward ();
        break;
      case motors_action_backward:
        speed = low_speed_normal;
        high_speed = high_speed_normal;
        left_motor_backward ();
        break;
      case motors_action_left:
        speed = low_speed_turn;
        high_speed = high_speed_turn;
        left_motor_backward ();
        break;
      case motors_action_right:
        speed = low_speed_turn;
        high_speed = high_speed_turn;
        left_motor_forward ();
        break;
      default:
        ;
        /* This includes motors_action_stop */
        SynthOS_wait (motors_action != left_action);
        goto handle;
    }

    left_motor_set (speed);

    left_motor_enable ();

    mark = clock;
    value = qualify (left_encoder ());

    /* Waiting for the first value change */
    for (;;) {
        motors_left_timer = clock;
        SynthOS_wait (clock != motors_left_timer || motors_action != left_action);
        if (motors_action != left_action)
            goto stop_motor;
        new_value = qualify (left_encoder ());
        if (new_value != 0 && new_value != value)
            break;
        if (clock - mark >= sector_maximum_delay)
            do_power_down ("motors: left failed 1\n");
    }

    motors_left_count ++;

    last_clock = clock;

    /* Getting 3 values */
    for (index = 0; index < 3; index ++) {
        mark = clock;
        for (;;) {
            motors_left_timer = clock;
            SynthOS_wait (clock != motors_left_timer || motors_action != left_action);
            if (motors_action != left_action)
                goto stop_motor;
            new_value = qualify (left_encoder ());
            if (new_value != 0 && new_value != value)
                break;
            if (clock - mark >= sector_maximum_delay)
                do_power_down ("motors: left failed 2\n");
        }
        clocks [index] = normalize (clock - last_clock, new_value);
        last_clock = clock;
        value = new_value;
        motors_left_count ++;
    }

    /* Main loop */
    index = 0;
    acc_start = motors_left_count;
    /* No more adjustments until get this number of readings */
    acc_count = initial_acceleration_count;
    for (;;) {
        if (acc_count != 0 && motors_left_count - acc_start >= acc_count)
            acc_count = 0;
        if (acc_count == 0) {
            middle = get_middle (clocks, index);
            if (middle > time_top) {
                /* Try to increase the speed, we move too slow */
                if (speed < high_speed) {
                    print2 ("motors: left up: %u %u\n", middle, speed);
                    speed ++;
                    left_motor_set (speed);
                    acc_start = motors_left_count;
                    /* No more adjustments until get this number of readings */
                    acc_count = acceleration_count;
                }
            } else 
                if (middle < time_bottom) {
                    /* Try to decrease the speed, we move too fast */
                    if (speed > 0) {
                        print2 ("motors: left down: %u %u\n", middle, speed);
                        speed --;
                        left_motor_set (speed);
                        acc_start = motors_left_count;
                        /* No more adjustments until get this number of readings */
                        acc_count = acceleration_count;
                    }
                }
        }
        mark = clock;
        for (;;) {
            motors_left_timer = clock;
            SynthOS_wait (clock != motors_left_timer || motors_action != left_action);
            if (motors_action != left_action)
                goto stop_motor;
            new_value = qualify (left_encoder ());
            if (new_value != 0 && new_value != value)
                break;
            if (clock - mark >= sector_maximum_delay)
                do_power_down ("motors: left failed 3\n");
        }
        clocks [index] = normalize (clock - last_clock, new_value);
        last_clock = clock;
        value = new_value;
        index = (index + 1) % 3;
        motors_left_count ++;
    }
}
