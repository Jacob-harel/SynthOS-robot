/**
 * @addtogroup    DFRobot
 * @{
 * @file
 * @author        Igor Serikov
 * @date          07-29-2014
 *
 * @brief         Surroundings scanning and high level motion control
 *                module
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
#include "print.h"
#include "timer.h"
#include "hardware.h"
#include "util.h"
#include "motors.h"

typedef enum {
    pan_start                    =  600, /* pan pulse time in us */
    pan_stop                     = 1800, /* pan pulse time in us */
    pan_step                     =   15, /* pan pulse time in us */
    pan_reset_pulses             =   25,
#ifdef MIN_DISTANCE
    min_distance                 =   MIN_DISTANCE, /* in cm */
#else
    min_distance                 =   30, /* in cm */
#endif
    incremental_pan_pulses       =    2,
    turn_step_count              =    5, /* in wheel sectors */
    calibration_trigger_distance =    8  /* in cm */
} values_type;

unsigned robot_timer;

/**
 * @brief  Drives pan servo
 * @param  duration  pulse duration in microseconds
 * @param  count  number of pulses to send
 */
void drive_pan (unsigned duration, unsigned count) {
    unsigned i, start;

    for (i = 0; i < count; i ++) {
        pan_pulse (duration);
        /* Wait 20 ms, let servo work */
        start = pclock ();
        robot_timer = clock;
        /* Sleep first */
        SynthOS_wait (clock - robot_timer >= 2);
        /* Spin for the rest of the required time */
        while (pdiff (start, pclock ()) < 2 * clock_divider)
            SynthOS_sleep ();
    }
}

/**
 * @brief  Scanning and high level motion control function
 *
 * This function starts the motion and constantly scans the surroundings
 * using the ultrasonic sensor. Whenever we detected an object that is
 * close than "min_distance", we make left turn, stop and do a scanning
 * If nothing was is found during the turn, we move forward again.
 * Otherwise, we make another left turn and repeat the full scanning turn.
 * The basic constraints are:
 * 1. The scanning step should not be too big otherwise we can miss
 *    something.
 * 2. "min_distance" should be big enough so we would have time to
 *    make a full scan before hitting the object.
 */
void robot () {
    int dir;
    unsigned pos, val, min;

    /* To calibrate the center position, put your hand in front of the
     * sensor (not further away than calibration_trigger_distance) and turn the power.
     */
    val = SynthOS_call (ultrasonic_measure ());
    if (val <= calibration_trigger_distance) {
        SynthOS_call (drive_pan (pan_stop, pan_reset_pulses));
        SynthOS_call (drive_pan (pan_start, pan_reset_pulses));
        SynthOS_call (drive_pan ((pan_start + pan_stop) / 2, pan_reset_pulses));
        do_power_down ("Calibration\n");
    }

    SynthOS_call (drive_pan (pan_start, pan_reset_pulses));

    pos = pan_start;
    dir = pan_step;
    for (;;) {
        SynthOS_call (drive_pan (pos, incremental_pan_pulses));
        for (;;) {
            val = SynthOS_call (ultrasonic_measure ());
            if (pos <= 800 || pos >= 1600)
                min = min_distance * 14 / 10;
            else
                min = min_distance;
            if (val >= min_distance)
                break;
            /* We detected an object that is close than "min_distance" */
            print1 ("robot: left, got %u\n", val);
            motors_left ();
            SynthOS_wait (motors_left_count + motors_right_count >= turn_step_count);
            motors_stop ();
            /* Turn to the initial scanning position */
            SynthOS_call (drive_pan (pan_start, pan_reset_pulses));
            pos = pan_start;
            dir = pan_step;
        }
        if (pos >= pan_stop) {
            dir = - pan_step;
            if (motors_action != motors_action_forward) {
                /* We made a full turn while scanning surroundings after a stop and found no
                   object that are close to us so we can resume moving forward. */
                print0 ("robot: forward\n");
                motors_forward ();
            }
        } else 
            if (pos <= pan_start)
                dir = pan_step;
        pos += dir;
    }
}
