/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "mbedtls/x509_crt.h"
#include "mbedtls/error.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/compat-1.3.h"

#include "socket.h"

#include "ssl_transport_interface.h"

#include "timer_interface.h"

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
int ssl_random_callback(void *p_rng, unsigned char *output, size_t output_len)
{
    int i;

    if (output_len <= 0)
    {
        return 1;
    }

    for (i = 0; i < output_len; i++)
    {
        *output++ = rand() % 0xff;
    }

    srand(rand());

    return 0;
}

int recv_timeout(void *ctx, unsigned char *buf, size_t len, uint32_t timeout)
{
    uint32_t start_ms = millis();

    do
    {
        if (getSn_RX_RSR((uint8_t)ctx) > 0)
        {
            return recv((uint8_t)ctx, (uint8_t *)buf, (uint16_t)len);
        }
    } while ((millis() - start_ms) < timeout);

    return 0;
}
/*Shell for mbedtls debug function.
 *DEBUG_LEBEL can be changed from 0 to 3*/
#ifdef MBEDTLS_DEBUG_C
void WIZnetDebugCB(void *ctx, int level, const char *file, int line, const char *str)
{
    if (level <= DEBUG_LEVEL)
    {
        printf("%s\r\n", str);
    }
}
#endif

/* SSL context initialization
 * */
int ssl_transport_init(tlsContext_t *tlsContext, int *socket_fd, const char *host)
{
    int ret = 1;

#if defined(MBEDTLS_ERROR_C)
    char error_buf[100];
#endif

#if defined(MBEDTLS_DEBUG_C)
    debug_set_threshold(DEBUG_LEVEL);
#endif

    /*
        Initialize session data
    */

#if defined(MBEDTLS_ENTROPY_C)
    mbedtls_entropy_init(&tlsContext->entropy);
#endif

    mbedtls_ctr_drbg_init(&tlsContext->ctr_drbg);
    mbedtls_ssl_init(&tlsContext->ssl);
    mbedtls_ssl_config_init(&tlsContext->conf);
    mbedtls_x509_crt_init(&tlsContext->cacert);
    mbedtls_x509_crt_init(&tlsContext->clicert);
    mbedtls_pk_init(&tlsContext->pkey);

    /*
        Initialize certificates
    */
#if defined(MBEDTLS_ENTROPY_C)
    if ((ret = mbedtls_ctr_drbg_seed(&tlsContext->ctr_drbg, mbedtls_entropy_func, &tlsContext->entropy,
                                     (const unsigned char *)pers, strlen(pers))) != 0)
    {
        printf(" failed\r\n  ! mbedtls_ctr_drbg_seed returned -0x%x\r\n", -ret);

        return -1;
    }
#endif

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_ssl_conf_dbg(&tlsContext->conf, WIZnetDebugCB, stdout);
#endif

    /*
        Parse certificate
    */
    if (tlsContext->rootca_option != MBEDTLS_SSL_VERIFY_NONE)
    {
        ret = mbedtls_x509_crt_parse(&tlsContext->cacert, (const unsigned char *)tlsContext->root_ca, strlen(tlsContext->root_ca) + 1);

        if (ret < 0)
        {
            printf(" failed\r\n  !  mbedtls_x509_crt_parse returned -0x%x while parsing root cert\r\n", -ret);

            return -1;
        }
        printf("ok! mbedtls_x509_crt_parse returned -0x%x while parsing root cert\r\n", -ret);

        if ((ret = mbedtls_ssl_set_hostname(&tlsContext->ssl, host)) != 0)
        {
            printf(" failed mbedtls_ssl_set_hostname returned %d\r\n", ret);

            return -1;
        }
        printf("ok! mbedtls_ssl_set_hostname returned %d\r\n", ret);
    }

    if (tlsContext->clica_option == true)
    {
        ret = mbedtls_x509_crt_parse((&tlsContext->clicert), (const unsigned char *)tlsContext->client_cert, strlen(tlsContext->client_cert) + 1);
        if (ret != 0)
        {
            printf(" failed\r\n  !  mbedtls_x509_crt_parse returned -0x%x while parsing device cert\r\n", -ret);

            return -1;
        }
        printf("ok! mbedtls_x509_crt_parse returned -0x%x while parsing device cert\r\n", -ret);

        ret = mbedtls_pk_parse_key(&tlsContext->pkey, (const unsigned char *)tlsContext->private_key, strlen(tlsContext->private_key) + 1, NULL, 0);
        if (ret != 0)
        {
            printf(" failed\r\n  !  mbedtls_pk_parse_key returned -0x%x while parsing private key\r\n", -ret);

            return -1;
        }
        printf("ok! mbedtls_pk_parse_key returned -0x%x while parsing private key\r\n", -ret);
    }

    if ((ret = mbedtls_ssl_config_defaults(&tlsContext->conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        printf(" failed mbedtls_ssl_config_defaults returned %d\r\n", ret);

        return -1;
    }

    printf("Root CA verify option %d\r\n", tlsContext->rootca_option);

    mbedtls_ssl_conf_authmode(&tlsContext->conf, tlsContext->rootca_option);
    mbedtls_ssl_conf_ca_chain(&tlsContext->conf, &tlsContext->cacert, NULL);
    mbedtls_ssl_conf_rng(&tlsContext->conf, ssl_random_callback, &tlsContext->ctr_drbg);

    if (tlsContext->clica_option == true)
    {
        if ((ret = mbedtls_ssl_conf_own_cert(&tlsContext->conf, &tlsContext->clicert, &tlsContext->pkey)) != 0)
        {
            printf("failed! mbedtls_ssl_conf_own_cert returned %d\r\n", ret);

            return -1;
        }
        printf("ok! mbedtls_ssl_conf_own_cert returned %d\r\n", ret);
    }

    mbedtls_ssl_conf_endpoint(&tlsContext->conf, MBEDTLS_SSL_IS_CLIENT);
    mbedtls_ssl_conf_read_timeout(&tlsContext->conf, SSL_RECV_TIMEOUT_VALUE);

    if ((ret = mbedtls_ssl_setup(&tlsContext->ssl, &tlsContext->conf)) != 0)
    {
        printf(" failed mbedtls_ssl_setup returned -0x%x\r\n", -ret);

        return -1;
    }
    mbedtls_ssl_set_bio(&tlsContext->ssl, socket_fd, send, recv, recv_timeout);

    return 0;
}

/*Free the memory for ssl context*/
void ssl_transport_deinit(tlsContext_t *tlsContext)
{
    /*  free SSL context memory  */

    mbedtls_ssl_free(&tlsContext->ssl);
    mbedtls_ssl_config_free(&tlsContext->conf);
    mbedtls_ctr_drbg_free(&tlsContext->ctr_drbg);
#if defined(MBEDTLS_ENTROPY_C)
    mbedtls_entropy_free(&tlsContext->entropy);
#endif
    mbedtls_x509_crt_free(&tlsContext->cacert);
    mbedtls_x509_crt_free(&tlsContext->clicert);
    mbedtls_pk_free(&tlsContext->pkey);
}

int ssl_socket_connect_timeout(tlsContext_t *tlsContext, char *addr, unsigned int port, unsigned int local_port, uint32_t timeout)
{
    int ret;
    uint32_t start_ms = millis();

    uint8_t sock = (uint8_t)(tlsContext->ssl.p_bio);

    /*socket open*/
    ret = socket(sock, Sn_MR_TCP, local_port, 0x00);
    if (ret != sock)
        return ret;

    /*Connect to the target*/
    ret = connect(sock, addr, port);
    if (ret != SOCK_OK)
        return ret;

    printf(" Performing the SSL/TLS handshake...\r\n");

    while ((ret = mbedtls_ssl_handshake(&tlsContext->ssl)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
#if defined(MBEDTLS_ERROR_C)
            memset(tempBuf, 0, 1024);
            mbedtls_strerror(ret, (char *)tempBuf, DEBUG_BUFFER_SIZE);
            printf(" failed\r\n  ! mbedtls_ssl_handshake returned %d: %s\n\r", ret, tempBuf);
#endif
            printf(" failed\r\n  ! mbedtls_ssl_handshake returned -0x%x\n\r", -ret);

            return (-1);
        }

        if ((millis() - start_ms) > timeout) // timeout
        {
#if defined(MBEDTLS_ERROR_C)
            memset(tempBuf, 0, 1024);
            mbedtls_strerror(ret, (char *)tempBuf, DEBUG_BUFFER_SIZE);
            printf(" timeout\r\n  ! mbedtls_ssl_handshake returned %d: %s\n\r", ret, tempBuf);
#endif
            printf(" timeout\r\n  ! mbedtls_ssl_handshake returned -0x%x\n\r", ret);

            return (-1);
        }
    }

#if defined(MBEDTLS_DEBUG_C)
    printf(" ok\n\r    [ Ciphersuite is %s ]\n\r",
           mbedtls_ssl_get_ciphersuite(&tlsContext->ssl));
#endif
    printf(" ok\n\r    [ Ciphersuite is %s ]\n\r",
           mbedtls_ssl_get_ciphersuite(&tlsContext->ssl));

    return (0);
}

int ssl_transport_read(tlsContext_t *tlsContext, unsigned char *readbuf, unsigned int len)
{
    return mbedtls_ssl_read(&tlsContext->ssl, readbuf, len);
}

int ssl_transport_write(tlsContext_t *tlsContext, unsigned char *writebuf, unsigned int len)
{
    return mbedtls_ssl_write(&tlsContext->ssl, writebuf, len);
}

int ssl_transport_disconnect(tlsContext_t *tlsContext, uint32_t timeout)
{
    int ret = 0;
    uint8_t sock = (uint8_t)(tlsContext->ssl.p_bio);
    uint32_t tickStart = millis();

    do
    {
        ret = disconnect(sock);
        if ((ret == SOCK_OK) || (ret == SOCKERR_TIMEOUT))
            break;
    } while ((millis() - tickStart) < timeout);

    if (ret == SOCK_OK)
        ret = sock; // socket number

    return ret;
}

/* ssl Close notify */
unsigned int ssl_transport_close_notify(tlsContext_t *tlsContext)
{
    uint32_t rc;
    do
        rc = mbedtls_ssl_close_notify(&tlsContext->ssl);
    while (rc == MBEDTLS_ERR_SSL_WANT_WRITE);

    return rc;
}

/* ssl session reset */
int ssl_transport_session_reset(tlsContext_t *tlsContext)
{
    return mbedtls_ssl_session_reset(&tlsContext->ssl);
}

int ssl_transport_check_ca(uint8_t *ca_data, uint32_t ca_len)
{
    int ret;

    mbedtls_x509_crt ca_cert;
    mbedtls_x509_crt_init(&ca_cert);

    ret = mbedtls_x509_crt_parse(&ca_cert, (const char *)ca_data, ca_len + 1);
    if (ret < 0)
    {
        printf(" failed\r\n  !  mbedtls_x509_crt_parse returned -0x%x while parsing root cert\r\n", -ret);
    }
    else
        printf("ok! mbedtls_x509_crt_parse returned -0x%x while parsing root cert\r\n", -ret);

    mbedtls_x509_crt_free(&ca_cert);

    return ret;
}

int ssl_transport_check_pkey(uint8_t *pkey_data, uint32_t pkey_len)
{
    int ret;

    mbedtls_pk_context pk_cert;
    mbedtls_pk_init(&pk_cert);

    ret = mbedtls_pk_parse_key(&pk_cert, (const char *)pkey_data, pkey_len + 1, NULL, 0);
    if (ret != 0)
    {
        printf(" failed\r\n  !  mbedtls_pk_parse_key returned -0x%x while parsing private key\r\n", -ret);
    }
    else
    {
        printf(" ok !  mbedtls_pk_parse_key returned -0x%x while parsing private key\r\n", -ret);
    }

    mbedtls_pk_free(&pk_cert);

    return ret;
}
