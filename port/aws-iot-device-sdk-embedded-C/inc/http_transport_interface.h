/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _HTTP_TRANSPORT_INTERFACE_H_
#define _HTTP_TRANSPORT_INTERFACE_H_

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "socket.h"

#include "transport_interface.h"
#include "ssl_transport_interface.h"
#include "core_http_client.h"
#include "core_http_config.h"


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
typedef enum
{
    HTTP_IDLE = 0,
    HTTP_RUNNING,
} http_state_t;

typedef enum
{
    GET = 0,
    POST,
} http_method_t;

typedef struct __http_config
{
    http_state_t http_state;
    http_method_t http_method;
    char http_domain[HTTP_DOMAIN_MAX_SIZE];
    uint32_t http_domain_len;
    char http_path[HTTP_DOMAIN_MAX_SIZE];
    uint32_t http_path_len;
    char http_ip[4];
    uint16_t http_port;
    uint8_t ssl_flag;
    char *bodyDataPtr;
    uint32_t bodyLen;
} http_config_t;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Common */
int32_t http_send_request(TransportInterface_t *pTransportInterface, uint8_t *buffer, char *pMethod, http_config_t *http_config);

/* HTTP */
int8_t http_connect(uint8_t sock, http_config_t *http_config);
int8_t http_close(uint8_t sock, http_config_t *http_config);
int32_t http_write(NetworkContext_t *pNetworkContext, const void *pBuffer, size_t bytesToSend);
int32_t http_read(NetworkContext_t *pNetworkContext, void *pBuffer, size_t bytesToRecv);
int32_t http_get(uint8_t sock, uint8_t *buffer, char *http_url, tlsContext_t *tls_context);
int32_t http_post(uint8_t sock, uint8_t *buffer, char *http_url, tlsContext_t *tls_context);

/* HTTPS */
int8_t https_connect(uint8_t sock, http_config_t *http_config, tlsContext_t *tls_context);
int32_t https_write(NetworkContext_t *pNetworkContext, const void *pBuffer, size_t bytesToSend);
int32_t https_read(NetworkContext_t *pNetworkContext, void *pBuffer, size_t bytesToRecv);

/* Util */
HTTPStatus_t getUrlPath(const char *pUrl, size_t urlLen, const char **pPath, size_t *pPathLen);
HTTPStatus_t getUrlAddress(const char *pUrl, size_t urlLen, const char **pAddress, size_t *pAddressLen, uint32_t *port);
HTTPStatus_t getUrlInfo(const char *pUrl, size_t urlLen, const char **pAddress, size_t *pAddressLen, const char **pPath, size_t *pPathLen, uint32_t *port);
int is_https(const char *pUrl);

#endif /* _HTTP_TRANSPORT_INTERFACE_H_ */
