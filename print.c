/**
 * @addtogroup    SynthOS
 * @{
 * @file
 * @author        Igor Serikov
 * @date          07-29-2014
 *
 * @brief         SynthOS printing functions
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
#include <string.h>

#include "uart.h"
#include "print.h"

/**
 *  @brief  SynthOS_call-style printing routine
 *
 *  We use a fixed number and  follow "printf" conventions otherwise. 
 *  UART is used as an output.  Some conversions are not supported.
 *
 *  Route all your printing through this function to avoid interleaving
 *  outputs.
 */
void print (long fmt, long a1, long a2, long a3)  {
    /*
     *  "Top digit first" integer-to-string conversion does not
     *   need a buffer as we print what we get on every step.
     *  
     *   To get n such that b ^ (n + 1) > v, we use floor (v / b)
     *   as a boundary:
     *     v = b * x + y, y < b -> v - b * x < b                 (1)
     *     d > x, integers -> d >= x + 1 -> d * b - x * b >= b   (2)
     *     1,2 -> v - b * x < d * b - x * b -> v < d * b
     */
    char * s = (char*) (uintptr_t) fmt;
    long * p = args;

    char * ss, * xs, h;
    unsigned w, n;
    unsigned long u, x, d, b;
    unsigned char c, f;
    long args [3];

    args [0] = a1;
    args [1] = a2;
    args [2] = a3;

    while ((c = *s ++) != 0)
        switch (c) {
          case '%':
            ss = s;
            c = *s ++;
            if (c == '%') {
                uart_put_byte ('%');
                break;
            }
            f = ' ';
            if (c == '0')
                f = '0';
            w = 0;
            while (c >= '0' && c <= '9') {
                w = c - '0';
                c = *s ++;
            }
            if (c == 'l')
                c = *s ++;
            if (c == 'u') {
                u = *p ++;
                b = 10;
                h = 'a';
            } else if (c == 'd' || c == 'i') {
                if (*p >= 0)
                    u = *p;
                else {
                    uart_put_byte ('-');
                    u = - *p;
                }
                p ++;
                b = 10;
                h = 'a';
            } else if (c == 'x') {
                u = *p ++;
                b = 16;
                h = 'a';
            } else if (c == 'X') {
                u = *p ++;
                b = 16;
                h = 'A';
            } else  if (c == 'c') {
                c = (char) (*p ++);
                if (c == '\n') {
                    uart_put_byte ('\r');
                    uart_put_byte ('\n');
                    break;
                }
                while (w -- > 1)
                    uart_put_byte (' ');
                uart_put_byte (c);
                break;
            } else  if (c == 's') {
                xs = (char *) (uintptr_t) (*p ++);
                n = strlen (xs);
                while (n ++ < w)
                    uart_put_byte (' ');
                while (*xs) {
                    if (*xs == '\n')
                        uart_put_byte ('\r');
                    uart_put_byte (*xs ++);
                }
                break;
            } else {
                uart_put_byte ('%');
                s = ss;
                break;
            }
            x = u /  b;
            for (d = n = 1; d <= x; d *= b) 
                n ++;
            while (n ++ < w)
                uart_put_byte (f);
            while (d > 0) {
                c = (unsigned char) (u / d);
                if (c < 10)
                    c += '0';
                else
                    c = c - 10 + h;
                uart_put_byte (c);
                u = u % d;
                d /= b;
            }
            break;
          case '\n':
            ;
            uart_put_byte ('\r');
            uart_put_byte ('\n');
            break;
          default:
            ;
            uart_put_byte (c);
            break;
        }
}
