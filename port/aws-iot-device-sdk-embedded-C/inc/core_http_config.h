/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CORE_HTTP_CONFIG_H_
#define _CORE_HTTP_CONFIG_H_

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
/* Port */
#define HTTP_PORT 80
#define HTTPS_PORT 443

/* Buffer */
#define HTTP_BUF_MAX_SIZE (1024 * 2)
#define HTTP_DOMAIN_MAX_SIZE 128

/* Timeout */
#define HTTP_TIMEOUT (1000 * 10) // 10 seconds

/* HTTP header */
#ifndef HTTP_CONTENT_TYPE_FIELD
#define HTTP_CONTENT_TYPE_FIELD "Content-Type"                             /**< HTTP header field "Content-Type". */
#define HTTP_CONTENT_TYPE_FIELD_LEN (sizeof(HTTP_CONTENT_TYPE_FIELD) - 1U) /**< The length of #HTTP_CONTENT_TYPE_FIELD. */
#endif

#ifndef HTTP_CONTENT_TYPE_VALUE
#define HTTP_CONTENT_TYPE_VALUE "application/octet-stream"
#define HTTP_CONTENT_TYPE_VALUE_LEN (sizeof(HTTP_CONTENT_TYPE_VALUE) - 1U) /**< The length of #HTTP_CONTENT_TYPE_VALUE. */
#endif

#define HTTP_CONNECTION_CLOSE_VALUE "close"
#define HTTP_CONNECTION_CLOSE_VALUE_LEN (sizeof(HTTP_CONNECTION_CLOSE_VALUE) - 1U)

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

#endif /* _CORE_HTTP_CONFIG_H_ */
