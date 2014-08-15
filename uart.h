/**
 * @addtogroup    Atmega328p
 * @{
 * @file
 * @uthor        Igor Serikov
 * @date         07-29-2014
 *
 * @brief        Atmega328p UART driver interface
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
 * -------------------------------------------------------------------
 * We use ring buffers for both sending and receiving. We do not need
 * to disable interrupts while manipulating the buffers. Note that
 * we check the corresponding condition explicitly after SynthOS_wait
 * as there is no guarantee that they are still met when the scheduler
 * gives control back to us.
 */
#ifndef UART_BAUDRATE
#define UART_BAUDRATE             115200
#endif

#ifndef UART_SEND_BUFFER_SIZE
#define UART_SEND_BUFFER_SIZE         64
#endif

#ifndef UART_RECEIVE_BUFFER_SIZE
#define UART_RECEIVE_BUFFER_SIZE      64
#endif

extern volatile unsigned uart_send_put, uart_send_get, uart_receive_put, uart_receive_get;
extern volatile unsigned char uart_send_buf [UART_SEND_BUFFER_SIZE], uart_receive_buf [UART_RECEIVE_BUFFER_SIZE];

void uart_transmit (void);

/**
 * @brief  Places byte \a b into the UART output buffer with. Waits, if there is no room.
 */
#define uart_put_byte(b)                                                \
    do {                                                                \
        unsigned char _b = (b);                                         \
        while (! ((uart_send_put + 1) % UART_SEND_BUFFER_SIZE != uart_send_get)) \
            SynthOS_wait ((uart_send_put + 1) % UART_SEND_BUFFER_SIZE != uart_send_get); \
        uart_send_buf [uart_send_put] = _b;                             \
        uart_send_put = (uart_send_put + 1) % UART_SEND_BUFFER_SIZE;    \
        uart_transmit ();                                               \
    } while (0)

/**
 * @brief  Places byte \a b into the UART output buffer with. Spins, if there is no room.
 */
#define uart_put_byte_busy(b)                                           \
    do {                                                                \
        unsigned char _b = (b);                                         \
        while (! ((uart_send_put + 1) % UART_SEND_BUFFER_SIZE != uart_send_get)) ; \
        uart_send_buf [uart_send_put] = _b;                             \
        uart_send_put = (uart_send_put + 1) % UART_SEND_BUFFER_SIZE;    \
        uart_transmit ();                                               \
    } while (0)

/**
 * @brief  Extracts a byte from UART input buffer and place it into location \a l. Waits, if there is no data.
 */
#define uart_get_byte(l)                                                \
    do {                                                                \
        while (!(uart_receive_get != uart_receive_put))                 \
            SynthOS_wait (uart_receive_get != uart_receive_put);        \
        l = uart_receive_buf [uart_receive_get];                        \
        uart_receive_get = (uart_receive_get + 1) % UART_RECEIVE_BUFFER_SIZE; \
    } while (0)
