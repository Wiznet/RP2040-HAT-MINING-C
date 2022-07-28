/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SSL_TRANSPORT_INTERFACE_H_
#define _SSL_TRANSPORT_INTERFACE_H_

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdlib.h>

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "mbedtls/x509_crt.h"
#include "mbedtls/error.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/compat-1.3.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
#define DEBUG_LEVEL 3
#define DEBUG_BUFFER_SIZE 1024

#define SSL_RECV_TIMEOUT_VALUE 10000

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
typedef struct
{
#if defined(MBEDTLS_ENTROPY_C)
    mbedtls_entropy_context entropy;
#endif
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
    mbedtls_x509_crt clicert;
    mbedtls_pk_context pkey;
    uint8_t *root_ca;
    uint8_t *client_cert;
    uint8_t *private_key;
    uint8_t rootca_option;
    uint8_t clica_option;
} tlsContext_t;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
int ssl_random_callback(void *p_rng, unsigned char *output, size_t output_len);
int recv_timeout(void *ctx, unsigned char *buf, size_t len, uint32_t timeout);
#if defined(MBEDTLS_DEBUG_C)
void WIZnetDebugCB(void *ctx, int level, const char *file, int line, const char *str);
#endif
int ssl_transport_init(tlsContext_t *tlsContext, int *socket_fd, const char *host);
void ssl_transport_deinit(tlsContext_t *tlsContext);
int ssl_socket_connect_timeout(tlsContext_t *tlsContext, char *addr, unsigned int port, unsigned int local_port, uint32_t timeout);
int ssl_transport_read(tlsContext_t *tlsContext, unsigned char *readbuf, unsigned int len);
int ssl_transport_write(tlsContext_t *tlsContext, unsigned char *writebuf, unsigned int len);
int ssl_transport_disconnect(tlsContext_t *tlsContext, uint32_t timeout);
unsigned int ssl_transport_close_notify(tlsContext_t *tlsContext);
int ssl_transport_session_reset(tlsContext_t *tlsContext);
int ssl_transport_check_ca(uint8_t *ca_data, uint32_t ca_len);
int ssl_transport_check_pkey(uint8_t *pkey_data, uint32_t pkey_len);

#endif /* _SSL_TRANSPORT_INTERFACE_H_ */
