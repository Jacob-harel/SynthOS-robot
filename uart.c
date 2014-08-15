/**
 * @addtogroup    Atmega328p
 * @{
 * @file
 * @uthor        Igor Serikov
 * @date         07-29-2014
 *
 * @brief        Atmega328p UART driver
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
#include <avr/io.h>
#include "aug-interrupt.h"

#include "uart.h"

#define UART_PRESCALLER  (unsigned) (((F_CPU / (UART_BAUDRATE * 8UL))) - 1)

/**
 * @brief UART initialization routine
 */
static void uart_init (void) __attribute__ ((constructor));
static void uart_init (void) {
    uart_send_put = uart_send_get = uart_receive_put = uart_receive_get = 0;

    UCSR0B = _BV (TXEN0) | _BV (RXEN0) | _BV (RXCIE0);
    UCSR0A |= _BV (U2X0);
    UCSR0C = _BV (UCSZ00) | _BV (UCSZ01);
    UBRR0H = (uint8_t) (UART_PRESCALLER >> 8);
    UBRR0L = (uint8_t) UART_PRESCALLER;
}

/* This is modified by interrupts */
volatile unsigned uart_send_put, uart_send_get, uart_receive_put, uart_receive_get;
volatile unsigned char uart_send_buf [UART_SEND_BUFFER_SIZE], uart_receive_buf [UART_RECEIVE_BUFFER_SIZE];

/** @brief Activates UART transmission by enable "register empty" interrupt */
void uart_transmit (void) {
    UCSR0B |= _BV (UDRIE0);
}

ISR (USART_UDRE_vect) {
    if (uart_send_get != uart_send_put) {
        UDR0 = uart_send_buf [uart_send_get];
        uart_send_get = (uart_send_get + 1) % UART_SEND_BUFFER_SIZE;
        return;
    }
    UCSR0B &= ~_BV (UDRIE0);
}

ISR (USART_RX_vect) {
    unsigned uart_receive_next = (uart_receive_put + 1) % UART_RECEIVE_BUFFER_SIZE;
    unsigned char b = UDR0;
    if (uart_receive_next != uart_receive_get) {
        uart_receive_buf [uart_receive_put] = b;
        uart_receive_put = uart_receive_next;
    }
}
