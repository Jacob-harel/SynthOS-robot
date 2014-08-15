/**
 *
 * Project:       Arduino (DFRobot rover v2) robot
 * File:          config.c
 * Author:        Igor Serikov
 * Date:          07-29-2014
 *
 * Purpose:       Project configuration.
 *
 * Copyright (c) 2014 Zeidman Technologies, Inc.
 * 15565 Swiss Creek Lane, Cupertino California, 95014 
 * All Rights Reserved
 *
 * Zeidman Technologies gives an unlimited, nonexclusive license to
 * use this code  as long as this header comment section is kept
 * intact in all distributions and all future versions of this file
 * and the routines within it.
 *
 */

/* This is the processor type. It is defined by GCC but SynthOS needs
   it too. */
#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

/* This is CPU frequency. Note that util/delay.h uses it. */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
