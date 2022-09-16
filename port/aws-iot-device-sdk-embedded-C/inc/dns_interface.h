/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _DNS_INTERFACE_H_
#define _DNS_INTERFACE_H_

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
// DNS non-block mode : DNS op states
#define STATE_DNS_INIT 0
#define STATE_DNS_SEND_QUERY 1
#define STATE_DNS_RECV_RESPONSE 2
#define STATE_DNS_DONE 3
#define STATE_DNS_STOP 4

#define DNS_TIMEOUT 10000

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
// DNS non-block mode: DNS return value
enum
{
    DNS_RET_TIMEOUT = -2,
    DNS_RET_FAILED = -1,
    DNS_RET_RUNNING = 0,
    DNS_RET_SUCCESS = 1,
    DNS_RET_STOPPED = 2
};

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
void DNS_init_handler(uint8_t s, uint8_t *buf, uint32_t (*tickFunc)(void));
int8_t DNS_run_handler(uint8_t *dns_ip, uint8_t *name, uint8_t *ip_from_dns, uint32_t timeout);
int8_t process_dns(uint8_t channel);
int8_t get_ipaddr_from_dns(uint8_t *domain, uint8_t *ip_from_dns, uint8_t *buf, uint32_t timeout);

#endif /* _DNS_INTERFACE_H_ */
