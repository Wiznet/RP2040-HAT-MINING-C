/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _TIMER_INTERFACE_H_
#define _TIMER_INTERFACE_H_

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
void repeating_timer_callback(void);
time_t millis(void);
#ifdef __cplusplus
}
#endif
#endif /* _TIMER_INTERFACE_H_ */
