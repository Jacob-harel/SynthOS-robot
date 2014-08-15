/**
 * @addtogroup    SynthOS
 * @{
 * @file
 * @author        Igor Serikov
 * @date          07-29-2014
 *
 * @brief         SynthOS printing functions interface
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
 * SynthOS does not support ellipses yet, so we have to use the following
 * macros.
 */
#include <stdint.h>

/** @brief Output format string \a fmt following printf conventions */
#define print0(fmt) SynthOS_call (print ((uintptr_t)(fmt), 0, 0, 0))

/** @brief Output format string \a fmt following printf conventions and using \a a1 as argument */
#define print1(fmt, a1) SynthOS_call (print ((uintptr_t)(fmt),  (a1), 0, 0))

/** @brief Output format string \a fmt following printf conventions and using \a a1-a2 as arguments */
#define print2(fmt, a1, a2) SynthOS_call (print ((uintptr_t)(fmt),  (a1),  (a2), 0))

/** @brief Output format string \a fmt following printf conventions and using \a a1-a3 as arguments */
#define print3(fmt, a1, a2, a3) SynthOS_call (print ((uintptr_t)(fmt),  (a1),  (a2),  (a3)))
