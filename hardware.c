/**
 * @addtogroup    DFRobot
 * @{
 * @file
 * @author        Igor Serikov
 * @date          07-29-2014
 *
 * @brief         Generic hardware control module
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
 * Peripheral        | Pin(s)
 * ------------------|-------------------------------------
 * IR eye            | digital pin 12, analog inputs 2-5
 * Ultrasonic sensor | digital pin 4
 * Buzzar            | digital pin 11
 * Pan servo         | digital pin 10
 * Tilt servo        | digital pin 9
 *
 * Left and right motors and encoders are swaped to prevent
 * the wires from hanging.
 */

#include <avr/io.h>
#include "aug-interrupt.h"
#include "aug-delay.h"
#include "aug-math.h"

#include "timer.h"
#include "hardware.h"

/**
 * @brief Ultrasonic sensor state machine states
 */
typedef enum {
    us_none,
    us_begin,
    us_end
} us_state_type;

/**
 * @brief Ultrasonic sensor state machine current state
 */
volatile us_state_type us_state;

static volatile unsigned us_begin_time, us_end_time;

/**
 * @brief Hardware initialization routine
 */
static void hardware_init (void) __attribute__ ((constructor));
static void hardware_init (void) {
    /* Ultrasonic distance sensor interrupt setup */
    PCMSK2 &= ~_BV (PCINT20);
    PCICR |= _BV (PCIE2);

    /* Motors timer
       16000000 / 256 / 255 = 245.098039215 (245 Hz) */
    TCCR0A = _BV (COM0B1) | _BV (COM0A1) | _BV (WGM00) | _BV (WGM01);
    TCCR0B = _BV (CS02);

    /* Motor pins are set to output */
    DDRD |= _BV (DDD7);
    DDRB |= _BV (DDB0);

    /* Multiplexer setup */
    ADCSRA = _BV (ADPS0) | _BV (ADPS1) | _BV (ADPS2) | _BV (ADEN);

    /* IR leds pin is set to output */
    DDRB |= _BV (DDB4);

    /* Led pin is set output */
    DDRB |= _BV (DDB5);

    /* Buzzer pin is set to output */
    DDRB |= _BV (DDB3);

    /* Pan pin is set to output */
    DDRB |= _BV (DDB2);

    /* Tilt pin is set to output */
    DDRB |= _BV (DDB1);
}

static void send_to_servo (volatile uint8_t * port, uint8_t bits, unsigned duration) {
    unsigned i;

    /* We disable interrupts to guarantee precision of the delay */
    cli ();
    *port |= bits;
    for (i = 0; i < duration; i += 10)
        _delay_us (10);
    *port &= ~bits;
    sei ();
}

/**
 * @brief  Sends a pulse to pan servo
 * @param  duration  pulse duratio in microseconds
 */
void pan_pulse (unsigned duration) {
    send_to_servo (&PORTB, _BV (PORTB2), duration);
}

/**
 * @brief  Sends a pulse to tilt servo
 * @param  duration  pulse duratio in microseconds
 */
void tilt_pulse (unsigned duration) {
    send_to_servo (&PORTB, _BV (PORTB1), duration);
}

/* Ultraonic sensor interrupt handler */
ISR (PCINT2_vect) {
    switch (us_state) {
      case us_none:
        /* Sensor whistles */
        us_begin_time = pclock ();
        us_state = us_begin;
        break;
      case us_begin:
        /* Sensor got an echo or a timeout */
        us_end_time = pclock ();
        us_state = us_end;
        PCMSK2 &= ~_BV (PCINT20);
        break;
      default:
        break;
    }
}

/**
 * @brief   Measure distance before to an object using ultrasonic sensor
 * @return  distance in cm
 */
unsigned ultrasonic_measure () {
    uint8_t sreg = SREG;

    cli ();

    us_state = us_none;

    PCIFR &= ~_BV (PCIF2);

    PORTD &= ~_BV (PORTD4);
    DDRD |= _BV (DDD4);
    _delay_us (2);
    PORTD |= _BV (PORTD4);
    _delay_us (15);
    PORTD &= ~_BV (PORTD4);
    _delay_us (2);
    DDRD &= ~_BV (DDD4);

    PCMSK2 |= _BV (PCINT20);

    SREG = sreg;

    SynthOS_wait (us_state == us_end);

    /* Sound travels 343 m/s */
    return round (time_step * pdiff (us_begin_time, us_end_time) * 343 * 100 / 2);
}

/** @brief  Enables (starts) left motor */
void left_motor_enable (void) {
    DDRD |= _BV (DDD5);
}

/** @brief  Disables (stops) left motor */
void left_motor_disable (void) {
    DDRD &= ~_BV (DDD5);
}

/** 
 * @brief  Sets left motor torque 
 * @param  torque  torque as a part of 255 (0-255)
 */
void left_motor_set (uint8_t torque) {
    OCR0B = torque;
}

/** @brief  Sets left motor direction to forward */
void left_motor_forward (void) {
    PORTD &= ~_BV (PORTD7);
}

/** @brief  Sets left motor direction to backward */
void left_motor_backward (void) {
    PORTD |= _BV (PORTD7);
}

/** @brief  Enables (starts) right motor */
void right_motor_enable (void) {
    DDRD |= _BV (DDD6);
}

/** @brief  Disables (stops) right motor */
void right_motor_disable (void) {
    DDRD &= ~_BV (DDD6);
}

/** 
 * @brief  Sets right motor torque 
 * @param  torque  torque as a part of 255 (0-255)
 */
void right_motor_set (uint8_t torque) {
    OCR0A = torque;
}

/** @brief  Sets right motor direction to forward */
void right_motor_forward (void) {
    PORTB &= ~_BV (PORTB0);
}

/** @brief  Sets right motor direction to backward */
void right_motor_backward (void) {
    PORTB |= _BV (PORTB0);
}

/**
 * @brief Reads ADC converted value from a given
 *        analog input
 * @param  pin  input pin (0-15)
 * @return  10 bit value from ADC
*/
static uint16_t read_mux (uint8_t pin) {
    ADMUX = _BV (REFS0) | pin;
    ADCSRA |= _BV (ADSC);
    while (ADCSRA & _BV (ADSC)) {} // 13 ADC cycles
    return (uint16_t) ADCL | (uint16_t) ADCH << 8;
}


/** 
 * @brief Reads left encoder value
 * @return  10 bit value from ADC
 */
uint16_t left_encoder (void) {
    return read_mux (0);
}

/** 
 * @brief Reads right encoder value
 * @return  10 bit value from ADC
 */
uint16_t right_encoder (void) {
    return read_mux (1);
}

/** 
 * @brief Reads temperature sensor value
 * @return  10 bit value from ADC
 */
uint16_t temperature (void) {
    return read_mux (8);
}

/** 
 * @brief Reads left infrared eye sensor value
 * @return  10 bit value from ADC
 */
uint16_t left_eye (void) {
    return read_mux (2);
}

/** 
 * @brief Reads top infrared eye sensor value
 * @return  10 bit value from ADC
 */
uint16_t top_eye (void) {
    return read_mux (3);
}

/** 
 * @brief Reads right infrared eye sensor value
 * @return  10 bit value from ADC
 */
uint16_t right_eye (void) {
    return read_mux (4);
}

/** 
 * @brief Reads bottom infrared eye sensor value
 * @return  10 bit value from ADC
 */
uint16_t bottom_eye (void) {
    return read_mux (5);
}

/** @brief Enables infrared leds  */
void ir_leds_enable (void) {
    PORTB |= _BV (PORTB4);
}

/** @brief Disables infrared leds  */
void ir_leds_disable (void) {
    PORTB &= ~_BV (PORTB4);
}

/** @brief Enables led (pin 13)  */
void led_enable (void) {
    PORTB |= _BV (PORTB5);
}

/** @brief Disables led (pin 13)  */
void led_disable (void) {
    PORTB &= ~_BV (PORTB5);
}

/** @brief Enables buzzer  */
void buzzer_enable (void) {
    PORTB |= _BV (PORTB3);
}

/** @brief Disables buzzer  */
void buzzer_disable (void) {
    PORTB &= ~_BV (PORTB3);
}

/** @brief Shuts system power down  */
void power_down (void) {
    SMCR = _BV (SM1) | _BV (SE);
    __asm__ __volatile__ ("sleep" ::: "memory");
}
